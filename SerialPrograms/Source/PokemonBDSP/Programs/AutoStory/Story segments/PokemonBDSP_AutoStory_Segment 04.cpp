/*  BDSP AutoStory - Segment 04
 *  From after acquiring the third badge until after acquiring the fourth badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "../Utils/PokemonBDSP_AutoStory_Battle.h"
#include "PokemonBDSP_AutoStory_Segment 04.h"

using namespace std::chrono_literals;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_04::name()       const{ return "04: Third Badge to Fourth Badge"; }
std::string AutoStory_Segment_04::start_text() const{ return "Start: Third Gym Badge acquired (Hearthome City)."; }
std::string AutoStory_Segment_04::end_text()   const{ return "End: Fourth Gym Badge acquired (Pastoria City)."; }

void AutoStory_Segment_04::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_04().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_04::name()       const{ return "004 - " + AutoStory_Segment_04().name(); }
std::string AutoStory_Checkpoint_04::start_text() const{ return "Third Gym Badge acquired (Hearthome City)."; }
std::string AutoStory_Checkpoint_04::end_text()   const{ return "Fourth Gym Badge acquired (Pastoria City)."; }

void AutoStory_Checkpoint_04::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_04(env, context, options, stats);
}


static bool leave_gym(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("leave_gym: walking out of gym");
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); /* Replaces 7 Dpad down presses*/

    pbf_press_dpad(context, DPAD_RIGHT, 280ms, 200ms);
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); /* Replaces 2 Dpad down presses*/
    pbf_press_dpad(context, DPAD_LEFT, 280ms, 200ms);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); /* Replaces 4 Dpad down presses*/
    pbf_press_dpad(context, DPAD_RIGHT, 280ms, 200ms);
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); /* Replaces 2 Dpad down presses*/
    pbf_press_dpad(context, DPAD_LEFT, 280ms, 200ms);
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 10000ms, 0ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("leave_gym: black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("leave_gym: transition confirmed.");
    return true;
}

static bool go_to_galactic_warehouse(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("Walking towards the team Galactic warehouse");
    pbf_wait(context, 6000ms);
    pbf_mash_button(context, BUTTON_B, 4000ms);
    pbf_wait(context, 2000ms);
    pbf_move_left_joystick(context, {0, -1}, 3000ms, 100ms); /* Replaces 14 Dpad down presses*/
    pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); /* Replaces 14 Dpad right presses*/
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); /* Replaces 4 Dpad up presses*/
    pbf_move_left_joystick(context, {+1, 0}, 3200ms, 100ms); /* Replaces 25 Dpad right presses*/
    pbf_move_left_joystick(context, {0, +1}, 1600ms, 100ms); /* Replaces 11 Dpad up presses*/
    pbf_move_left_joystick(context, {-1, 0}, 2600ms, 100ms); /* Replaces 19 Dpad left presses*/
    pbf_move_left_joystick(context, {0, +1}, 1500ms, 100ms); /* Replaces 9 Dpad up presses*/
    pbf_move_left_joystick(context, {-1, 0}, 2400ms, 100ms); /* Replaces 11 Dpad left presses*/
    pbf_move_left_joystick(context, {0, +1}, 4000ms, 100ms); /* Replaces 13 Dpad up presses*/
    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); /* Replaces 3 Dpad right presses*/
    pbf_press_dpad(context, DPAD_DOWN, 280ms, 200ms);
    pbf_mash_button(context, BUTTON_A, 8000ms);
    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 8000ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("go_to_galactic_warehouse: black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("go_to_galactic_warehouse: transition confirmed.");
    return true;
}

void checkpoint_04(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!leave_gym(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "leave_gym: transition not detected.", env.console);
            }
            if (!go_to_galactic_warehouse(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_galactic_warehouse: transition not detected.", env.console);
            }

            if (options.help_mode == HelpMode::OUTSIDE_HELP){
                handle_battle_with_outside_help(
                    env.program_info(),
                    env.console,
                    context,
                    "Veilstone_GalacticGrunts_2v2",
                    options
                );
            }
        }
    );
}


}
}
}
