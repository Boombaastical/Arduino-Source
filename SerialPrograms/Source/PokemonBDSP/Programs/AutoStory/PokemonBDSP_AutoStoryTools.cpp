/*  BDSP AutoStory Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
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


void checkpoint_save(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    static constexpr size_t MAX_RETRIES = 10;

    bool success = false;

    for (size_t attempt = 0; attempt < MAX_RETRIES; attempt++){
        try{
            env.console.log("Checkpoint save attempt: " + std::to_string(attempt));

            // ---------- STEP 1: Ensure menu ----------
            MenuWatcher menu(COLOR_RED);
            if (!menu.detect(env.console.video().snapshot())){
                env.console.log("Opening menu...");
                overworld_to_menu(env.console, context);
                context.wait_for(std::chrono::milliseconds(300));
            }else{
                env.console.log("Already in menu.");
            }

            // ---------- STEP 2: Trigger save ----------
            pbf_press_button(context, BUTTON_R, 80ms, 2000ms);

            // ---------- STEP 3: Wait for save dialog ----------
            ShortDialogWatcher dialog(COLOR_RED);
            int dialog_ret = wait_until(
                env.console, context,
                std::chrono::seconds(5),
                {dialog}
            );

            if (dialog_ret < 0){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Save dialog not detected.",
                    env.console
                );
            }

            env.console.log("Save dialog detected.");

            // Confirm save
            pbf_press_button(context, BUTTON_ZL, 80ms, 2000ms);

            // ---------- STEP 4: Wait for save to finish ----------
            OverworldWatcher overworld;
            int overworld_ret = wait_until(
                env.console, context,
                std::chrono::seconds(10),
                {overworld}
            );

            if (overworld_ret < 0){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Did not return to overworld after save.",
                    env.console
                );
            }

            env.console.log("Returned to overworld after save.");

            success = true;
            break;

        }catch (OperationFailedException& e){
            env.console.log(
                "Save attempt failed: " + e.message(),
                COLOR_RED
            );

            context.wait_for(std::chrono::milliseconds(1000));
        }
    }

    // ---------- FALLBACK ----------
    if (!success){
        env.console.log(
            "Save failed after retries. Performing blind save fallback.",
            COLOR_ORANGE
        );

        pbf_press_button(context, BUTTON_X, 80ms, 1000ms);
        pbf_press_button(context, BUTTON_R, 80ms, 2000ms);
        pbf_press_button(context, BUTTON_ZL, 80ms, 5000ms);

        pbf_mash_button(context, BUTTON_B, 1000ms);
        context.wait_for_all_requests();
    }

    stats.m_checkpoint++;
    env.update_stats();

    send_program_status_notification(
        env, notif_status_update,
        success ? "Saved at checkpoint (verified)." : "Saved (blind fallback)."
    );
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

            env.console.log("Resetting game from HOME before retry...", COLOR_ORANGE);
            bool ok = reset_game_from_home(env, env.console, context, true, 1000ms);
            if (!ok){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Failed to reset game from HOME during checkpoint retry.",
                    env.console
                );
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


}
}
}
