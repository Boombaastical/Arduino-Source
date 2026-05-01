/*  BDSP AutoStory Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "Detect/PokemonBDSP_AutoStory_OverworldDetector.h"
#include "PokemonBDSP_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


const char* torterra_move_slug(TorterraMove move){
    switch (move){
    case TorterraMove::EARTHQUAKE:  return "earthquake";
    case TorterraMove::WOOD_HAMMER: return "wood-hammer";
    case TorterraMove::RAZOR_LEAF:  return "razor-leaf";
    case TorterraMove::CRUNCH:      return "crunch";
    case TorterraMove::GIGA_DRAIN:  return "giga-drain";
    case TorterraMove::IRON_TAIL:   return "iron-tail";
    case TorterraMove::ROCK_TOMB:   return "rock-tomb";
    case TorterraMove::FACADE:      return "facade";
    case TorterraMove::STONE_EDGE:  return "stone-edge";
    case TorterraMove::ROCK_SLIDE:  return "rock-slide";
    case TorterraMove::BULLDOZE:    return "bulldoze";
    case TorterraMove::STRENGTH:    return "strength";
    case TorterraMove::ROCK_SMASH:  return "rock-smash";
    case TorterraMove::ROCK_CLIMB:  return "rock-climb";
    }
    return "unknown";
}

void fake_save_game(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_X, 80ms, 1000ms);
    pbf_press_button(context, BUTTON_X, 80ms, 1000ms);
    pbf_press_button(context, BUTTON_R, 80ms, 1000ms);
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
}


void checkpoint_save(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    fake_save_game(env, context);
    {
        OverworldWatcher overworld;
        int ret = wait_until(env.console, context, std::chrono::seconds(30), {{overworld}});
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "checkpoint_save: overworld not detected within 30 seconds.",
                env.console
            );
        }
    }
    stats.m_checkpoint++;
    env.update_stats();
    send_program_status_notification(env, notif_status_update, "Saved at checkpoint.");
}


void checkpoint_reattempt_loop(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    std::function<void(size_t attempt_number)>&& action
){
    static constexpr size_t MAX_ATTEMPTS = 20;

    for (size_t i = 0;; i++){
        try{
            if (i == 0){
                checkpoint_save(env, context, notif_status_update, stats);
            }

            action(i);
            return; // success

        }catch (OperationFailedException& e){
            env.console.log("checkpoint_reattempt_loop: attempt " + std::to_string(i) + " failed: " + e.message(), COLOR_RED);
            stats.m_reset++;
            env.update_stats();

            if (i + 1 >= MAX_ATTEMPTS){
                throw;
            }
        }
    }
}



void repeat(std::initializer_list<std::function<void()>> actions, size_t times){
    for (size_t i = 0; i < times; i++){
        for (const auto& action : actions){
            action();
        }
    }
}


void wait_for_dialogue(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& label,
    Milliseconds timeout
){
    auto log_detected = [&]{
        if (!label.empty()){
            stream.log(label + " dialogue detected.");
        }
    };

    ShortDialogWatcher watcher_cyan(COLOR_CYAN);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [timeout](ProControllerContext& context){
            context.wait_for(timeout);
        },
        {{watcher_cyan}}
    );
    if (ret == 0){ log_detected(); return; }

    ShortDialogWatcher watcher_red(COLOR_RED);
    ret = run_until<ProControllerContext>(
        stream, context,
        [timeout](ProControllerContext& context){
            context.wait_for(timeout);
        },
        {{watcher_red}}
    );
    if (ret == 0){ log_detected(); return; }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        (label.empty() ? "wait_for_dialogue" : label) + ": dialogue not detected within timeout.",
        stream
    );
}


void mash_until_dialogue_ends(
    VideoStream& stream,
    ProControllerContext& context,
    Button button,
    Milliseconds timeout
){
    OverworldWatcher overworld;
    int ret = run_until<ProControllerContext>(
        stream, context,
        [button, timeout](ProControllerContext& context){
            pbf_mash_button(context, button, timeout);
        },
        {{overworld}}
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "mash_until_dialogue_ends: overworld not reached within timeout.",
            stream
        );
    }
}


void repeat_dpad(
    ProControllerContext& context,
    DpadState& state,
    DpadPosition dir,
    Milliseconds press,
    Milliseconds hold,
    size_t times
){
    if (state.last_dir != DPAD_NONE && state.last_dir != dir){
        times += 1;
    }
    state.last_dir = dir;
    for (size_t i = 0; i < times; i++){
        pbf_press_dpad(context, dir, press, hold);
    }
}


bool heal_pokemon(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& label
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 1800ms, 100ms); // 7+
    context.wait_for_all_requests();

    pbf_mash_button(context, BUTTON_A, 5000ms);
    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    wait_for_dialogue(stream, context, label + " - Pokemon healing");
    mash_until_dialogue_ends(stream, context, BUTTON_B);
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    // Exiting the Pokemon Center
    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 15000ms, 100ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("heal_pokemon(" + label + "): black screen not detected", COLOR_RED);
        return false;
    }

    return true;
}


}
}
}
