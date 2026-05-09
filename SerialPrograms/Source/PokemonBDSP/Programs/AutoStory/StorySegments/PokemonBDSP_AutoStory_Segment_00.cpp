/*  BDSP AutoStory - Segment 00
 *  From beginning cinematic until after choosing the starter Pokemon.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonBDSP/Resources/PokemonBDSP_NameDatabase.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP/Programs/PokemonBDSP_BasicCatcher.h"
#include "PokemonBDSP/Programs/PokemonBDSP_EncounterHandler.h"
#include "PokemonBDSP/Programs/PokemonBDSP_RunFromBattle.h"
#include "PokemonBDSP/Programs/PokemonBDSP_OverworldTrigger.h"
#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP/Programs/ShinyHunting/PokemonBDSP_StarterReset.h"
#include "PokemonBDSP_AutoStory_Segment_00.h"

using namespace std::chrono_literals;

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

//Helper functions for the entire segment. These are not meant to be used outside of 
//this segment and are not guaranteed to work properly if used elsewhere. They may 
// also be modified or removed without warning.

    //  Helper function to set time quickly by navigating through the system menu.
        static void set_time_fast(
            SingleSwitchProgramEnvironment& env,
            ProControllerContext& context,
            const DateTime& target
        ){
            env.log("[AutoStory] Fast date set starting...", COLOR_ORANGE);

            // 1) Home
            NintendoSwitch::go_home(env.console, context);

            // 2) Navigate to Date/Time settings quickly.
            NintendoSwitch::home_to_date_time(env.console, context, true);

            // 3) Enter date editor.
            pbf_press_button(context, BUTTON_A, 80ms, 240ms);

            // 4) Wait for date screen and set exact time.
            NintendoSwitch::DateChangeWatcher date_reader(env.console);
            int ret = wait_until(
                env.console, context, 10s,
                {date_reader}
            );
            if (ret < 0){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Failed to enter Date Change screen.",
                    env.console
                );
            }

            {
                VideoOverlaySet overlays(env.console.overlay());
                date_reader.make_overlays(overlays);
                date_reader.set_date(env.program_info(), env.console, context, target);
            }

            // 5) Commit date.
            pbf_press_button(context, BUTTON_A, 160ms, 240ms);

            // 6) Back to game.
            pbf_press_button(
                context, BUTTON_HOME, 160ms,
                ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0
            );
            NintendoSwitch::resume_game_from_home(env.console, context, false);

            context.wait_for_all_requests();
            context.wait_for(500ms);
        }
    // ---------------------------------------------------------------------------
    // Starter selection
    // ---------------------------------------------------------------------------

        static void select_turtwig(VideoStream& stream, ProControllerContext& context) {
            stream.log("[DEBUG] Turtwig Selection: Starting starter selection");

            pbf_press_button(context, BUTTON_ZL, 160ms, 240ms);
            context.wait_for_all_requests();

            ShortDialogWatcher watcher(COLOR_CYAN);
            int ret = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& ctx) { ctx.wait_for(std::chrono::milliseconds(5000)); },
                { watcher }
            );
            if (ret != 0) {
                ShortDialogWatcher watcher_red(COLOR_RED);
                ret = run_until<ProControllerContext>(
                    stream, context,
                    [](ProControllerContext& ctx) { ctx.wait_for(std::chrono::milliseconds(5000)); },
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

        static void select_chimchar(VideoStream& stream, ProControllerContext& context) {
            stream.log("[DEBUG] Chimchar Selection: Starting starter selection");

            pbf_press_dpad(context, DPAD_RIGHT, 160ms, 840ms);
            pbf_press_button(context, BUTTON_ZL, 160ms, 240ms);
            context.wait_for_all_requests();

            ShortDialogWatcher watcher(COLOR_CYAN);
            int ret = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& ctx) { ctx.wait_for(std::chrono::milliseconds(5000)); },
                { watcher }
            );
            if (ret != 0) {
                ShortDialogWatcher watcher_red(COLOR_RED);
                ret = run_until<ProControllerContext>(
                    stream, context,
                    [](ProControllerContext& ctx) { ctx.wait_for(std::chrono::milliseconds(5000)); },
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

        static void select_piplup(VideoStream& stream, ProControllerContext& context) {
            stream.log("[DEBUG] Piplup Selection: Starting starter selection");

            pbf_press_dpad(context, DPAD_RIGHT, 160ms, 840ms);
            pbf_press_dpad(context, DPAD_RIGHT, 160ms, 840ms);
            pbf_press_button(context, BUTTON_ZL, 160ms, 240ms);
            context.wait_for_all_requests();

            ShortDialogWatcher watcher(COLOR_CYAN);
            int ret = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& ctx) { ctx.wait_for(std::chrono::milliseconds(5000)); },
                { watcher }
            );
            if (ret != 0) {
                ShortDialogWatcher watcher_red(COLOR_RED);
                ret = run_until<ProControllerContext>(
                    stream, context,
                    [](ProControllerContext& ctx) { ctx.wait_for(std::chrono::milliseconds(5000)); },
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
    // Fight the starter battle. This is mostly mashing A and waiting, but we have 
    // to be careful to wait for the right amount of time at each stage to avoid desyncs.
    // ---------------------------------------------------------------------------
        static void fight_starly(VideoStream& stream, ProControllerContext& context) {
            stream.log("Starting Starly battle...");

            // Give time for battle transition
            pbf_wait(context, 5000ms);

            // Mash A to get through intro dialogue
            pbf_mash_button(context, BUTTON_A, 5000ms);

            // Use first move repeatedly (Tackle/Scratch/Pound)
            for (int i = 0; i < 6; i++) {
                pbf_press_button(context, BUTTON_A, 100ms, 2000ms); // Select move
                pbf_wait(context, 3000ms); // Wait for animation + damage
            }

            // Mash through faint + post-battle dialogue
            pbf_mash_button(context, BUTTON_A, 8000ms);

            stream.log("Starly battle complete.");

            wait_for_dialogue(stream, context, "Phase 9 (post battle text)");

            pbf_mash_button(context, BUTTON_B, 20000ms);

            wait_for_dialogue(stream, context, "Phase 9 (post battle text)");

            pbf_mash_button(context, BUTTON_B, 5000ms);

            context.wait_for_all_requests();


        }

    // ---------------------------------------------------------------------------
    // Helper function for 1st grass patch
    // ---------------------------------------------------------------------------
        class BattleDialogWatcher : public DetectorToFinder<BattleDialogDetector> {
        public:
            BattleDialogWatcher(Color color = COLOR_RED)
                : DetectorToFinder("BattleDialogWatcher", std::chrono::milliseconds(250), color)
            {
            }
        };
        static void walk_right_until_on_path(VideoStream& stream, ProControllerContext& context) {

            BlackScreenWatcher battle_start;
            BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
            BattleDialogWatcher battle_dialog(COLOR_YELLOW);

            for (size_t step = 0; step < 20; step++) {

                context.wait_for_all_requests();

                // Move right (fixed step)
                pbf_move_left_joystick(context, { +1, 0 }, 200ms, 0ms);

                context.wait_for_all_requests();

                // Check for battle after movement
                int battle_ret = wait_until(
                    stream, context,
                    2500ms,
                    {
                        {battle_start},  // 0
                        {battle_menu},   // 1
                        {battle_dialog}, // 2
                    }
                    );

                if (battle_ret >= 0) {
                    stream.log(
                        "[AutoStory] battle detected during movement. ret = " + std::to_string(battle_ret),
                        COLOR_ORANGE
                    );

                    bool escaped = false;
                    auto deadline = std::chrono::steady_clock::now() + 30s;

                    while (std::chrono::steady_clock::now() < deadline) {
                        // Spam escape inputs
                        pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                        context.wait_for_all_requests();

                        pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms); // down
                        pbf_move_left_joystick(context, { 1, 0 }, 80ms, 80ms);  // right
                        pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                        context.wait_for_all_requests();

                        EndBattleWatcher battle_end;
                        if (wait_until(stream, context, 1500ms, { {battle_end} }) == 0) {
                            escaped = true;
                            break;
                        }
                    }

                    // Light text advance
                    pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                    context.wait_for_all_requests();

                    // Reposition cursor on "Run"
                    pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms);
                    pbf_move_left_joystick(context, { 1, 0 }, 80ms, 80ms);
                    pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                    context.wait_for_all_requests();

                    if (!escaped) {
                        stream.log("[AutoStory] run_from_battle failed. Exiting to HOME.", COLOR_RED);

                        pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                        pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                        pbf_press_button(context, BUTTON_A, 80ms, 1200ms);

                        throw OperationFailedException(
                            ErrorReport::SEND_ERROR_REPORT,
                            "Failed to run from battle while moving right.",
                            stream
                        );
                    }

                    // Critical: let overworld stabilize before next step
                    context.wait_for_all_requests();
                    context.wait_for(1000ms);

                    continue;
                }
            }

            stream.log("[AutoStory] Completed fixed right movement (18 steps).", COLOR_GREEN);
        }

    // ---------------------------------------------------------------------------
    // Helper function to walk up through the second grass patch
    // ---------------------------------------------------------------------------
        static void walk_up_through_grass_2(VideoStream& stream, ProControllerContext& context) {

            BlackScreenWatcher battle_start;
            BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
            BattleDialogWatcher battle_dialog(COLOR_YELLOW);

            for (size_t step = 0; step < 18; step++) {

                context.wait_for_all_requests();

                // Move right (fixed step)
                pbf_move_left_joystick(context, { 0, 1 }, 200ms, 0ms);

                context.wait_for_all_requests();

                // Check for battle after movement
                int battle_ret = wait_until(
                    stream, context,
                    2500ms,
                    {
                        {battle_start},  // 0
                        {battle_menu},   // 1
                        {battle_dialog}, // 2
                    }
                    );

                if (battle_ret >= 0) {
                    stream.log(
                        "[AutoStory] battle detected during movement. ret = " + std::to_string(battle_ret),
                        COLOR_ORANGE
                    );

                    bool escaped = false;
                    auto deadline = std::chrono::steady_clock::now() + 30s;

                    while (std::chrono::steady_clock::now() < deadline) {
                        // Spam escape inputs
                        pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                        context.wait_for_all_requests();

                        pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms); // down
                        pbf_move_left_joystick(context, { 1, 0 }, 80ms, 80ms);  // right
                        pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                        context.wait_for_all_requests();

                        EndBattleWatcher battle_end;
                        if (wait_until(stream, context, 1500ms, { {battle_end} }) == 0) {
                            escaped = true;
                            break;
                        }
                    }

                    // Light text advance
                    pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                    context.wait_for_all_requests();

                    // Reposition cursor on "Run"
                    pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms);
                    pbf_move_left_joystick(context, { 1, 0 }, 80ms, 80ms);
                    pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                    context.wait_for_all_requests();

                    if (!escaped) {
                        stream.log("[AutoStory] run_from_battle failed. Exiting to HOME.", COLOR_RED);

                        pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                        pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                        pbf_press_button(context, BUTTON_A, 80ms, 1200ms);

                        throw OperationFailedException(
                            ErrorReport::SEND_ERROR_REPORT,
                            "Failed to run from battle while moving right.",
                            stream
                        );
                    }

                    // Critical: let overworld stabilize before next step
                    context.wait_for_all_requests();
                    context.wait_for(1000ms);

                    continue;
                }
            }

            stream.log("[AutoStory] Completed fixed up movement (18 steps).", COLOR_GREEN);
        }
    // ---------------------------------------------------------------------------
    //  Helper function to walk right through the second grass patch
    // ---------------------------------------------------------------------------
        static void walk_right_through_grass_2(VideoStream& stream, ProControllerContext& context) {

            BlackScreenWatcher battle_start;
            BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
            BattleDialogWatcher battle_dialog(COLOR_YELLOW);

            for (size_t step = 0; step < 10; step++) {

                context.wait_for_all_requests();

                // Move right (fixed step)
                pbf_move_left_joystick(context, { +1, 0 }, 200ms, 0ms);

                context.wait_for_all_requests();

                // Check for battle after movement
                int battle_ret = wait_until(
                    stream, context,
                    2500ms,
                    {
                        {battle_start},  // 0
                        {battle_menu},   // 1
                        {battle_dialog}, // 2
                    }
                    );

                if (battle_ret >= 0) {
                    stream.log(
                        "[AutoStory] battle detected during movement. ret = " + std::to_string(battle_ret),
                        COLOR_ORANGE
                    );

                    bool escaped = false;
                    auto deadline = std::chrono::steady_clock::now() + 30s;

                    while (std::chrono::steady_clock::now() < deadline) {
                        // Spam escape inputs
                        pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                        context.wait_for_all_requests();

                        pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms); // down
                        pbf_move_left_joystick(context, { 1, 0 }, 80ms, 80ms);  // right
                        pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                        context.wait_for_all_requests();

                        EndBattleWatcher battle_end;
                        if (wait_until(stream, context, 1500ms, { {battle_end} }) == 0) {
                            escaped = true;
                            break;
                        }
                    }

                    // Light text advance
                    pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                    context.wait_for_all_requests();

                    // Reposition cursor on "Run"
                    pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms);
                    pbf_move_left_joystick(context, { 1, 0 }, 80ms, 80ms);
                    pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                    context.wait_for_all_requests();

                    if (!escaped) {
                        stream.log("[AutoStory] run_from_battle failed. Exiting to HOME.", COLOR_RED);

                        pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                        pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                        pbf_press_button(context, BUTTON_A, 80ms, 1200ms);

                        throw OperationFailedException(
                            ErrorReport::SEND_ERROR_REPORT,
                            "Failed to run from battle while moving right.",
                            stream
                        );
                    }

                    // Critical: let overworld stabilize before next step
                    context.wait_for_all_requests();
                    context.wait_for(1000ms);

                    continue;
                }
            }

            stream.log("[AutoStory] Completed fixed right movement (10 steps).", COLOR_GREEN);
        }
    
    //helper function for post catch tutorial to catch one pokemon
        static void catch_1_pokemon(
            VideoStream& stream,
            ProControllerContext& context
        ){
            stream.log("[AutoStory] Starting catch_1_pokemon helper...", COLOR_BLUE);

            context.wait_for_all_requests();
            pbf_wait(context, 1000ms);

            BlackScreenWatcher battle_start;
            BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
            BattleDialogWatcher battle_dialog(COLOR_YELLOW);

            bool encounter_found = false;

            // Wiggle left/right until battle starts.
            while (!encounter_found){

                // Turn left.
                pbf_move_left_joystick(context, {-1, 0}, 120ms, 200ms);
                context.wait_for_all_requests();

                int ret = wait_until(
                    stream,
                    context,
                    1500ms,
                    {
                        {battle_start},
                        {battle_menu},
                        {battle_dialog},
                    }
                );

                if (ret >= 0){
                    encounter_found = true;
                    break;
                }

                // Turn right.
                pbf_move_left_joystick(context, {1, 0}, 120ms, 200ms);
                context.wait_for_all_requests();

                ret = wait_until(
                    stream,
                    context,
                    1500ms,
                    {
                        {battle_start},
                        {battle_menu},
                        {battle_dialog},
                    }
                );

                if (ret >= 0){
                    encounter_found = true;
                    break;
                }
            }

            stream.log("[AutoStory] Wild battle detected!", COLOR_GREEN);

            // Wait until the actual battle menu appears.
            BattleMenuWatcher confirm_menu(BattleType::STANDARD);

            int menu_ret = wait_until(
                stream,
                context,
                15s,
                {
                    {confirm_menu},
                }
                );

            if (menu_ret < 0) {
                stream.log(
                    "[AutoStory] Failed to detect battle menu.",
                    COLOR_RED
                );
                return;
            }

            stream.log("[AutoStory] Battle menu detected.", COLOR_GREEN);

            context.wait_for_all_requests();
            pbf_wait(context, 500ms);

            // Catch the pokemon.
            CatchResults result = basic_catcher(
                stream,
                context,
                Language::English,
                "poke-ball",
                10
            );

            switch (result.result){
            case CatchResult::POKEMON_CAUGHT:
                stream.log(
                    "[AutoStory] Pokemon successfully caught.",
                    COLOR_GREEN
                );
                break;

            default:
                stream.log(
                    "[AutoStory] Catch attempt failed.",
                    COLOR_ORANGE
                );
                break;
            }
            context.wait_for_all_requests();

            // Force clear any remaining text boxes / summaries.
            pbf_mash_button(context, BUTTON_B, 15000ms);

            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);

        }

    // ---------------------------------------------------------------------------
    // Helper function to walk left through the third grass patch
    // ---------------------------------------------------------------------------
        static void walk_left_through_grass_3(VideoStream& stream, ProControllerContext& context) {

            BlackScreenWatcher battle_start;
            BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
            BattleDialogWatcher battle_dialog(COLOR_YELLOW);

            for (size_t step = 0; step < 18; step++) {

                context.wait_for_all_requests();

                // Move left (fixed step)
                pbf_move_left_joystick(context, { -1, 0 }, 200ms, 0ms);

                context.wait_for_all_requests();

                // Check for battle after movement
                int battle_ret = wait_until(
                    stream, context,
                    2500ms,
                    {
                        {battle_start},  // 0
                        {battle_menu},   // 1
                        {battle_dialog}, // 2
                    }
                    );

                if (battle_ret >= 0) {
                    stream.log(
                        "[AutoStory] battle detected during movement. ret = " + std::to_string(battle_ret),
                        COLOR_ORANGE
                    );

                    bool escaped = false;
                    auto deadline = std::chrono::steady_clock::now() + 30s;

                    while (std::chrono::steady_clock::now() < deadline) {
                        // Spam escape inputs
                        pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                        context.wait_for_all_requests();

                        pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms); // down
                        pbf_move_left_joystick(context, { 1, 0 }, 80ms, 80ms);  // right
                        pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                        context.wait_for_all_requests();

                        EndBattleWatcher battle_end;
                        if (wait_until(stream, context, 1500ms, { {battle_end} }) == 0) {
                            escaped = true;
                            break;
                        }
                    }

                    // Light text advance
                    pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                    context.wait_for_all_requests();

                    // Reposition cursor on "Run"
                    pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms);
                    pbf_move_left_joystick(context, { 1, 0 }, 80ms, 80ms);
                    pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                    context.wait_for_all_requests();

                    if (!escaped) {
                        stream.log("[AutoStory] run_from_battle failed. Exiting to HOME.", COLOR_RED);

                        pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                        pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                        pbf_press_button(context, BUTTON_A, 80ms, 1200ms);

                        throw OperationFailedException(
                            ErrorReport::SEND_ERROR_REPORT,
                            "Failed to run from battle while moving right.",
                            stream
                        );
                    }

                    // Critical: let overworld stabilize before next step
                    context.wait_for_all_requests();
                    context.wait_for(1000ms);

                    continue;
                }
            }

            stream.log("[AutoStory] Completed fixed up movement (18 steps).", COLOR_GREEN);
        }
    // ---------------------------------------------------------------------------
    // Helper function to walk up through the third grass patch
    // ---------------------------------------------------------------------------
        static void walk_up_through_grass_3(VideoStream& stream, ProControllerContext& context) {

            BlackScreenWatcher battle_start;
            BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
            BattleDialogWatcher battle_dialog(COLOR_YELLOW);

            for (size_t step = 0; step < 18; step++) {

                context.wait_for_all_requests();

                // Move up (fixed step)
                pbf_move_left_joystick(context, { 0, 1 }, 200ms, 0ms);

                context.wait_for_all_requests();

                // Check for battle after movement
                int battle_ret = wait_until(
                    stream, context,
                    2500ms,
                    {
                        {battle_start},  // 0
                        {battle_menu},   // 1
                        {battle_dialog}, // 2
                    }
                    );

                if (battle_ret >= 0) {
                    stream.log(
                        "[AutoStory] battle detected during movement. ret = " + std::to_string(battle_ret),
                        COLOR_ORANGE
                    );

                    bool escaped = false;
                    auto deadline = std::chrono::steady_clock::now() + 30s;

                    while (std::chrono::steady_clock::now() < deadline) {
                        // Spam escape inputs
                        pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                        context.wait_for_all_requests();

                        pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms); // down
                        pbf_move_left_joystick(context, { 1, 0 }, 80ms, 80ms);  // right
                        pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                        context.wait_for_all_requests();

                        EndBattleWatcher battle_end;
                        if (wait_until(stream, context, 1500ms, { {battle_end} }) == 0) {
                            escaped = true;
                            break;
                        }
                    }

                    // Light text advance
                    pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                    context.wait_for_all_requests();

                    // Reposition cursor on "Run"
                    pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms);
                    pbf_move_left_joystick(context, { 1, 0 }, 80ms, 80ms);
                    pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                    context.wait_for_all_requests();

                    if (!escaped) {
                        stream.log("[AutoStory] run_from_battle failed. Exiting to HOME.", COLOR_RED);

                        pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                        pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                        pbf_press_button(context, BUTTON_A, 80ms, 1200ms);

                        throw OperationFailedException(
                            ErrorReport::SEND_ERROR_REPORT,
                            "Failed to run from battle while moving right.",
                            stream
                        );
                    }

                    // Critical: let overworld stabilize before next step
                    context.wait_for_all_requests();
                    context.wait_for(1000ms);

                    continue;
                }
            }

            stream.log("[AutoStory] Completed fixed up movement (18 steps).", COLOR_GREEN);
        }
    // ---------------------------------------------------------------------------
	// Helper function to walk right to fight the first trainer
    // ---------------------------------------------------------------------------
        static void walk_right_to_first_trainer(VideoStream& stream, ProControllerContext& context) {

            BlackScreenWatcher battle_start;
            BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
            BattleDialogWatcher battle_dialog(COLOR_YELLOW);

            for (size_t step = 0; step <5; step++) {

                context.wait_for_all_requests();

                // Move right (fixed step)
                pbf_move_left_joystick(context, { 1, 0 }, 200ms, 0ms);

                context.wait_for_all_requests();

                // Check for battle after movement
                int battle_ret = wait_until(
                    stream, context,
                    2500ms,
                    {
                        {battle_start},  // 0
                        {battle_menu},   // 1
                        {battle_dialog}, // 2
                    }
                    );

                if (battle_ret >= 0) {
                    stream.log(
                        "[AutoStory] battle detected during movement. ret = " + std::to_string(battle_ret),
                        COLOR_ORANGE
                    );

                    bool escaped = false;
                    auto deadline = std::chrono::steady_clock::now() + 30s;

                    while (std::chrono::steady_clock::now() < deadline) {
                        // Spam escape inputs
                        pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                        context.wait_for_all_requests();

                        pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms); // down
                        pbf_move_left_joystick(context, { 1, 0 }, 80ms, 80ms);  // right
                        pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                        context.wait_for_all_requests();

                        EndBattleWatcher battle_end;
                        if (wait_until(stream, context, 1500ms, { {battle_end} }) == 0) {
                            escaped = true;
                            break;
                        }
                    }

                    // Light text advance
                    pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                    context.wait_for_all_requests();

                    // Reposition cursor on "Run"
                    pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms);
                    pbf_move_left_joystick(context, { 1, 0 }, 80ms, 80ms);
                    pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                    context.wait_for_all_requests();

                    if (!escaped) {
                        stream.log("[AutoStory] run_from_battle failed. Exiting to HOME.", COLOR_RED);

                        pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                        pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                        pbf_press_button(context, BUTTON_A, 80ms, 1200ms);

                        throw OperationFailedException(
                            ErrorReport::SEND_ERROR_REPORT,
                            "Failed to run from battle while moving right.",
                            stream
                        );
                    }

                    // Critical: let overworld stabilize before next step
                    context.wait_for_all_requests();
                    context.wait_for(1000ms);

                    continue;
                }
            }

            stream.log("[AutoStory] Completed fixed up movement (18 steps).", COLOR_GREEN);
        }
    //Navigation functions for each part of the tutorial. These functions are meant to be used 
    //in order and rely on the state of the game being what it is at the end of the previous function. 
    //They may also be modified or removed without warning.
        
            // ---------------------------------------------------------------------------
            // Segment 00-000 begins
            // Navigates from player's room to Route 201 through Mom/Barry dialogues.
            // ---------------------------------------------------------------------------

        static void tutorial_part_1_navigation(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context) {
            DateTime t{ 2026, 1, 1, 0, 0, 0 };
            set_time_fast(env, context, t);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 527ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, 1.000000 }, 456ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 915ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, 1.000000 }, 642ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 557ms, 0ms);
            pbf_wait(context, 978ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1112ms, 0ms);
            pbf_wait(context, 4967ms);

            wait_for_dialogue(stream, context, "Phase 1 (Mom)");

            pbf_mash_button(context, BUTTON_A, 5000ms);
            pbf_wait(context, 300ms);

            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1662ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, -1.000000 }, 49ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1215ms, 0ms);
            pbf_wait(context, 1419ms);

            wait_for_dialogue(stream, context, "Phase 2 (Mom Warning)");

            pbf_mash_button(context, BUTTON_A, 2500ms);
            pbf_wait(context, 300ms);

            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 680ms, 0ms);

            pbf_wait(context, 4000ms);

            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1748ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 18ms, 0ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 2763ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 58ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1364ms, 0ms);

            wait_for_dialogue(stream, context, "Phase 3 (Barry 1st)");
            pbf_mash_button(context, BUTTON_A, 6000ms);
            pbf_wait(context, 800ms);
            wait_for_dialogue(stream, context, "Phase 3 (Barry 1st)");
            pbf_mash_button(context, BUTTON_A, 6000ms);
            pbf_wait(context, 800ms);

            pbf_wait(context, 1306ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 2722ms, 0ms);
            pbf_wait(context, 1445ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 161ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 370ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 667ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 130ms, 0ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 705ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 196ms, 0ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 684ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, 1.000000 }, 205ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 590ms, 0ms);
            pbf_wait(context, 1422ms);

            wait_for_dialogue(stream, context, "Phase 8 (Barry 2nd dialog)");
            pbf_mash_button(context, BUTTON_A, 10000ms);

            pbf_wait(context, 856ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 920ms, 0ms);
            pbf_wait(context, 631ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 972ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, -1.000000 }, 72ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 1127ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, -1.000000 }, 82ms, 0ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1335ms, 0ms);
            pbf_wait(context, 1124ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 2513ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, 1.000000 }, 51ms, 0ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 5861ms, 0ms);
            pbf_wait(context, 134ms);


            wait_for_dialogue(stream, context, "Phase 9 (Barry Route 201)");

            pbf_mash_button(context, BUTTON_B, 6000ms);
            pbf_wait(context, 800ms);

            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 700ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 279ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 0.000000 }, 10ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 145ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 0.000000 }, 7703ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 140ms, 0ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 1700ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 39ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 0.000000 }, 544ms, 1000ms);

            pbf_wait(context, 1000ms);

            save_game(stream, context);
            context.wait_for_all_requests();
        }
            // ---------------------------------------------------------------------------
            //  Segment 00-001 begins
            //  Navigates into lake Verity to recieve starter
            // ---------------------------------------------------------------------------
            static void starter_nav(VideoStream& stream, ProControllerContext& context) {
                // re-sync control
                pbf_press_button(context, BUTTON_B, 20ms, 200ms);
                pbf_wait(context, 500ms);

                // continue movement
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 300ms, 0ms);
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 300ms, 0ms);

                wait_for_dialogue(stream, context, "Phase 10 (Barry before the starter selection)");

                pbf_mash_button(context, BUTTON_A, 1000ms);

                pbf_wait(context, 500ms);
                std::shared_ptr<const ImageRGB32> briefcase =
                    std::make_shared<const ImageRGB32>(
                        RESOURCE_PATH() + "PokemonBDSP/StarterBriefcase.png"
                    );

                //  Mash B until we see the briefcase.
                ImageMatchWatcher detector(briefcase, {0.5, 0.1, 0.5, 0.7}, 100, true);

                int ret = run_until<ProControllerContext>(
                    stream, context,
                    [](ProControllerContext& context){
                        pbf_mash_button(context, BUTTON_B, 120000ms);
                    },
                    {{detector}}
                );

                if (ret == 0){
                    stream.log("Detected briefcase!");
                }else{
                    stream.log("Timed out waiting for briefcase.");
                    throw OperationFailedException(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Failed to detect briefcase in Segment 00.",
                        stream
                    );
                }

                //  Wait for briefcase to fully open.
                stream.log("Mashing B for briefcase to fully open.");
                pbf_mash_button(context, BUTTON_B, 2000ms);
            }

           // ---------------------------------------------------------------------------
           //  Segment 00-002 begins
           //  Navigates from Exit of lake Verity to receipt of the Pokedex
           // ---------------------------------------------------------------------------

            static void tutorial_part_2_navigation(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context) {
                context.wait_for_all_requests();

                context.wait_for_all_requests();
                pbf_wait(context, 563ms);
                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1700ms, 0ms); 
                pbf_move_left_joystick(context, { 1.000000, -1.000000 }, 510ms, 0ms);   
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 50ms, 0ms);   
                pbf_move_left_joystick(context, { 1.000000, -1.000000 }, 310ms, 0ms);   
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 5500ms, 0ms);  
                pbf_wait(context, 3963ms);  
                wait_for_dialogue(stream, context, "Phase 10 (Professor dialog to mom)"); 
                pbf_mash_button(context, BUTTON_B, 40000ms); 
                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 820ms, 0ms); 
                pbf_wait(context, 1483ms);   
                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 900ms, 0ms);   
                pbf_wait(context, 555ms);    
                pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 50ms, 0ms);  
                pbf_wait(context, 23ms);   
                pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 5020ms, 0ms); 
                pbf_wait(context, 169ms);      
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 745ms, 0ms);   
                DateTime t{ 2026, 1, 1, 0, 0, 50 };       
                set_time_fast(env, context, t);    
                walk_right_until_on_path(stream, context);  
                walk_up_through_grass_2(stream, context); 
                walk_right_through_grass_2(stream, context); 
                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 730ms, 0ms);  
                pbf_wait(context, 941ms);         
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 1950ms, 0ms);   
                wait_for_dialogue(stream, context, "Phase 11 (dawn conversation)");  
                pbf_mash_button(context, BUTTON_B, 50000ms);         
                wait_for_dialogue(stream, context, "Phase 12 (Nickname reject)");  
                pbf_mash_button(context, BUTTON_B, 40000ms);        
                wait_for_dialogue(stream, context, "Phase 13 (Pokedex accept)");    
                pbf_mash_button(context, BUTTON_A, 40000ms);


                // pokedex received
            }


            // ---------------------------------------------------------------------------
            // End of Segment 00 - 002
            // Standing in front of the professor after receiving the pokedex.
            // ---------------------------------------------------------------------------

            // ---------------------------------------------------------------------------
            // Begin Segment 00 - 003
            // Will heal at Sandgem pokeCenter to reliably catch a second pokemon for Home
            // transfer.
            // Collects poketech and stands in front of the PC to end the tutorial section
            // and send the starter to pokemon HOME.
            // ---------------------------------------------------------------------------

            static void tutorial_part_3_navigation(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context) {
                /*pbf_move_left_joystick(context, {0.000000, -1.000000}, 2010ms, 0ms);

                wait_for_dialogue(stream, context, "Phase 14 (dawn tour)");

                pbf_mash_button(context, BUTTON_B, 25000ms);

                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 5125ms, 0ms);
                pbf_wait(context, 336ms);
                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1700ms, 0ms);
                pbf_wait(context, 213ms);
                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 2325ms, 0ms);
                pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 950ms, 0ms);
                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 2360ms, 0ms);
                pbf_wait(context, 253ms);
                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 4610ms, 0ms);
                pbf_wait(context, 954ms);
                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 220ms, 0ms);
                pbf_wait(context, 587ms);
                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1375ms, 0ms);
                pbf_wait(context, 464ms);
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 775ms, 0ms);
                pbf_wait(context, 1655ms);
                pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 640ms, 0ms);
                pbf_wait(context, 3128ms);
                pbf_press_dpad(context, DPAD_UP, 180ms, 0ms);
                pbf_wait(context, 472ms);
                pbf_press_dpad(context, DPAD_UP, 130ms, 0ms);
                pbf_wait(context, 1621ms);
                pbf_press_dpad(context, DPAD_RIGHT, 130ms, 0ms);
                pbf_press_button(context, BUTTON_A, 180ms, 0ms);

                wait_for_dialogue(stream, context, "Phase 15 (Mom conversation to accept parcel)");

                pbf_mash_button(context, BUTTON_B, 40000ms);

                context.wait_for_all_requests();
                pbf_wait(context, 1500ms);
                pbf_press_dpad(context, DPAD_LEFT, 150ms, 0ms);
                pbf_wait(context, 1143ms);
                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 875ms, 0ms);
                pbf_wait(context, 1169ms);
                pbf_press_button(context, BUTTON_A, 180ms, 0ms);
                pbf_wait(context, 946ms);
                pbf_press_button(context, BUTTON_A, 180ms, 0ms);
                pbf_wait(context, 9092ms);
                pbf_press_button(context, BUTTON_A, 180ms, 0ms);
                pbf_wait(context, 740ms);
                pbf_press_button(context, BUTTON_A, 180ms, 0ms);
                pbf_wait(context, 2998ms);
                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 240ms, 0ms);
                pbf_wait(context, 893ms);
                pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 5300ms, 0ms);
                pbf_wait(context, 282ms);
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 260ms, 0ms);
                context.wait_for_all_requests();
                pbf_wait(context, 1500ms);

                DateTime t{ 2026, 1, 1, 0, 0, 50 };
                set_time_fast(env, context, t);
                walk_right_until_on_path(stream, context);
                walk_up_through_grass_2(stream, context);
                walk_right_through_grass_2(stream, context);
                context.wait_for_all_requests();
                pbf_wait(context, 1500ms);

                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 560ms, 0ms);
                pbf_wait(context, 1500ms);
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 5000ms, 0ms);
                pbf_wait(context, 1500ms);
                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1000ms, 0ms);
                pbf_wait(context, 1500ms);
                pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 1855ms, 0ms);
                pbf_wait(context, 1500ms);
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 430ms, 0ms);
                pbf_wait(context, 1500ms);
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 850ms, 0ms);
                pbf_wait(context, 1500ms);
                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1120ms, 0ms);

                wait_for_dialogue(stream, context, "Phase 16 (catch tutorial)");

                pbf_mash_button(context, BUTTON_B, 70000ms);
                context.wait_for_all_requests();
                pbf_wait(context, 2000ms);*/
                /*catch_1_pokemon(stream, context);
                context.wait_for_all_requests();
                pbf_wait(context, 1500ms);*/
                walk_left_through_grass_3(stream, context);
                walk_up_through_grass_3(stream, context);
                walk_right_to_first_trainer(stream, context);


                /*pbf_press_button(context, BUTTON_B, 100ms, 100ms);
                pbf_mash_button(context, BUTTON_A, 200ms);
                pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_mash_button(context, BUTTON_A, 200ms);
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
                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
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
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
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
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
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
                pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
                pbf_mash_button(context, BUTTON_A, 200ms);
                // catch a pokemon
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
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                // youngster Tristan
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
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
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                // lass Natalie
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
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
                pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
                // youngster logan
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
                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                pbf_mash_button(context, BUTTON_A, 200ms);
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
                // heal pokecenter
                pbf_press_button(context, BUTTON_A, 100ms, 100ms);
                pbf_mash_button(context, BUTTON_B, 200ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                // deposit starter
                // home transfer
                // withdraw starter
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
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
                pbf_press_dpad(context, DPAD_LEFT, 80ms, 300ms);
                pbf_press_button(context, BUTTON_A, 100ms, 100ms);
                pbf_mash_button(context, BUTTON_B, 200ms);
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
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                pbf_mash_button(context, BUTTON_A, 200ms);
                // poketech challenge begins
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
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_mash_button(context, BUTTON_A, 200ms);
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
                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
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
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_mash_button(context, BUTTON_A, 200ms);
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
                pbf_mash_button(context, BUTTON_A, 200ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
                pbf_mash_button(context, BUTTON_A, 200ms);*/
                // poketech acquired
            }
            // ---------------------------------------------------------------------------
            // Segment 00-02 ends
            // Standing in front of the Jubilife City Pokemon Center PC.
            // Player will need to deposit pokemon in the pc and transfer to Pokemon Home
            // and level to 100 before starting the next segment.
            // ---------------------------------------------------------------------------




            // ---------------------------------------------------------------------------
            //  Segment / Checkpoint classes
            // ---------------------------------------------------------------------------

            std::string AutoStory_Segment_00::name()       const { return "00: Intro to Poketech"; }
            std::string AutoStory_Segment_00::start_text() const { return "Start: Game launched, Standing in the player's room after tv stops playing"; }
            std::string AutoStory_Segment_00::end_text()   const { return "End: Starter Pokemon chosen, received, and Pokedex acquired, standing in front of the PC."; }

            void AutoStory_Segment_00::run_segment(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) const {
                stats.m_segment++;
                env.update_stats();
                context.wait_for_all_requests();
                env.console.log("Start Segment " + name(), COLOR_ORANGE);

                AutoStory_Checkpoint_000().run_checkpoint(env, context, options, stats);
                AutoStory_Checkpoint_001().run_checkpoint(env, context, options, stats);
                AutoStory_Checkpoint_002().run_checkpoint(env, context, options, stats);
                AutoStory_Checkpoint_003().run_checkpoint(env, context, options, stats);

                context.wait_for_all_requests();
                env.console.log("End Segment " + name(), COLOR_GREEN);
            }

            void checkpoint_000(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) {
                checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
                    [&](size_t /*attempt*/) {
                        // Segment 00-00: Intro and starter selection
                        tutorial_part_1_navigation(env, env.console, context);
                    }
                );
            }
            void checkpoint_001(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) {
                checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
                    [&](size_t /*attempt*/){
                        // Segment 00-01: Starter acquisition
                        starter_nav(env.console, context);
                        // Segment 00-00 starter selection
                        switch (options.starter_choice) {
                        case StarterChoice::TURTWIG:  select_turtwig(env.console, context);  break;
                        case StarterChoice::CHIMCHAR: select_chimchar(env.console, context); break;
                        case StarterChoice::PIPLUP:   select_piplup(env.console, context);   break;
                        }
                        //Fight Starly
                        fight_starly(env.console, context);
                    }
                );
            }
            void checkpoint_002(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) {
                checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
                    [&](size_t /*attempt*/){
                        // Segment 00-002: Pokedex acquisition
                        tutorial_part_2_navigation(env, env.console, context);
                    }
                );
            }
            void checkpoint_003(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) {
                checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
                    [&](size_t /*attempt*/){
                        // Segment 00-003: Poketech acquisition
                        tutorial_part_3_navigation(env, env.console, context);
                    }
                );
            }

            // ========================
            // Checkpoint 000 Wrapper
            // ========================
            std::string AutoStory_Checkpoint_000::name()       const { return "000 - " + AutoStory_Segment_00().name(); }
            std::string AutoStory_Checkpoint_000::start_text() const { return "Game launched, Standing in the player's room after tv stops playing"; }
            std::string AutoStory_Checkpoint_000::end_text()   const { return "Standing outside Lake Verity before picking starter."; }

            void AutoStory_Checkpoint_000::run_checkpoint(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) const {
                checkpoint_000(env, context, options, stats);
            }

            // ========================
            // Checkpoint 001 Wrapper
            // ========================

            std::string AutoStory_Checkpoint_001::name()       const{ return "001 - " + AutoStory_Segment_00().name(); }
            std::string AutoStory_Checkpoint_001::start_text() const{ return AutoStory_Checkpoint_000().end_text(); }
            std::string AutoStory_Checkpoint_001::end_text()   const{ return "Standing outside Lake Verity after picking starter."; }

            void AutoStory_Checkpoint_001::run_checkpoint(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) const{
                checkpoint_001(env, context, options, stats);
            }


            // ========================
            // Checkpoint 002 Wrapper
            // ========================

            std::string AutoStory_Checkpoint_002::name()       const{ return "002 - " + AutoStory_Segment_00().name(); }
            std::string AutoStory_Checkpoint_002::start_text() const{ return AutoStory_Checkpoint_001().end_text(); }
            std::string AutoStory_Checkpoint_002::end_text()   const{ return "Standing in front of The professor after recieving the Pokedex"; }

            void AutoStory_Checkpoint_002::run_checkpoint(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) const{
                checkpoint_002(env, context, options, stats);
            }
            // ========================
            // Checkpoint 003 Wrapper
            // ========================

            std::string AutoStory_Checkpoint_003::name()       const{ return "003 - " + AutoStory_Segment_00().name(); }
            std::string AutoStory_Checkpoint_003::start_text() const{ return AutoStory_Checkpoint_002().end_text(); }
            std::string AutoStory_Checkpoint_003::end_text()   const{ return "Standing in front of PC. Recieved the Poketech. Ready to deposit starter"; }

            void AutoStory_Checkpoint_003::run_checkpoint(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) const{
                checkpoint_003(env, context, options, stats);
            }
        }
    }
}
