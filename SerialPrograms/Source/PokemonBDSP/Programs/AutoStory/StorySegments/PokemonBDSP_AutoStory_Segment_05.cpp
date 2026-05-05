/*  BDSP AutoStory - Segment 05
 *  From after acquiring the fourth badge until after acquiring the fifth badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Globals.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MarkFinder.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "../Utils/PokemonBDSP_AutoStory_Battle.h"
#include "PokemonBDSP_AutoStory_Segment_05.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_05::name()       const{ return "05: Fourth Badge to Fifth Badge"; }
std::string AutoStory_Segment_05::start_text() const{ return "Start: Fourth Gym Badge acquired (Pastoria City)."; }
std::string AutoStory_Segment_05::end_text()   const{ return "End: Fifth Gym Badge acquired (Veilstone City)."; }

void AutoStory_Segment_05::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_05().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_05::name()       const{ return "005 - " + AutoStory_Segment_05().name(); }
std::string AutoStory_Checkpoint_05::start_text() const{ return "Fourth Gym Badge acquired (Pastoria City)."; }
std::string AutoStory_Checkpoint_05::end_text()   const{ return "Fifth Gym Badge acquired (Veilstone City)."; }

void AutoStory_Checkpoint_05::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_05(env, context, options, stats);
}

static void move_back_after_unexpected_battle(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& trainerid
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    if (trainerid == "route_213_tuber_chelsea") {
        DpadState dpad;

        pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 4+
        pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 4+
        dpad.last_dir = DPAD_UP;
        repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 6);
    } else if (trainerid == "route_213_tuber_jared") {
        // Just need to wait, nothing else
    } else if (trainerid == "route_213_beauty_cindy") {
        pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms); // 1+
    }
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

    if (trainerid == "pastoria_clint"){
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
            stream.log("handle_battle_pastoria_clint: black screen not detected!", COLOR_RED);
            return false;
        }
        context.wait_for_all_requests();
        pbf_mash_button(context, BUTTON_B, 2000ms);
        stream.log("handle_battle_pastoria_clint: transition confirmed.");
        return true;
    } else if (
        trainerid == "route_213_tuber_chelsea" or
        trainerid == "route_213_tuber_jared" or
        trainerid == "route_213_beauty_cindy"
    ) {
        // Select Razor leaf
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
            stream.log("route_213_tuber_chelsea / jared: black screen not detected!", COLOR_RED);
            return false;
        }
        context.wait_for_all_requests();
        pbf_mash_button(context, BUTTON_B, 2000ms);
        move_back_after_unexpected_battle(stream, context, trainerid);
        return true;
    } else if (trainerid == "route_214_galactic_grunt"){
        // Select Bulldoze and mash that
        pbf_press_dpad(context, DPAD_DOWN, 280ms, 200ms);
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("handle_battle_route_214_galactic_grunt: black screen not detected!", COLOR_RED);
            return false;
        }
        context.wait_for_all_requests();
        pbf_mash_button(context, BUTTON_B, 2000ms);
        stream.log("handle_battle_route_214_galactic_grunt: transition confirmed.");
        return true;
    }
    return false;
}

static bool leave_gym(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("leave_gym: walking out of gym");

    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 1+
    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); // 10+
    pbf_move_left_joystick(context, {0, -1}, 6000ms, 100ms); // 34+
    pbf_move_left_joystick(context, {-1, 0}, 1800ms, 100ms); // 9+
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 5+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3);
    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 100000ms, 100ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("leave_gym: Didn't enter building!", COLOR_RED);
        return false;
    }
    stream.log("leave_gym: transition confirmed.");
    return true;
}

static bool go_to_pastoria_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_pastoria_pokemon_center: going to pokemon center");

    pbf_move_left_joystick(context, {+1, 0}, 900ms, 100ms); // 6
    pbf_move_left_joystick(context, {0, +1}, 2400ms, 100ms); // 12+
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 3+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 7);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret1 = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 20000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret1 < 0){
            stream.log("go_to_pastoria_pokemon_centert: Didn't detect transition!", COLOR_RED);
            return false;
        }
        stream.log("go_to_pastoria_pokemon_center: transition confirmed.");
    return true;
}

static bool exit_pastoria_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    return heal_pokemon(stream, context, "Pastoria");
}

static bool pastoria_city_drama(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("pastoria_city_drama: starting the drama");

    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 10
    pbf_move_left_joystick(context, {0, +1}, 500ms, 100ms); // 3
    pbf_move_left_joystick(context, {+1, 0}, 1000ms, 100ms); // 7
    pbf_move_left_joystick(context, {0, +1}, 1400ms, 100ms); // 7+
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 2+

    context.wait_for_all_requests();

    ShortDialogWatcher talk_to_grunt_1(COLOR_BLUE);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 2000ms);
        },
        {{talk_to_grunt_1}}
    );
    if (ret < 0) {
        stream.log("pastoria_city_drama_talk_to_grunt_1: dialog box not detected!", COLOR_RED);
    }
    if (ret == 0){
        stream.log("pastoria_city_drama_talk_to_grunt_1: talking to grunt!");
        pbf_mash_button(context, BUTTON_B, 8000ms);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {+1, 0}, 4800ms, 100ms); // 24+
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);

    ShortDialogWatcher talk_to_grunt_2(COLOR_BLUE);
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 2000ms);
        },
        {{talk_to_grunt_1}}
    );
    if (ret < 0) {
        stream.log("pastoria_city_drama_talk_to_grunt_2: dialog box not detected!", COLOR_RED);
    }
    if (ret == 0){
        stream.log("pastoria_city_drama_talk_to_grunt_2: talking to grunt!");
        
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret1 = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 200000ms);
            },
            {{black_screen}}
        );
        if (ret1 < 0){
            stream.log("pastoria_city_drama_talk_to_clint: Didn't detect start of battle with Clint!", COLOR_RED);
            return false;
        }
        stream.log("pastoria_city_drama_talk_to_clint: battling Clint.");
        handle_battle(stream, context, "pastoria_clint");
        pbf_mash_button(context, BUTTON_B, 3000ms);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("pastoria_city_drama: transition not detected!", COLOR_RED);
        return false;
    }
    stream.log("pastoria_city_drama: transition detected.");
    return true;
}

static bool go_through_house_east_pastoria(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("Going through the building at the entrance of Pastoria city");
    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, 0}, 100000ms, 100ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("go_through_house_east_pastoria: Didn't exit building!", COLOR_RED);
        return false;
    }
    stream.log("go_through_house_east_pastoria: transition confirmed.");

    return true;
}

static bool go_through_route_213_chasing(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_route_213_chasing: continuing the drama");

    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 12
    activate_repel(stream, context);
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);
    pbf_move_left_joystick(context, {+1, 0}, 3600ms, 100ms); // 18+

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 5);

    MarkDetector beauty_cindy_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 5000ms, 100ms); // 15+
        },
        {{beauty_cindy_fight}}
    );
    if (ret == 0){
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_through_route_213_beauty_cindy: Didn't detect battle starting!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            context.wait_for_all_requests();
            stream.log("go_through_route_213_beauty_cindy: Starting battle.");
            handle_battle(stream, context, "route_213_beauty_cindy");
            context.wait_for_all_requests();
            pbf_wait(context, 3000ms);
        }
    }
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms); // 4
    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 9
    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 8+
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 2+
    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms); // 4
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 3+
    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms); // 3+

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2);

    context.wait_for_all_requests();

    ShortDialogWatcher talk_to_grunt(COLOR_BLUE);
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            ssf_press_left_joystick(context, {+1, 0}, 0ms, 5000ms);
            ssf_mash1_button(context, BUTTON_A, 5000ms);
        },
        {{talk_to_grunt}}
    );
    if (ret < 0) {
        stream.log("go_through_route_213_chasing_talk_to_grunt: dialog box not detected!", COLOR_RED);
    }
    if (ret == 0){
        stream.log("go_through_route_213_chasing_talk_to_grunt: talking to grunt!");
        pbf_mash_button(context, BUTTON_B, 6000ms);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 2+
    context.wait_for_all_requests();

    ShortDialogWatcher repel_dialog(COLOR_RED);
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms);
            pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms);
        },
        {{repel_dialog}}
    );
    if (ret == 0){
        pbf_mash_button(context, BUTTON_B, 3000ms);
    } else if (ret < 0){
            stream.log("go_through_route_213: Didn't detect repel dialog box!", COLOR_RED);
            return false;
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms);
    
    MarkDetector tuber_jared_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, 0}, 3200ms, 100ms); // 17
            pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 3
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);
        },
        {{tuber_jared_fight}}
    );
    if (ret == 0){
        context.wait_for_all_requests();
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_through_route_213_tuber_jared: Didn't detect battle starting!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("go_through_route_213_tuber_jared: Starting battle.");
            handle_battle(stream, context, "route_213_tuber_jared");
            context.wait_for_all_requests();
            pbf_wait(context, 3000ms);
        }
    }

    context.wait_for_all_requests();

    MarkDetector tuber_chelsea_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            DpadState dpad;
            pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 6+
            pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms); // 3+
            dpad.last_dir = DPAD_LEFT;
            repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 6);
        },
        {{tuber_chelsea_fight}}
    );
    if (ret == 0){
        context.wait_for_all_requests();
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_through_route_213_tuber_chelsea: Didn't detect battle starting!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("go_through_route_213_tuber_chelsea: Starting battle.");
            handle_battle(stream, context, "route_213_tuber_chelsea");
            context.wait_for_all_requests();
            pbf_wait(context, 3000ms);
        }
    }
    
    BlackScreenOverWatcher route_213_east_building(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 20000ms, 100ms);
        },
        {{route_213_east_building}}
    );
    if (ret < 0){
        stream.log("go_through_route_213_east_building: Didn't detect entering the building!", COLOR_RED);
        return false;
    };
    
    return true;
}

static bool go_through_building_route_213(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("Going through building between routes 214 and 213");
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 30000ms, 100ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("go_through_building_route_214: Didn't exit building!", COLOR_RED);
        return false;
    }
    stream.log("go_through_building_route_213: transition confirmed.");

    return true;
}

static bool go_through_route_214(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_route_214: continuing the drama");

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {0, +1}, 4000ms, 100ms); // 21+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1);

    context.wait_for_all_requests();

    ShortDialogWatcher talk_to_grunt(COLOR_BLUE);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            ssf_press_left_joystick(context, {+1, 0}, 0ms, 5000ms);
            ssf_mash1_button(context, BUTTON_A, 5000ms);
        },
        {{talk_to_grunt}}
    );
    if (ret < 0) {
        stream.log("go_through_route_214_talk_to_grunt: dialog box not detected!", COLOR_RED);
    }
    if (ret == 0){
        stream.log("go_through_route_214_talk_to_grunt: talking to grunt!");
        pbf_mash_button(context, BUTTON_B, 6000ms);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 2200ms, 100ms); // 11+
    pbf_move_left_joystick(context, {+1, 0}, 1200ms, 100ms); // 6+
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2);

    ShortDialogWatcher talk_to_grunt_2(COLOR_BLUE);
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
            ssf_mash1_button(context, BUTTON_A, 5000ms);
        },
        {{talk_to_grunt_2}}
    );
    if (ret < 0) {
        stream.log("go_through_route_214_talk_to_grunt_2: dialog box not detected!", COLOR_RED);
    }
    if (ret == 0){
        stream.log("go_through_route_214_talk_to_grunt_2: talking to grunt!");
        context.wait_for_all_requests();
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_through_route_214_galactic_grunt: Didn't detect battle starting!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("go_through_route_214_galactic_grunt: Starting battle.");
            handle_battle(stream, context, "route_214_galactic_grunt");
            context.wait_for_all_requests();
            pbf_wait(context, 5000ms);
        }
    };

    context.wait_for_all_requests();
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);

    ShortDialogWatcher talk_to_cynthia(COLOR_BLUE);
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            ssf_press_left_joystick(context, {+1, 0}, 0ms, 10000ms);
            ssf_mash1_button(context, BUTTON_A, 10000ms);
        },
        {{talk_to_cynthia}}
    );
    if (ret < 0) {
        stream.log("go_through_route_214_talk_to_cynthia: dialog box not detected!", COLOR_RED);
    }
    if (ret == 0){
        stream.log("go_through_route_214_talk_to_cynthia: talking to Cynthia!");
        pbf_mash_button(context, BUTTON_A, 10000ms);
    };

    context.wait_for_all_requests();

    return true;
}

void checkpoint_05(
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
            if (!go_to_pastoria_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_pastoria_pokemon_center: transition not detected.", env.console);
            }
            if (!exit_pastoria_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "exit_pastoria_pokemon_center: transition not detected.", env.console);
            }
            if (!pastoria_city_drama(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "pastoria_city_drama: transition not detected.", env.console);
            }
            if (!go_through_house_east_pastoria(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_house_east_pastoria: transition not detected.", env.console);
            }
            if (!go_through_route_213_chasing(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_route_213_chasing: transition not detected.", env.console);
            }
            if (!go_through_building_route_213(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_building_route_213: transition not detected.", env.console);
            }*/
            if (!go_through_route_214(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_route_214: transition not detected.", env.console);
            }
        }
    );
}


}
}
}
