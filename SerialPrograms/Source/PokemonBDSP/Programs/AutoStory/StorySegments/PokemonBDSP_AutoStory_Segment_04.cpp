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
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "../Utils/PokemonBDSP_AutoStory_Battle.h"
#include "PokemonBDSP_AutoStory_Segment_04.h"

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

    AutoStory_Checkpoint_23().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_23::name()       const{ return "004 - " + AutoStory_Segment_04().name(); }
std::string AutoStory_Checkpoint_23::start_text() const{ return "Third Gym Badge acquired (Hearthome City)."; }
std::string AutoStory_Checkpoint_23::end_text()   const{ return "Fourth Gym Badge acquired (Pastoria City)."; }

void AutoStory_Checkpoint_23::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    Checkpoint_23(env, context, options, stats);
}

static bool handle_battle(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& trainerid
){
    context.wait_for_all_requests();
    pbf_wait(context, 15000ms);
    pbf_press_button(context, BUTTON_A, 80ms, 300ms);
    pbf_wait(context, 500ms);

    if (trainerid == "veilstone_galactic_grunts_1"){
        // Select Razor Leaf and mash that
        pbf_press_dpad(context, DPAD_UP, 280ms, 200ms);
        
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
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
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("handle_battle_psychic_abigail: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("handle_battle_psychic_abigail: transition confirmed.");
        return true;
    } else if (trainerid == "route_214_collector_brady"){
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("handle_battle_collector_brady: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("handle_battle_collector_brady: transition confirmed.");
        return true;
    }
    return false;
}

static bool activate_repel(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("Activating repel");
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_X, 80ms, 300ms);
    pbf_wait(context, 500ms);
    pbf_press_button(context, BUTTON_PLUS, 80ms, 300ms);
    pbf_wait(context, 500ms);
    pbf_press_button(context, BUTTON_B, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2);
    pbf_press_button(context, BUTTON_A, 80ms, 300ms);
    context.wait_for_all_requests();

    pbf_wait(context, 1000ms);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 4);
    pbf_mash_button(context, BUTTON_A, 400ms);
    pbf_mash_button(context, BUTTON_B, 2000ms);
    return true;
}

static bool leave_gym(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("leave_gym: walking out of gym");
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 7

    pbf_press_dpad(context, DPAD_RIGHT, 280ms, 200ms);
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 2
    pbf_press_dpad(context, DPAD_LEFT, 280ms, 200ms);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); // 4
    pbf_press_dpad(context, DPAD_RIGHT, 280ms, 200ms);
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 2
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

static bool go_to_veilstone_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    stream.log("Walking towards the pokemon center");
    pbf_wait(context, 4000ms);
    wait_for_dialogue(stream, context, "Dawn");
    mash_until_dialogue_ends(stream, context, BUTTON_B);
    context.wait_for_all_requests();
    pbf_wait(context, 4000ms);
    
    pbf_move_left_joystick(context, {0, -1}, 3000ms, 100ms); // 21
    pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); // 14
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 4
    pbf_move_left_joystick(context, {+1, 0}, 3400ms, 100ms); // 25
    pbf_move_left_joystick(context, {0, +1}, 1600ms, 100ms); // 11
    pbf_move_left_joystick(context, {-1, 0}, 1600ms, 100ms); // 12
    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms); // 5
    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 8+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2);
    context.wait_for_all_requests();

    /* Entering the Pokemon Center */
    BlackScreenOverWatcher black_screen1(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
        },
        {{black_screen1}}
    );
    if (ret < 0){
        stream.log("Didn't enter pokemon center!", COLOR_RED);
        return false;
    }

    stream.log("Entered pokemon center successfully!");

    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 1800ms, 100ms); // 7+
    context.wait_for_all_requests();

    pbf_mash_button(context, BUTTON_A, 5000ms);
    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    wait_for_dialogue(stream, context, "Pokemon healing");
    mash_until_dialogue_ends(stream, context, BUTTON_B);
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    /* Exiting the Pokemon Center */
    BlackScreenOverWatcher black_screen2(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 15000ms, 100ms);
        },
        {{black_screen2}}
    );
    if (ret < 0){
        stream.log("go_to_veilstone_pokemon_center: black screen not detected (1)", COLOR_RED);
        return false;
    }
    stream.log("go_to_veilstone_pokemon_center: transition confirmed.");
    return true;

}

static bool go_to_galactic_warehouse(
    VideoStream& stream,
    ProControllerContext& context
){
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); // 3+
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 6
    pbf_move_left_joystick(context, {0, -1}, 1400ms, 100ms); // 6+
    pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms); // 7
    pbf_move_left_joystick(context, {0, +1}, 2400ms, 100ms); // 9+
    pbf_move_left_joystick(context, {-1, 0}, 2800ms, 100ms); // 12+
    pbf_move_left_joystick(context, {0, +1}, 4000ms, 100ms); // 15+
    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 5+
    pbf_press_dpad(context, DPAD_DOWN, 400ms, 200ms);

    context.wait_for_all_requests();
    BlackScreenOverWatcher battle_black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 12000ms);
        },
        {{battle_black_screen}}
    );
    if (ret < 0){
        stream.log("go_to_galactic_warehouse: battle black screen not detected (2)", COLOR_RED);
        return false;
    }
    stream.log("go_to_galactic_warehouse: battle transition confirmed.");
    return true;
}

static bool enter_galactic_warehouse(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    pbf_wait(context, 3000ms);
    context.wait_for_all_requests();
    
    pbf_mash_button(context, BUTTON_B, 16000ms);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {+1, 0}, 1200ms, 100ms); // 6+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);

    context.wait_for_all_requests();
    
    BlackScreenOverWatcher black_screen_1(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
        },
        {{black_screen_1}}
    );
    if (ret < 0){
        stream.log("Entering Galactic warehouse black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("Entered warehouse");

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 7+
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 2+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 2+
    context.wait_for_all_requests();
    // Get fly
    pbf_mash_button(context, BUTTON_A, 10000ms);

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    pbf_move_left_joystick(context, {0, -1}, 1600ms, 100ms); // 5+
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 6);
    
    // Black screen detector here when exiting the warehouse
    BlackScreenOverWatcher black_screen_2(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
        },
        {{black_screen_2}}
    );
    if (ret < 0){
        stream.log("enter_galactic_warehouse: black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("enter_galactic_warehouse: transition confirmed.");
    return true;
}

static bool go_to_route_214(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    stream.log("Moving towards Route 214");
    pbf_wait(context, 3000ms);
    DpadState dpad;
    pbf_move_left_joystick(context, {0, -1}, 1600ms, 100ms); // 5+
    pbf_move_left_joystick(context, {-1, 0}, 1300ms, 100ms); // 8
    pbf_move_left_joystick(context, {0, -1}, 3000ms, 100ms); // 13+
    pbf_move_left_joystick(context, {+1, 0}, 3000ms, 100ms); // 15+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 4);

    pbf_move_left_joystick(context, {0, -1}, 5000ms, 100ms); // 23+
    pbf_move_left_joystick(context, {+1, 0}, 1000ms, 100ms); // 3+
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 3);

    pbf_move_left_joystick(context, {+1, 0}, 2200ms, 100ms); // 15
    context.wait_for_all_requests();

    BlackScreenOverWatcher black_screen_1(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
        },
        {{black_screen_1}}
    );
    if (ret < 0){
        stream.log("go_to_route_214_enter_house: black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("Entered house");
    pbf_wait(context, 2000ms);

    context.wait_for_all_requests();
    // Add a black screen detector here when entering the connecting house
    BlackScreenOverWatcher black_screen_2(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
        },
        {{black_screen_2}}
    );
    if (ret < 0){
        stream.log("go_to_route_214_exit_house: black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("Exited house");
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {0, -1}, 6000ms, 100ms);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 200000ms);
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

static bool walk_through_route_214_1(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    stream.log("Continuing on the first part of route 214");
    pbf_wait(context, 3000ms);
    DpadState dpad;

    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 4
    pbf_move_left_joystick(context, {+1, 0}, 3000ms, 100ms); // 13+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);

    activate_repel(stream, context);

    {
        BattleMenuWatcher battle_menu(BattleType::TRAINER);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                // TODO: fill in the actual movement sequence past Collector Brady
                pbf_move_left_joystick(context, {0, -1}, 2000ms, 100ms);
                pbf_move_left_joystick(context, {+1, 0}, 1500ms, 100ms);
            },
            {{battle_menu}}
        );
        if (ret == 0){
            handle_battle(stream, context, "route_214_collector_brady");
        }
    }

    return true;
}

void Checkpoint_23(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            /*if (!leave_gym(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "leave_gym: transition not detected.", env.console);
            }
            if (!go_to_veilstone_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_galactic_warehouse: transition not detected.", env.console);
            }
            if (!go_to_galactic_warehouse(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_galactic_warehouse: transition not detected.", env.console);
            }
            if (!handle_battle(env.console, context, "veilstone_galactic_grunts_1")){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_galactic_warehouse: transition not detected.", env.console);
            }
            if (!enter_galactic_warehouse(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_galactic_warehouse: transition not detected.", env.console);
            }*/
            if (!go_to_route_214(env.console, context)){
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
