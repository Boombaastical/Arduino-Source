/*  Tree Shaker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <functional>
#include <optional>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_FlagTracker.h"
#include "PokemonLA/Inference/PokemonLA_OverworldDetector.h"
#include "PokemonLA/PokemonLA_TravelLocations.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_GameSave.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA_TreeShaker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Pokemon;


TreeShaker_Descriptor::TreeShaker_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:TreeShaker",
        STRING_POKEMON + " LA", "Tree Shaker",
        "",
        "Visit all trees in the Obsidian Fieldlands, shake every non-shaking tree, "
        "wait 20 minutes, and repeat until all trees are shaking.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}


TreeShaker::TreeShaker()
    : MAX_THROW_ATTEMPTS(
        "<b>Max Throw Attempts per Tree:</b><br>"
        "How many times to retry throwing at a tree before giving up and treating it as having berries. "
        "Each retry nudges the aim slightly.",
        LockMode::LOCK_WHILE_RUNNING,
        6, 1, 10
    )
    , START_TREE(
        "<b>Start from tree number:</b><br>"
        "Which tree to start from (1 = first tree). Useful for testing a specific tree.",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, 10
    )
    , ITEM_NOTIFICATION_BOX(
        "<b>Item Notification Box:</b><br>"
        "Screen region (x, y, width, height — all 0 to 1) where the 'received item' notification appears. "
        "Tune by watching the screen when berries are received from a tree.",
        LockMode::LOCK_WHILE_RUNNING,
        0.55, 0.70, 0.40, 0.20
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(MAX_THROW_ATTEMPTS);
    PA_ADD_OPTION(START_TREE);
    PA_ADD_OPTION(ITEM_NOTIFICATION_BOX);
    PA_ADD_OPTION(NOTIFICATIONS);
}


// ── Internal types ─────────────────────────────────────────────────────────

enum class TreeOutcome { FIGHT, BERRIES, NOTHING };
enum class TreeState   { UNKNOWN, SHAKING, BERRIES };

namespace{

// ── Berry drop detection ───────────────────────────────────────────────────

static const ImageFloatBox BERRY_XMARK_BOX{0.935, 0.605, 0.017, 0.028};
static const ImageFloatBox BERRY_TEXT_BOX {0.867, 0.576, 0.055, 0.031};
static const ImageFloatBox BERRY_LEVEL_BOXES[6] = {
    {0.055000, 0.210000, 0.028000, 0.048000},
    {0.055000, 0.286000, 0.028000, 0.048000},
    {0.055000, 0.360000, 0.028000, 0.048000},
    {0.055000, 0.434000, 0.028000, 0.048000},
    {0.055000, 0.507000, 0.028000, 0.048000},
    {0.055000, 0.580000, 0.028000, 0.048000},
};


class BerryDropDetector : public VisualInferenceCallback{
public:
    BerryDropDetector(Logger& logger, VideoOverlay& overlay)
        : VisualInferenceCallback("BerryDropDetector")
        , m_logger(logger)
        , m_overlay(overlay)
        , m_last_log(WallClock::min())
        , m_notification_seen(false)
        , m_phase1_started(false)
        , m_phase2_started(false)
    {
        m_logger.log("BerryDropDetector: initialized (v3 - 6 level boxes)");
        for (size_t i = 0; i < 6; i++){
            m_ov_level[i].emplace(m_overlay, BERRY_LEVEL_BOXES[i], COLOR_RED);
        }
    }
    virtual void make_overlays(VideoOverlaySet&) const override{}
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock now) override{
        const double scale = frame.height() / 1080.0;
        const size_t min_sz = std::max<size_t>(1, size_t(10 * scale * scale));
        const std::vector<std::pair<uint32_t,uint32_t>> white_filter = {
            {combine_rgb(200, 200, 200), combine_rgb(255, 255, 255)},
        };

        // Phase 1: latch when both the X-mark and white item text are detected.
        if (!m_notification_seen){
            if (!m_phase1_started){
                m_phase1_started = true;
                m_logger.log("BerryDropDetector [phase1]: started");
            }
            // Blob-existence check: any white blob in the xmark box means the X is present.
            PackedBinaryMatrix xmark_matrix = compress_rgb32_to_binary_multirange(
                extract_box_reference(frame, BERRY_XMARK_BOX), white_filter
            );
            auto xmark_session = Kernels::Waterfill::make_WaterfillSession(xmark_matrix);
            auto xmark_finder  = xmark_session->make_iterator(min_sz);
            Kernels::Waterfill::WaterfillObject xmark_obj;
            bool xmark = xmark_finder->find_next(xmark_obj, false);

            // Blob-existence check: any white blob in the item text area means text is present.
            PackedBinaryMatrix text_matrix = compress_rgb32_to_binary_multirange(
                extract_box_reference(frame, BERRY_TEXT_BOX), white_filter
            );
            auto text_session = Kernels::Waterfill::make_WaterfillSession(text_matrix);
            auto text_finder  = text_session->make_iterator(min_sz);
            Kernels::Waterfill::WaterfillObject text_obj;
            bool text = text_finder->find_next(text_obj, false);

            if (xmark && !m_ov_xmark) m_ov_xmark.emplace(m_overlay, BERRY_XMARK_BOX, COLOR_YELLOW);
            else if (!xmark)          m_ov_xmark.reset();
            if (text && !m_ov_text)   m_ov_text.emplace(m_overlay, BERRY_TEXT_BOX, COLOR_GREEN);
            else if (!text)           m_ov_text.reset();

            if (now - m_last_log >= std::chrono::seconds(1)){
                m_logger.log(
                    "BerryDropDetector [phase1]: xmark=" + std::to_string(xmark) +
                    " text=" + std::to_string(text)
                );
                m_last_log = now;
            }

            if (xmark && text){
                m_notification_seen = true;
                m_logger.log("BerryDropDetector: notification latched, now watching for Lv.");
            }
            return false;
        }

        // Phase 2: check all 6 possible Lv. positions; succeed if any matches.
        // Filter tuned to the observed text color: ~RGB(88–96, 88–96, 77–86).
        const std::vector<std::pair<uint32_t,uint32_t>> level_filter = {
            {combine_rgb(70, 70, 60), combine_rgb(110, 110, 100)},
        };

        if (!m_phase2_started){
            m_phase2_started = true;
            m_logger.log("BerryDropDetector [phase2]: started");
        }

        // Pass 1: blob counting + color sampling (no template, no throw risk).
        size_t total_blobs    = 0;
        size_t total_max_area = 0;
        std::string color_results;
        size_t blob_per_box[6] = {};
        for (size_t i = 0; i < 6; i++){
            ImageStats stats = image_stats(extract_box_reference(frame, BERRY_LEVEL_BOXES[i]));
            color_results += "(" + std::to_string((uint8_t)stats.average.r) + "," +
                             std::to_string((uint8_t)stats.average.g) + "," +
                             std::to_string((uint8_t)stats.average.b) + ") ";

            PackedBinaryMatrix level_matrix = compress_rgb32_to_binary_multirange(
                extract_box_reference(frame, BERRY_LEVEL_BOXES[i]), level_filter
            );
            auto level_session = Kernels::Waterfill::make_WaterfillSession(level_matrix);
            auto level_finder  = level_session->make_iterator(min_sz);
            Kernels::Waterfill::WaterfillObject level_obj;
            size_t blobs = 0, max_area = 0;
            while (level_finder->find_next(level_obj, false)){
                blobs++;
                max_area = std::max(max_area, level_obj.area);
            }
            blob_per_box[i]  = blobs;
            total_blobs     += blobs;
            total_max_area   = std::max(total_max_area, max_area);
        }

        if (now - m_last_log >= std::chrono::seconds(1)){
            std::string blob_results;
            for (size_t i = 0; i < 6; i++) blob_results += std::to_string(blob_per_box[i]) + " ";
            m_logger.log(
                "BerryDropDetector [phase2 blobs]: total=" + std::to_string(total_blobs) +
                " max_area=" + std::to_string(total_max_area) +
                " per_box=[" + blob_results + "]" +
                " avg_rgb=[" + color_results + "]"
            );
            m_last_log = now;
        }

        // Any blob in any level box = "Lv." text detected.
        bool any_level = false;
        for (size_t i = 0; i < 6; i++){
            bool level_i = blob_per_box[i] > 0;
            m_ov_level[i]->color = level_i ? COLOR_GREEN : COLOR_RED;
            if (level_i) any_level = true;
        }

        return any_level;
    }
private:
    Logger& m_logger;
    VideoOverlay& m_overlay;
    WallClock m_last_log;
    bool m_notification_seen;
    bool m_phase1_started;
    bool m_phase2_started;
    std::optional<OverlayBoxScope> m_ov_xmark;
    std::optional<OverlayBoxScope> m_ov_text;
    std::optional<OverlayBoxScope> m_ov_level[6];
};

// ── HP bar detection (Pokemon in targeting mode) ───────────────────────────

static const ImageFloatBox HP_BAR_BOX{0.838000, 0.957000, 0.085000, 0.009000};

// Returns true when the green HP bar is visible (pokemon is in hand, not thrown).
static bool is_hp_bar_visible(const ImageViewRGB32& frame){
    const double scale = frame.height() / 1080.0;
    const size_t min_sz = std::max<size_t>(1, size_t(50 * scale * scale));
    // HP bar green in 0-255: R≈46-67, G≈201-215, B≈125-132
    // Tight range excludes PLA grass (yellow-green: high R, G<180).
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        extract_box_reference(frame, HP_BAR_BOX),
        combine_rgb(35, 185, 110),
        combine_rgb(80, 225, 145)
    );
    auto session = Kernels::Waterfill::make_WaterfillSession(matrix);
    auto finder  = session->make_iterator(min_sz);
    Kernels::Waterfill::WaterfillObject obj;
    return finder->find_next(obj, false);
}

// Watches the HP bar after a throw. The bar disappears when the pokemon is in
// flight; if it stays gone for 3 s the pokemon missed, so ZR is pressed to
// recall it. Returns true (stopping wait_until) when the recall is sent.
// Fight/berry detectors remain fully independent and take priority.
class HpBarManager : public VisualInferenceCallback{
public:
    HpBarManager(Logger& logger, VideoOverlay& overlay, ProControllerContext& context)
        : VisualInferenceCallback("HpBarManager")
        , m_logger(logger)
        , m_ov(overlay, HP_BAR_BOX, COLOR_CYAN)
        , m_context(context)
        , m_phase(Phase::WAITING_FOR_DISAPPEAR)
        , m_gone_since(WallClock::min())
    {}
    virtual void make_overlays(VideoOverlaySet&) const override{}
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock now) override{
        bool visible = is_hp_bar_visible(frame);
        switch (m_phase){
        case Phase::WAITING_FOR_DISAPPEAR:
            if (!visible){
                m_phase = Phase::BAR_GONE;
                m_gone_since = now;
                m_logger.log("HpBarManager: bar disappeared, throw confirmed.");
            }
            return false;
        case Phase::BAR_GONE:
            if (visible){
                // Pokemon returned without triggering fight or berries — counts as an
                // attempt regardless of how quickly the ball bounced back.
                m_logger.log("HpBarManager: bar reappeared with no result, retrying.");
                return true;
            }
            if (now - m_gone_since >= 2000ms){
                m_logger.log("HpBarManager: 2s elapsed, recalling pokemon.");
                pbf_press_button(m_context, BUTTON_ZR, 160ms, 0ms);
                m_context.wait_for_all_requests();
                m_phase = Phase::RECALLED;
                return true;
            }
            return false;
        case Phase::RECALLED:
            return true;
        }
        return false;
    }
private:
    enum class Phase{ WAITING_FOR_DISAPPEAR, BAR_GONE, RECALLED };
    Logger&               m_logger;
    OverlayBoxScope       m_ov;
    ProControllerContext& m_context;
    Phase                 m_phase;
    WallClock             m_gone_since;
};


// ── Map pin detection ──────────────────────────────────────────────────────

static const ImageFloatBox MAP_PIN_DIALOG_BOX{0.470, 0.050, 0.100, 0.150};
static const ImageFloatBox FLAG_PIN_BOX      {0.525, 0.020, 0.030, 0.050};

class PinDialogArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    PinDialogArrowMatcher()
        : WaterfillTemplateMatcher(
            "PokemonLA/YellowArrowRight-Template.png",
            Color(0xff808008), Color(0xffffffff), 200
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower   = 0.9;
        m_area_ratio_upper   = 1.1;
    }
    static const PinDialogArrowMatcher& instance(){
        static PinDialogArrowMatcher s;
        return s;
    }
};

class PinDialogDetector : public VisualInferenceCallback{
public:
    PinDialogDetector(VideoStream& stream)
        : VisualInferenceCallback("PinDialogDetector")
        , m_overlay(stream.overlay(), MAP_PIN_DIALOG_BOX, COLOR_GREEN)
    {}
    virtual void make_overlays(VideoOverlaySet& /*items*/) const override{}
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock /*ts*/) override{
        const double scale  = frame.height() / 1080.0;
        const size_t min_sz = size_t(200 * scale * scale);
        const std::vector<std::pair<uint32_t,uint32_t>> filters = {
            {combine_rgb(160,160,0), combine_rgb(255,255, 80)},
            {combine_rgb(200,200,0), combine_rgb(255,255,255)},
            {combine_rgb(200,200,0), combine_rgb(255,255,180)},
        };
        return match_template_by_waterfill(
            frame.size(),
            extract_box_reference(frame, MAP_PIN_DIALOG_BOX),
            PinDialogArrowMatcher::instance(),
            filters,
            {min_sz, SIZE_MAX},
            80.0,
            [](Kernels::Waterfill::WaterfillObject&) -> bool { return true; }
        );
    }
private:
    OverlayBoxScope m_overlay;
};

class FlagPinMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    FlagPinMatcher()
        : WaterfillTemplateMatcher(
            "PokemonLA/PinAtMapTop.png",
            Color(0xff808000), Color(0xffffffff), 20
        )
    {
        m_aspect_ratio_lower = 0.4;
        m_aspect_ratio_upper = 2.5;
        m_area_ratio_lower   = 0.4;
        m_area_ratio_upper   = 2.5;
    }
    static const FlagPinMatcher& instance(){
        static FlagPinMatcher s;
        return s;
    }
};

class FlagPinDetector : public VisualInferenceCallback{
public:
    FlagPinDetector(VideoStream& stream)
        : VisualInferenceCallback("FlagPinDetector")
        , m_overlay(stream.overlay(), FLAG_PIN_BOX, COLOR_RED)
    {}
    virtual void make_overlays(VideoOverlaySet& /*items*/) const override{}
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock /*ts*/) override{
        const double scale  = frame.height() / 1080.0;
        const size_t min_sz = std::max<size_t>(1, size_t(5 * scale * scale));
        const std::vector<std::pair<uint32_t,uint32_t>> filters = {
            {combine_rgb(65,60,0), combine_rgb(255,255,50)},
        };
        return match_template_by_waterfill(
            frame.size(),
            extract_box_reference(frame, FLAG_PIN_BOX),
            FlagPinMatcher::instance(),
            filters,
            {min_sz, SIZE_MAX},
            80.0,
            [](Kernels::Waterfill::WaterfillObject&) -> bool { return true; }
        );
    }
private:
    OverlayBoxScope m_overlay;
};

// ── Flag orientation ───────────────────────────────────────────────────────

class FlagDetectedStopper : public VisualInferenceCallback{
public:
    explicit FlagDetectedStopper(FlagTracker& tracker)
        : VisualInferenceCallback("FlagDetectedStopper")
        , m_tracker(tracker)
    {}
    virtual void make_overlays(VideoOverlaySet&) const override{}
    virtual bool process_frame(const ImageViewRGB32&, WallClock timestamp) override{
        double d, x, y;
        return m_tracker.get(d, x, y, timestamp);
    }
private:
    FlagTracker& m_tracker;
};

class FlagThresholdStopper : public VisualInferenceCallback{
public:
    FlagThresholdStopper(FlagTracker& tracker, bool stop_when_le_center)
        : VisualInferenceCallback("FlagThresholdStopper")
        , m_tracker(tracker)
        , m_stop_when_le_center(stop_when_le_center)
    {}
    virtual void make_overlays(VideoOverlaySet&) const override{}
    virtual bool process_frame(const ImageViewRGB32&, WallClock timestamp) override{
        double d, x, y;
        if (!m_tracker.get(d, x, y, timestamp)) return false;
        return m_stop_when_le_center ? (x <= 0.5) : (x >= 0.5);
    }
private:
    FlagTracker& m_tracker;
    bool m_stop_when_le_center;
};

}  // anonymous namespace


// ── Helpers ────────────────────────────────────────────────────────────────

// Alternates left/right aim nudges with increasing magnitude on retries.
static void adjust_aim(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int attempt){
    double shift = (attempt % 2 == 0) ? -1 : +1;
    auto time_increase = std::chrono::milliseconds(150 * attempt + 200);
    env.log("adjust_aim: shift=" + std::to_string(shift));
    pbf_move_right_joystick(context, {shift, 0.0}, time_increase, 100ms);
    context.wait_for_all_requests();
}


static TreeOutcome throw_at_tree(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    int attempt
){
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);
    if (attempt > 0) adjust_aim(env, context, attempt);

    pbf_press_button(context, BUTTON_ZR, 500ms, 0ms);
    context.wait_for_all_requests();

    BattleMenuDetector battle_detector(env.console, env.console, true);
    BerryDropDetector  berry_detector(env.console, env.console.overlay());
    HpBarManager       hp_bar_manager(env.console, env.console.overlay(), context);

    // 30 s outer ceiling; HpBarManager fires ZR recall after 3 s of bar absence.
    int triggered = wait_until(
        env.console, context, 30000ms,
        {{battle_detector}, {berry_detector}, {hp_bar_manager}}
    );

    if (triggered == 0){
        env.log("throw_at_tree: FIGHT detected.");
        return TreeOutcome::FIGHT;
    }
    if (triggered == 1){
        env.log("throw_at_tree: BERRIES detected.");
        return TreeOutcome::BERRIES;
    }
    env.log("throw_at_tree: NOTHING detected, pokemon recalled.");
    return TreeOutcome::NOTHING;
}


static void exit_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.log("exit_battle: running away.");
    pbf_press_button(context, BUTTON_B, 160ms, 1800ms);
    pbf_press_button(context, BUTTON_A, 160ms, 4300ms);
    context.wait_for_all_requests();
}


static void ensure_pokemon_menu(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    static const ImageFloatBox BOX0(0.850, 0.878, 0.006, 0.021);
    static const ImageFloatBox BOX1(0.905, 0.878, 0.006, 0.021);

    auto menu_brightness = [&]() -> double {
        VideoSnapshot snap = env.console.video().snapshot();
        if (!snap.frame) return 0.0;
        ImageStats s0 = image_stats(extract_box_reference(*snap.frame, BOX0));
        ImageStats s1 = image_stats(extract_box_reference(*snap.frame, BOX1));
        return (s0.average.r + s0.average.g + s0.average.b +
                s1.average.r + s1.average.g + s1.average.b) / 6.0;
    };

    double before = menu_brightness();
    env.log("ensure_pokemon_menu: before brightness=" + std::to_string(before));
    pbf_press_button(context, BUTTON_X, 160ms, 500ms);
    context.wait_for_all_requests();
    double after = menu_brightness();
    env.log("ensure_pokemon_menu: after brightness=" + std::to_string(after));

    if (before > after){
        // Was on items (lighter), switched to pokemon (darker) — done.
        env.log("ensure_pokemon_menu: switched from items to pokemon.");
    } else {
        // Was already on pokemon (darker), switched to items (lighter) — switch back.
        env.log("ensure_pokemon_menu: was already on pokemon, switching back.");
        pbf_press_button(context, BUTTON_X, 160ms, 1840ms);
        context.wait_for_all_requests();
    }
}


static void set_flagpin_on_map(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    while (true){
        MapDetector       map_open;
        OverworldDetector overworld(env.console, env.console);
        context.wait_for_all_requests();
        pbf_wait(context, 500ms);
        int ret = wait_until(env.console, context, std::chrono::seconds(3),
            {{map_open}, {overworld}});
        switch (ret){
        case 0:{
            env.log("Map open");
            bool pin_placed = false;
            while (!pin_placed){
                FlagPinDetector   flag_pin(env.console);
                PinDialogDetector pin_dialog(env.console);
                context.wait_for_all_requests();
                int ret2 = wait_until(env.console, context, std::chrono::seconds(3),
                    {{flag_pin}, {pin_dialog}});
                switch (ret2){
                case 0:
                    env.log("Pin placed, returning to overworld...");
                    pin_placed = true;
                    break;
                case 1:
                    env.log("Pin dialog box detected, placing pin");
                    pbf_press_button(context, BUTTON_A, 500ms, 300ms);
                    pbf_move_left_joystick(context, {0.0, -1.0}, 2000ms, 300ms);
                    pbf_wait(context, 1000ms);
                    break;
                default:
                    env.log("No pin nor pin dialog box detected, setting up pin...");
                    pbf_move_left_joystick(context, {0.0, +1.0}, 2000ms, 300ms);
                    pbf_press_button(context, BUTTON_A, 500ms, 300ms);
                    break;
                }
            }
            OverworldDetector ow(env.console, env.console);
            run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& ctx){ pbf_mash_button(ctx, BUTTON_B, 30000ms); },
                {{ow}}
            );
            return;
        }
        case 1:
            pbf_press_button(context, BUTTON_MINUS, 500ms, 300ms);
            continue;
        default:
            continue;
        }
    }
}


static void orient_to_flag(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    FlagTracker         flag_tracker(env.console, env.console);
    FlagDetectedStopper detected_stopper(flag_tracker);

    // Mirror find_flag from FlagNavigationAir: ZL to level camera, then Z-pattern sweep
    // (horizontal + vertical zig-zags). Try right first, then left if not found.
    pbf_mash_button(context, BUTTON_ZL, 1700ms);

    bool found = false;
    for (double dir : {+1.0, -1.0}){
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [dir](ProControllerContext& ctx){
                pbf_mash_button(ctx, BUTTON_ZL, 300ms);
                pbf_move_right_joystick(ctx, {dir,  0.0}, 3200ms, 0ms);
                pbf_move_right_joystick(ctx, {0.0, -1.0},  960ms, 0ms);
                pbf_move_right_joystick(ctx, {dir,  0.0}, 3200ms, 0ms);
                pbf_move_right_joystick(ctx, {0.0, +1.0}, 1600ms, 0ms);
                pbf_move_right_joystick(ctx, {dir,  0.0}, 3200ms, 0ms);
            },
            {{detected_stopper, flag_tracker}}
        );
        pbf_wait(context, 300ms);
        context.wait_for_all_requests();
        if (ret == 0){ found = true; break; }
    }
    if (!found){
        env.log("orient_to_flag: flag not found after full sweep.");
        return;
    }

    // Centering: nudges at 0.5 magnitude; 500ms when far (>5%), 250ms when close.
    for (int step = 0; step < 100; step++){
        double distance, flag_x, flag_y;
        if (!flag_tracker.get(distance, flag_x, flag_y)){
            env.log("orient_to_flag: flag lost at centering step " + std::to_string(step));
            break;
        }
        env.log("orient_to_flag: centering step " + std::to_string(step) + " flag_x=" + std::to_string(flag_x));
        if (std::abs(flag_x - 0.5) < 0.02){
            env.log("orient_to_flag: centered.");
            context.wait_for_all_requests();
            pbf_wait(context, 1000ms);
            return;
        }

        double dir = (flag_x > 0.5) ? +0.5 : -0.5;
        double dir_corr = 0;
        auto nudge = (std::abs(flag_x - 0.5) > 0.15)
            ? std::chrono::milliseconds(500)
            : std::chrono::milliseconds(250);
        dir_corr = (std::abs(flag_x - 0.5) > 0.15) ? dir : dir * 0.5;
        run_until<ProControllerContext>(
            env.console, context,
            [dir_corr, nudge](ProControllerContext& ctx){
                pbf_move_right_joystick(ctx, {dir_corr, 0.0}, nudge, 100ms);
            },
            {{flag_tracker}}
        );
        context.wait_for_all_requests();
    }
    env.log("orient_to_flag: could not center after 20 steps.");
}


static void slow_landing(ConsoleHandle& console, ProControllerContext& context, int cycles){
    for (int i = 0; i < cycles; i++){
        change_mount(console, context, MountState::BRAVIARY_OFF);
        pbf_wait(context, 250ms);
        change_mount(console, context, MountState::BRAVIARY_ON);
    }
    change_mount(console, context, MountState::BRAVIARY_OFF);
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();
}


// ── Tree navigation paths ──────────────────────────────────────────────────
//
// Each entry is a lambda that runs all the steps needed to reach tree[index]
// from the previous tree (or spawn point for index 0).
// Write each step as a plain function call — no extra lambda syntax needed.
//
// Example:
//   [](auto& env, auto& context){
//       fast_travel_from_overworld(env, env.console, context,
//           TravelLocations::instance().Fieldlands_Arena);
//       pbf_move_left_joystick(context, {0.0, -1.0}, 2000ms, 0ms);
//       pbf_move_right_joystick(context, {1.0, 0.0}, 500ms, 0ms);
//       pbf_press_button(context, BUTTON_B, 160ms, 0ms);
//   },
//
using NavStep = std::function<void(SingleSwitchProgramEnvironment&, ProControllerContext&)>;

static const std::vector<NavStep> TREE_PATHS = {
    // Tree 1: from spawn/save point  ← fill in after hardware test
    [](auto& env, auto& context){
        fast_travel_from_overworld(env, env.console, context, TravelLocations::instance().Fieldlands_Arena);
        context.wait_for_all_requests();
        pbf_wait(context, 500ms);
        change_mount(env.console, context, MountState::BRAVIARY_ON);
        context.wait_for_all_requests();
        pbf_move_right_joystick(context, {-1.0, 0.0}, 1150ms, 300ms);
        pbf_move_left_joystick(context, {0.0, +1.0}, 2800ms, 300ms);
        context.wait_for_all_requests();
        change_mount(env.console, context, MountState::BRAVIARY_OFF);
        context.wait_for_all_requests();
        pbf_wait(context, 500ms);

        change_mount(env.console, context, MountState::BRAVIARY_ON);
        pbf_move_right_joystick(context, {+0.5, 0.0}, 600ms, 300ms);
        pbf_move_left_joystick(context, {0.0, +1.0}, 1000ms, 300ms);
        pbf_press_button(context, BUTTON_B, 6500ms, 160ms);
        change_mount(env.console, context, MountState::BRAVIARY_OFF);
        pbf_wait(context, 1000ms);
        context.wait_for_all_requests();

        change_mount(env.console, context, MountState::BRAVIARY_ON);
        pbf_move_right_joystick(context, {-0.5, 0.0}, 700ms, 300ms);
        pbf_move_left_joystick(context, {0.0, +1.0}, 1000ms, 300ms);
        pbf_press_button(context, BUTTON_B, 7500ms, 160ms);
        change_mount(env.console, context, MountState::BRAVIARY_OFF);
        context.wait_for_all_requests();

        change_mount(env.console, context, MountState::BRAVIARY_ON);
        pbf_move_right_joystick(context, {-0.5, 0.0}, 575ms, 300ms);
        pbf_move_left_joystick(context, {0.0, +1.0}, 1000ms, 300ms);
        pbf_press_button(context, BUTTON_B, 4000ms, 160ms);
        change_mount(env.console, context, MountState::BRAVIARY_OFF);
        pbf_move_right_joystick(context, {0.0, -0.5}, 400ms, 300ms);
        context.wait_for_all_requests();
    },
    // Tree 2: from tree 0            ← fill in after hardware test
    [](auto& env, auto& context){
        context.wait_for_all_requests();
        pbf_wait(context, 500ms);
        change_mount(env.console, context, MountState::BRAVIARY_ON);
        orient_to_flag(env, context);
        context.wait_for_all_requests();
        pbf_move_right_joystick(context, {-0.5, 0.0}, 1000ms, 300ms);
        pbf_move_left_joystick(context, {0.0, +1.0}, 1000ms, 300ms);
        pbf_press_button(context, BUTTON_B, 4100ms, 300ms);
        slow_landing(env.console, context, 1);
        context.wait_for_all_requests();
    },
    // Tree 3
    [](auto& env, auto& context){
        context.wait_for_all_requests();
        pbf_wait(context, 500ms);
        change_mount(env.console, context, MountState::BRAVIARY_ON);
        orient_to_flag(env, context);
        context.wait_for_all_requests();
        pbf_move_right_joystick(context, {+0.5, 0.0}, 1400ms, 300ms);
        pbf_move_left_joystick(context, {0.0, +1.0}, 1000ms, 300ms);
        pbf_press_button(context, BUTTON_B, 5300ms, 300ms);
        slow_landing(env.console, context, 3);
        pbf_move_right_joystick(context, {0.0, -0.5}, 400ms, 300ms);
        context.wait_for_all_requests();
    },
    // Tree 4
    [](auto& env, auto& context){
        fast_travel_from_overworld(env, env.console, context, TravelLocations::instance().Fieldlands_Heights);
        context.wait_for_all_requests();
        pbf_wait(context, 500ms);
        change_mount(env.console, context, MountState::BRAVIARY_ON);
        pbf_move_right_joystick(context, {-0.5, 0.0}, 1700ms, 300ms);
        pbf_move_left_joystick(context, {0.0, +1.0}, 900ms, 300ms);
        pbf_press_button(context, BUTTON_B, 5100ms, 300ms);
        slow_landing(env.console, context, 1);
        pbf_move_right_joystick(context, {0.0, -0.5}, 500ms, 300ms);
    },
};


static void navigate_to_tree(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    int index
){
    if (index >= (int)TREE_PATHS.size()){
        env.log("navigate_to_tree: no path defined for tree " + std::to_string(index) + ".");
        return;
    }
    TREE_PATHS[(size_t)index](env, context);
    context.wait_for_all_requests();
}


// ── Main program ───────────────────────────────────────────────────────────

void TreeShaker::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    const int num_trees    = (int)TREE_PATHS.size();
    const int max_attempts = MAX_THROW_ATTEMPTS;
    const int start_tree   = std::max(0, (int)START_TREE - 1);

    std::vector<TreeState> tree_states((size_t)num_trees, TreeState::UNKNOWN);

    // Connect the controller
    require_player(env.console, context, BUTTON_L);

    while (true){
        
        // ── Fast travel to Heights Camp ──────────────────────────────────────────────────
        env.log("Going to Heights Camp...");
        fast_travel_from_overworld(env, env.console, context, TravelLocations::instance().Fieldlands_Heights);
        
        // ── Save game ──────────────────────────────────────────────────
        context.wait_for_all_requests();
        env.log("Saving game...");
        //save_game_from_overworld(env, env.console, context);

        // ── Place map pin + orient to flag ────────────────────────────
        env.log("Placing map pin...");
        set_flagpin_on_map(env, context);

        // ── Pass 1: visit every tree and discover its state ─────────────
        env.log("Pass 1: reconnaissance (starting from tree " + std::to_string(start_tree + 1) + ").");
        ensure_pokemon_menu(env, context);
        for (int i = start_tree; i < num_trees; i++){
            env.log("Tree " + std::to_string(i + 1) + "/" + std::to_string(num_trees) + ": approaching.");
            navigate_to_tree(env, context, i);

            TreeOutcome outcome = TreeOutcome::NOTHING;
            for (int attempt = 0; attempt < max_attempts; attempt++){
                outcome = throw_at_tree(env, context, attempt);
                if (outcome == TreeOutcome::FIGHT){
                    tree_states[(size_t)i] = TreeState::SHAKING;
                    exit_battle(env, context);
                    ensure_pokemon_menu(env, context);
                    break;
                }
                if (outcome == TreeOutcome::BERRIES){
                    tree_states[(size_t)i] = TreeState::BERRIES;
                    break;
                }
            }
            if (outcome == TreeOutcome::NOTHING){
                tree_states[(size_t)i] = TreeState::BERRIES;
                env.log("Tree " + std::to_string(i + 1) + ": max attempts reached, treating as BERRIES.");
            }

            env.log(
                "Tree " + std::to_string(i + 1) + " state: " +
                (tree_states[(size_t)i] == TreeState::SHAKING ? "SHAKING" : "BERRIES")
            );
        }

        // ── Stop condition ─────────────────────────────────────────────
        bool all_shaking = std::all_of(
            tree_states.begin(), tree_states.end(),
            [](TreeState s){ return s == TreeState::SHAKING; }
        );
        if (all_shaking){
            env.log("All trees are shaking! Program complete.");
            break;
        }

        // ── Reset to saved game ────────────────────────────────────────
        env.log("Resetting game to saved state...");
        reset_game_from_home(env, env.console, context);

        // ── Place map pin + orient to flag ────────────────────────────
        env.log("Placing map pin...");
        set_flagpin_on_map(env, context);
        env.log("Orienting to flag...");
        change_mount(env.console, context, MountState::BRAVIARY_ON);

        // ── Pass 1: shake only BERRIES trees (no detection needed) ──────
        env.log("Pass 2: shaking BERRIES trees.");
                ensure_pokemon_menu(env, context);
        for (int i = 0; i < num_trees; i++){
            if (tree_states[(size_t)i] == TreeState::BERRIES){
                env.log("Tree " + std::to_string(i + 1) + ": shaking.");
                navigate_to_tree(env, context, i);
                pbf_press_button(context, BUTTON_ZR, 500ms, 160ms);
                pbf_wait(context, 4500ms);
                context.wait_for_all_requests();
            }
        }

        // ── Wait 20 real-time minutes ──────────────────────────────────
        env.log("Waiting 20 minutes for trees to spawn Pokemon...");
        for (int minute = 0; minute < 20; minute++){
            env.log("Waiting... " + std::to_string(20 - minute) + " minute(s) remaining.");
            pbf_wait(context, 60000ms);
            context.wait_for_all_requests();
        }

        // ── Hard save ──────────────────────────────────────────────────
        env.log("Saving game after wait...");
        save_game_from_overworld(env, env.console, context);
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
