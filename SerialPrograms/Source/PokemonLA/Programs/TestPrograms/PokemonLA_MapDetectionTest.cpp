/*  Map Detection Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <ctime>
#include <optional>
#include <utility>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA_MapDetectionTest.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Pokemon;


// ── Pin template matcher ────────────────────────────────────────────────────
// Mirrors MapPinMatcher in PokemonLA_TreeShaker.cpp.  Defined here so the test
// program can run the same waterfill detection independently for tuning.

namespace{

class TestMapPinMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    TestMapPinMatcher() : WaterfillTemplateMatcher(
        "PokemonLA/PinAtMapTop.png",
        Color(0xff808000), Color(0xffffffff),
        20
    ){
        m_aspect_ratio_lower = 0.4;
        m_aspect_ratio_upper = 2.5;
        m_area_ratio_lower   = 0.4;
        m_area_ratio_upper   = 2.5;
    }
    static const TestMapPinMatcher& instance(){
        static TestMapPinMatcher m;
        return m;
    }
};

class TestMapPinDialogMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    TestMapPinDialogMatcher() : WaterfillTemplateMatcher(
        "PokemonLA/YellowArrowRight-Template.png",
        Color(0xff808008), Color(0xffffffff),
        200
    ){
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower   = 0.9;
        m_area_ratio_upper   = 1.1;
    }
    static const TestMapPinDialogMatcher& instance(){
        static TestMapPinDialogMatcher m;
        return m;
    }
};

}  // anonymous namespace


// ── Descriptor ─────────────────────────────────────────────────────────────

MapDetectionTest_Descriptor::MapDetectionTest_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:MapDetectionTest",
        STRING_POKEMON + " LA", "Map Detection Test",
        "",
        "Draw and calibrate detection boxes for map regions. "
        "Drag on the video to set the active box, then press \"Take Snapshot\" "
        "to save each box's region as a numbered PNG in the Resources folder.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::ENABLE_COMMANDS
    )
{}


// ── RegionDrawer ────────────────────────────────────────────────────────────
// Manages overlay boxes and mouse drag — mirrors BoxDraw's DrawnBox pattern.

class MapDetectionTest::RegionDrawer
    : public ConfigOption::Listener
    , public VideoOverlay::MouseListener
{
public:
    ~RegionDrawer(){ detach(); }

    RegionDrawer(MapDetectionTest& parent, VideoOverlay& overlay)
        : m_parent(parent)
        , m_overlay(overlay)
        , m_overlay_set(overlay)
    {
        try{
            m_parent.ACTIVE_REGION.add_listener(*this);
            m_parent.CURSOR_BOX.add_listener(*this);
            m_parent.PIN_BOX.add_listener(*this);
            m_parent.OPEN_BOX.add_listener(*this);
            overlay.add_mouse_listener(*this);
        }catch (...){
            detach();
            throw;
        }
    }

    virtual void on_config_value_changed(void*) override{
        std::lock_guard<Mutex> lg(m_lock);
        m_overlay_set.clear();
        m_overlay_set.add(COLOR_RED,   ImageFloatBox(m_parent.CURSOR_BOX));
        m_overlay_set.add(COLOR_GREEN, ImageFloatBox(m_parent.PIN_BOX));
        if (m_parent.OPEN_BOX.enabled()){
            m_overlay_set.add(COLOR_YELLOW, ImageFloatBox(m_parent.OPEN_BOX));
        }
    }

    virtual void on_mouse_press(double x, double y) override{
        m_mouse_start.emplace();
        m_mouse_start->first  = x;
        m_mouse_start->second = y;
    }
    virtual void on_mouse_release(double, double) override{
        m_mouse_start.reset();
    }
    virtual void on_mouse_move(double x, double y) override{
        if (!m_mouse_start) return;

        double xl = m_mouse_start->first;
        double xh = x;
        double yl = m_mouse_start->second;
        double yh = y;
        if (xl > xh) std::swap(xl, xh);
        if (yl > yh) std::swap(yl, yh);

        BoxOption& box = active_box();
        box.X.set(xl);
        box.Y.set(yl);
        box.WIDTH.set(xh - xl);
        box.HEIGHT.set(yh - yl);
    }

private:
    void detach(){
        m_overlay.remove_mouse_listener(*this);
        m_parent.OPEN_BOX.remove_listener(*this);
        m_parent.PIN_BOX.remove_listener(*this);
        m_parent.CURSOR_BOX.remove_listener(*this);
        m_parent.ACTIVE_REGION.remove_listener(*this);
    }

    BoxOption& active_box(){
        switch (m_parent.ACTIVE_REGION.get()){
        case ActiveRegion::PIN:  return m_parent.PIN_BOX;
        case ActiveRegion::OPEN: return m_parent.OPEN_BOX;
        default:                 return m_parent.CURSOR_BOX;
        }
    }

    MapDetectionTest& m_parent;
    VideoOverlay& m_overlay;
    VideoOverlaySet m_overlay_set;
    Mutex m_lock;
    std::optional<std::pair<double, double>> m_mouse_start;
};


// ── SnapshotButton ──────────────────────────────────────────────────────────

MapDetectionTest::SnapshotButton::SnapshotButton()
    : ButtonOption(
        "<b>Take Snapshot:</b><br>"
        "Saves each box's screen region as a new numbered PNG in "
        "Resources/PokemonLA/. Does not overwrite existing files.",
        "Take Snapshot"
    )
{}

void MapDetectionTest::SnapshotButton::set_source(
    VideoFeed* feed, BoxOption* cursor, BoxOption* pin, BoxOption* open
){
    m_feed       = feed;
    m_cursor_box = cursor;
    m_pin_box    = pin;
    m_open_box   = open;
}

void MapDetectionTest::SnapshotButton::press_button(){
    ButtonOption::press_button();
    if (!m_feed) return;

    VideoSnapshot snap = m_feed->snapshot();

    // Build a timestamp suffix: YYYYMMDD_HHMMSS
    std::time_t t = std::time(nullptr);
    char ts[32];
    std::strftime(ts, sizeof(ts), "%Y%m%d_%H%M%S", std::localtime(&t));
    std::string suffix(ts);

    std::string base = RESOURCE_PATH() + "PokemonLA/MapSnapshot_";

    auto save_region = [&](BoxOption* box, const std::string& name){
        if (!box) return;
        std::string path = base + name + "_" + suffix + ".png";
        extract_box_reference(snap, ImageFloatBox(*box)).save(path);
    };

    save_region(m_cursor_box, "cursor");
    save_region(m_pin_box,    "pin");
    if (m_open_box && m_open_box->enabled()){
        save_region(m_open_box, "open");
    }
}


// ── Constructor ─────────────────────────────────────────────────────────────

MapDetectionTest::MapDetectionTest()
    : ACTIVE_REGION(
        "<b>Mouse drag target:</b><br>"
        "Dragging on the video feed updates whichever box is selected here.",
        EnumDropdownDatabase<ActiveRegion>{
            {ActiveRegion::CURSOR, "cursor", "Cursor Box (red)"},
            {ActiveRegion::PIN,    "pin",    "Pin Dialog Box (green)"},
            {ActiveRegion::OPEN,   "open",   "Map Open Box (yellow)"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        ActiveRegion::CURSOR
    )
    , CURSOR_BOX(
        "<b>Pin-at-Top Box (red):</b><br>"
        "Region watched for the yellow map pin using waterfill detection.",
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::ALWAYS_ENABLED,
        false,
        {0.528, 0.020, 0.025, 0.050}
    )
    , PIN_BOX(
        "<b>Pin Dialog Box (green):</b><br>"
        "Region watched for the pin-placement dialog using yellow ratio detection.",
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::ALWAYS_ENABLED,
        false,
        {0.488, 0.155, 0.110, 0.122}
    )
    , OPEN_BOX(
        "<b>Map Open Box (yellow):</b><br>"
        "Bottom strip region detected via RMSD against MapOpen.png.",
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::DEFAULT_ENABLED,
        false,
        {0.000, 0.955, 0.500, 0.040}
    )
    , PIN_RMSD_THRESHOLD(
        "<b>Pin RMSD Threshold:</b><br>"
        "Maximum RMSD for a waterfill object to be accepted as the yellow pin.",
        LockMode::UNLOCK_WHILE_RUNNING,
        80.0, 0.0, 255.0
    )
    , PIN_MIN_AREA(
        "<b>Pin Min Area (pre-scale):</b><br>"
        "Minimum yellow-pixel count for a waterfill blob to be considered, "
        "before scaling by (screen_height / 1080)².",
        LockMode::UNLOCK_WHILE_RUNNING,
        15.0, 1.0, 500.0
    )
    , DIALOG_RMSD_THRESHOLD(
        "<b>Dialog RMSD Threshold:</b><br>"
        "Maximum RMSD for a waterfill icon blob to be accepted as a pin-dialog icon.",
        LockMode::UNLOCK_WHILE_RUNNING,
        80.0, 0.0, 255.0
    )
    , DIALOG_MIN_AREA(
        "<b>Dialog Min Area (pre-scale):</b><br>"
        "Minimum yellow-pixel count for a blob to be considered a dialog icon, "
        "before scaling by (screen_height / 1080)².",
        LockMode::UNLOCK_WHILE_RUNNING,
        100.0, 1.0, 2000.0
    )
    , OPEN_RMSD_THRESHOLD(
        "<b>Map Open RMSD Threshold:</b><br>"
        "RMSD below this value means the map-open strip matches MapOpen.png.",
        LockMode::UNLOCK_WHILE_RUNNING,
        62.0, 0.0, 255.0
    )
    , LOG_INTERVAL_MS(
        "<b>Log Interval (ms):</b><br>"
        "How often detection values are logged. Lower = more frequent output.",
        LockMode::UNLOCK_WHILE_RUNNING,
        5000, 100, 30000
    )
{
    PA_ADD_OPTION(ACTIVE_REGION);
    PA_ADD_OPTION(CURSOR_BOX);
    PA_ADD_OPTION(PIN_BOX);
    PA_ADD_OPTION(OPEN_BOX);
    PA_ADD_OPTION(PIN_RMSD_THRESHOLD);
    PA_ADD_OPTION(PIN_MIN_AREA);
    PA_ADD_OPTION(DIALOG_RMSD_THRESHOLD);
    PA_ADD_OPTION(DIALOG_MIN_AREA);
    PA_ADD_OPTION(OPEN_RMSD_THRESHOLD);
    PA_ADD_OPTION(LOG_INTERVAL_MS);
    PA_ADD_OPTION(SAVE_BUTTON);
}


// ── program() ───────────────────────────────────────────────────────────────

void MapDetectionTest::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    SAVE_BUTTON.set_source(&env.console.video(), &CURSOR_BOX, &PIN_BOX, &OPEN_BOX);

    RegionDrawer drawer(*this, env.console.overlay());
    drawer.on_config_value_changed(this);

    ImageMatch::ExactImageMatcher map_open_matcher(RESOURCE_PATH() + "PokemonLA/MapOpen.png");

    while (true){
        context.wait_for(std::chrono::milliseconds(LOG_INTERVAL_MS));
        VideoSnapshot snap = env.console.video().snapshot();
        if (!snap.frame) continue;

        const ImageFloatBox cursor_box = CURSOR_BOX;
        const ImageFloatBox pin_box    = PIN_BOX;
        const ImageFloatBox open_box   = OPEN_BOX;
        const double scale = snap.frame->height() / 1080.0;

        // ── Pin (waterfill) ───────────────────────────────────────────────
        ImageViewRGB32 cursor_region = extract_box_reference(*snap.frame, cursor_box);
        ImageStats cursor_stats = image_stats(cursor_region);

        const std::vector<std::pair<uint32_t,uint32_t>> pin_filters = {
            {combine_rgb(160, 160, 0), combine_rgb(255, 255, 255)},
        };
        const size_t min_size = std::max<size_t>(1, size_t((double)PIN_MIN_AREA * scale * scale));
        bool pin_detected = match_template_by_waterfill(
            snap.frame->size(), cursor_region,
            TestMapPinMatcher::instance(),
            pin_filters, {min_size, SIZE_MAX}, PIN_RMSD_THRESHOLD,
            [](Kernels::Waterfill::WaterfillObject&){ return true; }
        );
        env.log(
            "[Pin]    R=" + std::to_string(cursor_stats.average.r)
            + " G=" + std::to_string(cursor_stats.average.g)
            + " B=" + std::to_string(cursor_stats.average.b)
            + " | min_area=" + std::to_string(min_size)
            + " rmsd_thr=" + std::to_string((double)PIN_RMSD_THRESHOLD)
            + " | detected=" + (pin_detected ? "YES" : "NO")
        );

        // ── Dialog (waterfill) ────────────────────────────────────────────
        ImageViewRGB32 pin_region = extract_box_reference(*snap.frame, pin_box);
        ImageStats pin_stats = image_stats(pin_region);

        const std::vector<std::pair<uint32_t,uint32_t>> dialog_filters = {
            {combine_rgb(160, 160, 0), combine_rgb(255, 255, 255)},
        };
        const size_t dialog_min_size = std::max<size_t>(1, size_t((double)DIALOG_MIN_AREA * scale * scale));
        bool dialog_detected = match_template_by_waterfill(
            snap.frame->size(), pin_region,
            TestMapPinDialogMatcher::instance(),
            dialog_filters, {dialog_min_size, SIZE_MAX}, DIALOG_RMSD_THRESHOLD,
            [](Kernels::Waterfill::WaterfillObject&){ return true; }
        );
        env.log(
            "[Dialog] R=" + std::to_string(pin_stats.average.r)
            + " G=" + std::to_string(pin_stats.average.g)
            + " B=" + std::to_string(pin_stats.average.b)
            + " | min_area=" + std::to_string(dialog_min_size)
            + " rmsd_thr=" + std::to_string((double)DIALOG_RMSD_THRESHOLD)
            + " | detected=" + (dialog_detected ? "YES" : "NO")
        );

        // ── Map open (RMSD) ───────────────────────────────────────────────
        if (OPEN_BOX.enabled()){
            double open_rmsd = map_open_matcher.rmsd(
                extract_box_reference(*snap.frame, open_box)
            );
            bool open_detected = open_rmsd < (double)OPEN_RMSD_THRESHOLD;
            env.log(
                "[Open]   rmsd=" + std::to_string(open_rmsd)
                + " | threshold=" + std::to_string((double)OPEN_RMSD_THRESHOLD)
                + " | detected=" + (open_detected ? "YES" : "NO")
            );
        }
    }
}


}
}
}
