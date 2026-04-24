/*  BDSP AutoStory - Segment 00
 *  From beginning cinematic until after choosing the starter Pokemon.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment_00.h"

using namespace std::chrono_literals;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

// ---------------------------------------------------------------------------
//  Tutorial Part 2 navigation (from Routes/TutorialPart2.cpp)
//  Navigates from player starting position toward the professor area.
// ---------------------------------------------------------------------------

static void tutorial_part_2_navigation(VideoStream& stream, ProControllerContext& context){
    pbf_press_dpad(context, DPAD_DOWN, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_DOWN, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_DOWN, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_DOWN, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_DOWN, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_DOWN, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_DOWN, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_DOWN, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);
    pbf_press_dpad(context, DPAD_RIGHT, 20ms, 105ms);

    wait_for_dialogue(stream, context, "Phase 1 (Professor Introduction)");

    pbf_mash_button(context, BUTTON_A, 200ms);
    pbf_wait(context, 1000ms);
}


// ---------------------------------------------------------------------------
//  Starter selection (from Routes/Starter_*.cpp)
// ---------------------------------------------------------------------------

static void select_turtwig(VideoStream& stream, ProControllerContext& context){
    stream.log("[DEBUG] Turtwig Selection: Starting starter selection");

    pbf_press_button(context, BUTTON_ZL, 160ms, 240ms);
    context.wait_for_all_requests();

    ShortDialogWatcher watcher(COLOR_CYAN);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& ctx){ ctx.wait_for(std::chrono::milliseconds(5000)); },
        { watcher }
    );
    if (ret != 0){
        ShortDialogWatcher watcher_red(COLOR_RED);
        ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& ctx){ ctx.wait_for(std::chrono::milliseconds(5000)); },
            { watcher_red }
        );
    }
    stream.log(ret == 0 ? "[DEBUG] Turtwig Selection: Confirmation dialogue detected"
                        : "[WARNING] Turtwig Selection: Dialogue detection failed, proceeding anyway");

    pbf_wait(context, 400ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 400ms);
    pbf_press_button(context, BUTTON_ZL, 80ms, 5000ms);
    context.wait_for_all_requests();
    stream.log("[DEBUG] Turtwig Selection: Selection complete");
}

static void select_chimchar(VideoStream& stream, ProControllerContext& context){
    stream.log("[DEBUG] Chimchar Selection: Starting starter selection");

    pbf_press_dpad(context, DPAD_RIGHT, 160ms, 840ms);
    pbf_press_button(context, BUTTON_ZL, 160ms, 240ms);
    context.wait_for_all_requests();

    ShortDialogWatcher watcher(COLOR_CYAN);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& ctx){ ctx.wait_for(std::chrono::milliseconds(5000)); },
        { watcher }
    );
    if (ret != 0){
        ShortDialogWatcher watcher_red(COLOR_RED);
        ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& ctx){ ctx.wait_for(std::chrono::milliseconds(5000)); },
            { watcher_red }
        );
    }
    stream.log(ret == 0 ? "[DEBUG] Chimchar Selection: Confirmation dialogue detected"
                        : "[WARNING] Chimchar Selection: Dialogue detection failed, proceeding anyway");

    pbf_wait(context, 400ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 400ms);
    pbf_press_button(context, BUTTON_ZL, 80ms, 5000ms);
    context.wait_for_all_requests();
    stream.log("[DEBUG] Chimchar Selection: Selection complete");
}

static void select_piplup(VideoStream& stream, ProControllerContext& context){
    stream.log("[DEBUG] Piplup Selection: Starting starter selection");

    pbf_press_dpad(context, DPAD_RIGHT, 160ms, 840ms);
    pbf_press_dpad(context, DPAD_RIGHT, 160ms, 840ms);
    pbf_press_button(context, BUTTON_ZL, 160ms, 240ms);
    context.wait_for_all_requests();

    ShortDialogWatcher watcher(COLOR_CYAN);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& ctx){ ctx.wait_for(std::chrono::milliseconds(5000)); },
        { watcher }
    );
    if (ret != 0){
        ShortDialogWatcher watcher_red(COLOR_RED);
        ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& ctx){ ctx.wait_for(std::chrono::milliseconds(5000)); },
            { watcher_red }
        );
    }
    stream.log(ret == 0 ? "[DEBUG] Piplup Selection: Confirmation dialogue detected"
                        : "[WARNING] Piplup Selection: Dialogue detection failed, proceeding anyway");

    pbf_wait(context, 400ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 400ms);
    pbf_press_button(context, BUTTON_ZL, 80ms, 5000ms);
    context.wait_for_all_requests();
    stream.log("[DEBUG] Piplup Selection: Selection complete");
}


// ---------------------------------------------------------------------------
//  Base route navigation (from Routes/BaseRoute.cpp)
//  Navigates from player's room to Route 201 through Mom/Barry dialogues.
// ---------------------------------------------------------------------------

static void intro_navigation(VideoStream& stream, ProControllerContext& context){
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);

    wait_for_dialogue(stream, context, "Phase 2 (Mom)");

    pbf_mash_button(context, BUTTON_A, 5000ms);
    pbf_wait(context, 5000ms);

    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);

    wait_for_dialogue(stream, context, "Phase 4 (Mom Warning)");

    pbf_mash_button(context, BUTTON_A, 2500ms);
    pbf_wait(context, 800ms);

    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);

    wait_for_dialogue(stream, context, "Phase 6 (Barry 2nd)");

    pbf_mash_button(context, BUTTON_A, 500ms);
    pbf_wait(context, 800ms);

    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);

    wait_for_dialogue(stream, context, "Phase 8 (Barry Final)");

    pbf_mash_button(context, BUTTON_A, 500ms);
    pbf_wait(context, 800ms);

    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);

    wait_for_dialogue(stream, context, "Phase 9 (Barry Route 201)");

    pbf_mash_button(context, BUTTON_A, 500ms);

    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
}


// ---------------------------------------------------------------------------
//  Segment / Checkpoint classes
// ---------------------------------------------------------------------------

std::string AutoStory_Segment_00::name()       const{ return "00: Intro Cinematic to Starter Choice"; }
std::string AutoStory_Segment_00::start_text() const{ return "Start: Game launched, beginning cinematic playing."; }
std::string AutoStory_Segment_00::end_text()   const{ return "End: Starter Pokemon chosen and received."; }

void AutoStory_Segment_00::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_00().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_00::name()       const{ return "000 - " + AutoStory_Segment_00().name(); }
std::string AutoStory_Checkpoint_00::start_text() const{ return "Game launched, beginning cinematic playing."; }
std::string AutoStory_Checkpoint_00::end_text()   const{ return "Starter Pokemon chosen and received."; }

void AutoStory_Checkpoint_00::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_00(env, context, options, stats);
}


void checkpoint_00(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            // 1. Intro starting from room
            intro_navigation(env.console, context);

            // 2. Starter selection
            switch (options.starter_choice){
                case StarterChoice::TURTWIG:  select_turtwig(env.console, context);  break;
                case StarterChoice::CHIMCHAR: select_chimchar(env.console, context); break;
                case StarterChoice::PIPLUP:   select_piplup(env.console, context);   break;
            }

            // 3. Going back to the professor
            tutorial_part_2_navigation(env.console, context);
        }
    );
}


}
}
}
