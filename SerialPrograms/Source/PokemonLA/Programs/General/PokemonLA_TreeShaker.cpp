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

class ItemNotificationDetector : public VisualInferenceCallback{
public:
    ItemNotificationDetector(VideoOverlay& overlay, const ImageFloatBox& box)
        : VisualInferenceCallback("ItemNotificationDetector")
        , m_box(box)
        , m_overlay(overlay, m_box, COLOR_CYAN)
        , m_detected(false)
    {}

    bool detected() const{ return m_detected; }

    virtual void make_overlays(VideoOverlaySet& /*items*/) const override{}

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock /*timestamp*/) override{
        ImageViewRGB32 region = extract_box_reference(frame, m_box);
        ImageStats stats = image_stats(region);
        double brightness = (stats.average.r + stats.average.g + stats.average.b) / 3.0;
        if (brightness > 180.0){
            m_detected = true;
            return true;
        }
        return false;
    }

private:
    ImageFloatBox m_box;
    OverlayBoxScope m_overlay;
    bool m_detected;
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
    PinDialogDetector()
        : VisualInferenceCallback("PinDialogDetector")
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
    FlagPinDetector()
        : VisualInferenceCallback("FlagPinDetector")
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

}  // anonymous namespace


// ── Helpers ────────────────────────────────────────────────────────────────

static TreeOutcome throw_at_tree(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    const ImageFloatBox& item_box
){
    pbf_press_button(context, BUTTON_ZR, 500ms, 160ms);
    pbf_wait(context, 4500ms);
    context.wait_for_all_requests();

    BattleMenuDetector battle_detector(env.console, env.console, true);
    ItemNotificationDetector notif_detector(env.console, item_box);

    int triggered = wait_until(
        env.console, context, 8000ms,
        {{battle_detector}, {notif_detector}}
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
    for (size_t c = 0; true; c++){
        context.wait_for_all_requests();
        if (is_pokemon_selection(env.console, env.console.video().snapshot())){
            break;
        }
        if (c >= 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to switch to Pokemon selection after 5 attempts.",
                env.console
            );
        }
        env.log("Not on Pokemon selection, switching...", COLOR_ORANGE);
        pbf_press_button(context, BUTTON_X, 160ms, 1840ms);
    }
}


static void set_flagpin_on_map(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    while (true){
        MapDetector       map_open;
        OverworldDetector overworld(env.console, env.console);
        context.wait_for_all_requests();
        pbf_wait(context, 3000ms);
        int ret = wait_until(env.console, context, std::chrono::seconds(3),
            {{map_open}, {overworld}});
        switch (ret){
        case 0:{
            bool pin_placed = false;
            while (!pin_placed){
                FlagPinDetector   flag_pin;
                PinDialogDetector pin_dialog;
                context.wait_for_all_requests();
                int ret2 = wait_until(env.console, context, std::chrono::seconds(3),
                    {{flag_pin}, {pin_dialog}});
                switch (ret2){
                case 0:
                    pin_placed = true;
                    break;
                case 1:
                    pbf_press_button(context, BUTTON_A, 500ms, 300ms);
                    pbf_move_left_joystick(context, {0.0, -1.0}, 2000ms, 300ms);
                    pbf_wait(context, 1000ms);
                    break;
                default:
                    pbf_move_left_joystick(context, {0.0, +1.0}, 2000ms, 300ms);
                    pbf_press_button(context, BUTTON_A, 500ms, 300ms);
                    break;
                }
            }
            pbf_mash_button(context, BUTTON_B, 3000ms);
            { OverworldDetector ow(env.console, env.console);
              wait_until(env.console, context, std::chrono::seconds(30), {{ow}}); }
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
    FlagDetectedStopper stopper(flag_tracker);

    auto sweep = [&](uint32_t duration_ms, bool use_zl = false){
        run_until<ProControllerContext>(
            env.console, context,
            [duration_ms, use_zl](ProControllerContext& ctx){
                if (use_zl){ pbf_mash_button(ctx, BUTTON_ZL, 2000ms); }
                pbf_move_right_joystick(ctx, {+1.0, 0.0}, Milliseconds(duration_ms), 0ms);
            },
            {{flag_tracker}, {stopper}}
        );
    };

    sweep(3200, true);
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();
    sweep(600);

    for (int attempt = 0; attempt < 8; attempt++){
        double distance, flag_x, flag_y;
        if (!flag_tracker.get(distance, flag_x, flag_y)){
            env.log("orient_to_flag: flag not visible, re-sweeping...");
            sweep(2000);
            continue;
        }
        env.log("orient_to_flag: flag at x=" + std::to_string(flag_x));
        double error    = std::abs(flag_x - 0.5);
        if (error < 0.05){
            env.log("orient_to_flag: centered.");
            return;
        }
        int64_t corr_ms      = std::max<int64_t>(30, (int64_t)(640.0 * error * 0.6));
        double  joystick_mag = 1.0 / (1 << attempt);
        if (flag_x < 0.5){
            pbf_move_right_joystick(context, {-joystick_mag, 0.0}, Milliseconds(corr_ms), 0ms);
        }else{
            pbf_move_right_joystick(context, {+joystick_mag, 0.0}, Milliseconds(corr_ms), 0ms);
        }
        context.wait_for_all_requests();
        pbf_wait(context, 500ms);
        context.wait_for_all_requests();
        sweep(600);
    }
    env.log("orient_to_flag: could not center after 8 attempts.");
}


// ── Tree navigation paths ──────────────────────────────────────────────────
//
// Each entry is a sequence of steps executed in order to reach tree[index]
// from the previous tree (or spawn point for index 0).
//
// Each step is a lambda with (env, context). Any pbf call or fast-travel
// function is valid.
//
// Examples:
//   [](auto& env, auto& context){ pbf_move_left_joystick(context, {0.0, -1.0}, 2000ms, 0ms); }
//   [](auto& env, auto& context){ pbf_move_right_joystick(context, {1.0, 0.0}, 500ms, 0ms); }
//   [](auto& env, auto& context){ pbf_press_button(context, BUTTON_B, 160ms, 0ms); }
//   [](auto& env, auto& context){ fast_travel_from_overworld(env, env.console, context,
//                                     TravelLocations::instance().Fieldlands_Fieldlands); }
//   [](auto& env, auto& context){ fast_travel_from_overworld(env, env.console, context,
//                                     TravelLocations::instance().Fieldlands_Heights); }
//   [](auto& env, auto& context){ fast_travel_from_overworld(env, env.console, context,
//                                     TravelLocations::instance().Fieldlands_Arena); }
//
using NavStep = std::function<void(SingleSwitchProgramEnvironment&, ProControllerContext&)>;

static const std::vector<std::vector<NavStep>> TREE_PATHS = {
    // Tree 0: from spawn/save point  ← fill in after hardware test
    {
        [](auto& env, auto& context){ fast_travel_from_overworld(env, env.console, context, TravelLocations::instance().Fieldlands_Arena); },
        [](auto& /*env*/, auto& context){ context.wait_for_all_requests(); },
        [](auto& /*env*/, auto& context){ pbf_wait(context, 500ms); },
        [](auto& /*env*/, auto& context){ pbf_move_right_joystick(context, {-1.0, 0.0}, 400ms, 300ms); },
        [](auto& /*env*/, auto& context){ pbf_move_left_joystick(context, {0.0, +1.0}, 3000ms, 300ms); },
    },
    // Tree 1: from tree 0            ← fill in after hardware test
    {
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
    for (const NavStep& step : TREE_PATHS[(size_t)index]){
        step(env, context);
    }
    context.wait_for_all_requests();
}


// ── Main program ───────────────────────────────────────────────────────────

void TreeShaker::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    const ImageFloatBox item_box(
        ITEM_NOTIFICATION_BOX.x(), ITEM_NOTIFICATION_BOX.y(),
        ITEM_NOTIFICATION_BOX.width(), ITEM_NOTIFICATION_BOX.height()
    );
    const int num_trees    = (int)TREE_PATHS.size();
    const int max_attempts = MAX_THROW_ATTEMPTS;

    std::vector<TreeState> tree_states((size_t)num_trees, TreeState::UNKNOWN);

    // Connect the controller
    require_player(env.console, context, BUTTON_L);

    while (true){
        // ── Save game ──────────────────────────────────────────────────
        env.log("Saving game...");
        save_game_from_overworld(env, env.console, context);

        // ── Place map pin + orient to flag ────────────────────────────
        env.log("Placing map pin...");
        set_flagpin_on_map(env, context);
        env.log("Orienting to flag...");
        change_mount(env.console, context, MountState::BRAVIARY_ON);
        orient_to_flag(env, context);

        // ── Pass 1: visit every tree and discover its state ─────────────
        env.log("Pass 1: reconnaissance.");
        ensure_pokemon_menu(env, context);
        for (int i = 0; i < num_trees; i++){
            env.log("Tree " + std::to_string(i + 1) + "/" + std::to_string(num_trees) + ": approaching.");
            navigate_to_tree(env, context, i);

            TreeOutcome outcome = TreeOutcome::NOTHING;
            for (int attempt = 0; attempt < max_attempts; attempt++){
                outcome = throw_at_tree(env, context, item_box);
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
                if (attempt + 1 < max_attempts){
                    adjust_aim(env, context, attempt);
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
        orient_to_flag(env, context);

        // ── Pass 2: shake only BERRIES trees (no detection needed) ──────
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
