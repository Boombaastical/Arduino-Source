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
    
    /* Original code
    pbf_move_left_joystick(context, {0, -1}, 3000ms, 100ms); Replaces 14 Dpad down presses
    pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); Replaces 14 Dpad right presses
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); Replaces 4 Dpad up presses
    pbf_move_left_joystick(context, {+1, 0}, 3200ms, 100ms); Replaces 25 Dpad right presses
    pbf_move_left_joystick(context, {0, +1}, 1600ms, 100ms); Replaces 11 Dpad up presses
    pbf_move_left_joystick(context, {-1, 0}, 2600ms, 100ms); Replaces 19 Dpad left presses
    pbf_move_left_joystick(context, {0, +1}, 1500ms, 100ms); Replaces 9 Dpad up presses
    pbf_move_left_joystick(context, {-1, 0}, 2400ms, 100ms); Replaces 11 Dpad left presses
    pbf_move_left_joystick(context, {0, +1}, 4000ms, 100ms); Replaces 13 Dpad up presses
    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); Replaces 3 Dpad right presses
    */
    DpadState dpad;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 21);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 14);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 4);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 25);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 11);
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 12);

    /* Go to Pokemon center to heal after Gym */
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 5);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 6);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 2);
    /* Add a black screen detector here when entering the Pokemon Center */
    pbf_wait(context, 4000ms);
    // Character faces down after entering; 7 = 6 moves + 1 turn (direction not tracked across transition)
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 7);
    pbf_mash_button(context, BUTTON_A, 200ms);
    pbf_wait(context, 4000ms);
    pbf_mash_button(context, BUTTON_B, 2000ms);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 6);
    /* Add a black screen detector here when exiting the Pokemon Center */
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2);
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 6);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 4);
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 7);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 9);
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 12);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 15);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 3);
    
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

static bool handle_battle(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& trainerid
){
    pbf_wait(context, 10000ms);
    pbf_press_button(context, BUTTON_A, 80ms, 300ms);
    pbf_wait(context, 500ms);

    if (trainerid == "veilstone_galactic_grunts_1"){
        /* Select Razor Leaf and mash that */
        pbf_press_dpad(context, DPAD_UP, 280ms, 200ms);
        
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 30000ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("handle_battle_veilstone_galactic_grunts_2: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("handle_battle_veilstone_galactic_grunts_2: transition confirmed.");
        return true;
    } else if (trainerid == "route_214_psychic_abigail"){
        /* Directly selecting Crunch and mashing until end of battle*/
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 30000ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("handle_battle_psychic_abigail: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("handle_battle_psychic_abigail: transition confirmed.");
        return true;
    }
    return false;
}

static bool go_to_galactic_warehouse_2(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("Continue walking towards the team Galactic warehouse");
    pbf_wait(context, 4000ms);
    /* Finish the dialogue and go to pick up Fly in the warehouse*/
    pbf_mash_button(context, BUTTON_B, 10000ms);
    DpadState dpad;
    // Character direction unknown after battle transition; 6 = 5 moves + 1 turn
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 6);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 4);
    /* Add a black screen detector here when entering the warehouse */
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 5);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 4);
    pbf_mash_button(context, BUTTON_A, 2000ms);

    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 3);
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 5);
    
    /* Black screen detector here when exiting the warehouse */
    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_press_dpad(context, DPAD_DOWN, 2000ms, 300ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("handle_battle: black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("handle_battle: transition confirmed.");
    return true;
}

static bool go_to_route_214(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("Moving towards Route 214");
    DpadState dpad;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 4);
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 8);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 13);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 9);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 20);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 16);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 10);

    /* Add a black screen detector here when entering the connecting house */
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 10);

    /* Add a black screen detector here when exiting the connecting house */
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 14);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 8000ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_to_route_214: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_route_214: transition confirmed.");
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
            if (!handle_battle(env.console, context, "veilstone_galactic_grunts_1")){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_galactic_warehouse: transition not detected.", env.console);
            }
            if (!go_to_galactic_warehouse_2(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_galactic_warehouse: transition not detected.", env.console);
            }
            if (!handle_battle(env.console, context, "route_214_psychic_abigail")){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_galactic_warehouse: transition not detected.", env.console);
            }
            

        }
    );
}


}
}
}
