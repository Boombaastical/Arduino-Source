/*  Tree Shaker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <functional>
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
        5, 1, 10
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
    PA_ADD_OPTION(ITEM_NOTIFICATION_BOX);
    PA_ADD_OPTION(NOTIFICATIONS);
}


// ── Internal types ─────────────────────────────────────────────────────────

enum class TreeOutcome { FIGHT, BERRIES, NOTHING };
enum class TreeState   { UNKNOWN, SHAKING, BERRIES };

namespace{

// ── Berry drop detection ───────────────────────────────────────────────────

static const ImageFloatBox BERRY_XMARK_BOX{0.935, 0.605, 0.017, 0.028};
static const ImageFloatBox BERRY_DARK_BOX {0.800, 0.576, 0.040, 0.031};
static const ImageFloatBox BERRY_LEVEL_BOX{0.060, 0.365, 0.022, 0.030};

class BerriesXMarkMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    BerriesXMarkMatcher() : WaterfillTemplateMatcher(
        "PokemonLA/BerriesXMark.png",
        Color(0xffc8c8c8), Color(0xffffffff), 10
    ){
        m_aspect_ratio_lower = 0.5;
        m_aspect_ratio_upper = 2.0;
        m_area_ratio_lower   = 0.5;
        m_area_ratio_upper   = 2.0;
    }
    static const BerriesXMarkMatcher& instance(){
        static BerriesXMarkMatcher m; return m;
    }
};

class PokemonLevelUpMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    PokemonLevelUpMatcher() : WaterfillTemplateMatcher(
        "PokemonLA/PokemonLevelUp.png",
        Color(0xffc8c8c8), Color(0xffffffff), 10
    ){
        m_aspect_ratio_lower = 0.5;
        m_aspect_ratio_upper = 2.0;
        m_area_ratio_lower   = 0.5;
        m_area_ratio_upper   = 2.0;
    }
    static const PokemonLevelUpMatcher& instance(){
        static PokemonLevelUpMatcher m; return m;
    }
};

class BerryDropDetector : public VisualInferenceCallback{
public:
    BerryDropDetector(VideoOverlay& overlay)
        : VisualInferenceCallback("BerryDropDetector")
        , m_ov_xmark(overlay, BERRY_XMARK_BOX, COLOR_YELLOW)
        , m_ov_dark (overlay, BERRY_DARK_BOX,  COLOR_CYAN)
        , m_ov_level(overlay, BERRY_LEVEL_BOX, COLOR_MAGENTA)
    {}
    virtual void make_overlays(VideoOverlaySet&) const override{}
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock) override{
        const double scale = frame.height() / 1080.0;
        const size_t min_sz = std::max<size_t>(1, size_t(10 * scale * scale));
        const std::vector<std::pair<uint32_t,uint32_t>> white_filter = {
            {combine_rgb(200, 200, 200), combine_rgb(255, 255, 255)},
        };

        // Condition 1: white X mark
        bool xmark = match_template_by_waterfill(
            frame.size(),
            extract_box_reference(frame, BERRY_XMARK_BOX),
            BerriesXMarkMatcher::instance(),
            white_filter, {min_sz, SIZE_MAX}, 80.0,
            [](Kernels::Waterfill::WaterfillObject&){ return true; }
        );
        if (!xmark) return false;

        // Condition 2: dark overlay
        ImageStats dark_stats = image_stats(extract_box_reference(frame, BERRY_DARK_BOX));
        double brightness = (dark_stats.average.r + dark_stats.average.g + dark_stats.average.b) / 3.0;
        if (brightness >= 80.0) return false;

        // Condition 3: "Lv." text
        return match_template_by_waterfill(
            frame.size(),
            extract_box_reference(frame, BERRY_LEVEL_BOX),
            PokemonLevelUpMatcher::instance(),
            white_filter, {min_sz, SIZE_MAX}, 80.0,
            [](Kernels::Waterfill::WaterfillObject&){ return true; }
        );
    }
private:
    OverlayBoxScope m_ov_xmark;
    OverlayBoxScope m_ov_dark;
    OverlayBoxScope m_ov_level;
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

static TreeOutcome throw_at_tree(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    int attempt
){
    // Per-attempt camera nudge via right joystick (attempt 0 = first throw, no nudge).
    switch (attempt){
    case 0: break;
    case 1: /* pbf_move_right_joystick(context, {+0.5, 0.0}, 200ms, 0ms); */ break;
    case 2: /* pbf_move_right_joystick(context, {-0.5, 0.0}, 200ms, 0ms); */ break;
    case 3: /* pbf_move_right_joystick(context, {+1.0, 0.0}, 200ms, 0ms); */ break;
    case 4: /* pbf_move_right_joystick(context, {-1.0, 0.0}, 200ms, 0ms); */ break;
    default: break;
    }

    pbf_press_button(context, BUTTON_ZR, 500ms, 160ms);
    pbf_wait(context, 4500ms);
    context.wait_for_all_requests();

    BattleMenuDetector battle_detector(env.console, env.console, true);
    BerryDropDetector  berry_detector(env.console.overlay());

    int triggered = wait_until(
        env.console, context, 3000ms,
        {{battle_detector}, {berry_detector}}
    );

    if (triggered == 0){
        env.log("throw_at_tree: FIGHT detected.");
        return TreeOutcome::FIGHT;
    }
    if (triggered == 1){
        env.log("throw_at_tree: BERRIES detected.");
        return TreeOutcome::BERRIES;
    }
    env.log("throw_at_tree: NOTHING detected.");
    return TreeOutcome::NOTHING;
}


static void exit_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.log("exit_battle: running away.");
    pbf_press_button(context, BUTTON_B, 160ms, 1800ms);
    pbf_press_button(context, BUTTON_A, 160ms, 4300ms);
    context.wait_for_all_requests();
}


// Alternates left/right aim nudges with increasing magnitude on retries.
static void adjust_aim(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int attempt){
    double magnitude = 0.1 * (attempt / 2 + 1);
    double shift = (attempt % 2 == 0) ? -magnitude : +magnitude;
    shift = std::max(-1.0, std::min(1.0, shift));
    env.log("adjust_aim: shift=" + std::to_string(shift));
    pbf_move_left_joystick(context, {shift, 0.0}, 100ms, 100ms);
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
    bool found = false;
    for (double dir : {+1.0, -1.0}){
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [dir](ProControllerContext& ctx){
                pbf_mash_button(ctx, BUTTON_ZL, 2000ms);
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

    // Centering: nudges at 0.25 magnitude; 500ms when far (>5%), 250ms when close.
    for (int step = 0; step < 20; step++){
        double distance, flag_x, flag_y;
        if (!flag_tracker.get(distance, flag_x, flag_y)){
            env.log("orient_to_flag: flag lost at centering step " + std::to_string(step));
            break;
        }
        env.log("orient_to_flag: centering step " + std::to_string(step) + " flag_x=" + std::to_string(flag_x));
        if (std::abs(flag_x - 0.5) < 0.02){
            env.log("orient_to_flag: centered.");
            return;
        }

        double dir = (flag_x > 0.5) ? +0.25 : -0.25;
        auto nudge = (std::abs(flag_x - 0.5) > 0.05)
            ? std::chrono::milliseconds(500)
            : std::chrono::milliseconds(250);
        run_until<ProControllerContext>(
            env.console, context,
            [dir, nudge](ProControllerContext& ctx){
                pbf_move_right_joystick(ctx, {dir, 0.0}, nudge, 0ms);
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
        pbf_wait(context, 250ms);
    }
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
    // Tree 0: from spawn/save point  ← fill in after hardware test
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
        context.wait_for_all_requests();

        change_mount(env.console, context, MountState::BRAVIARY_ON);
        pbf_move_right_joystick(context, {-0.5, 0.0}, 700ms, 300ms);
        pbf_move_left_joystick(context, {0.0, +1.0}, 1000ms, 300ms);
        pbf_press_button(context, BUTTON_B, 7500ms, 160ms);
        change_mount(env.console, context, MountState::BRAVIARY_OFF);
        context.wait_for_all_requests();

        change_mount(env.console, context, MountState::BRAVIARY_ON);
        pbf_move_right_joystick(context, {-0.5, 0.0}, 550ms, 300ms);
        pbf_move_left_joystick(context, {0.0, +1.0}, 1000ms, 300ms);
        pbf_press_button(context, BUTTON_B, 4000ms, 160ms);
        change_mount(env.console, context, MountState::BRAVIARY_OFF);
        context.wait_for_all_requests();
    },
    // Tree 1: from tree 0            ← fill in after hardware test
    [](auto& /*env*/, auto& /*context*/){
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
        env.log("Pass 1: reconnaissance.");
        ensure_pokemon_menu(env, context);
        for (int i = 0; i < num_trees; i++){
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
