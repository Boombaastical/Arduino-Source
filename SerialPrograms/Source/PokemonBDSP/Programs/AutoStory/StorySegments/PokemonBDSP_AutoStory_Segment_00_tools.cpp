/*  BDSP AutoStory - Segment 00 Tools
 *
 *  Helper implementations for Segment 00.
 *
 */



#include "PokemonBDSP_AutoStory_Segment_00_tools.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"

#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"

#include "PokemonBDSP/Inference/PokemonBDSP_MenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"

#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_ExperienceGainDetector.h"

#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP/Programs/PokemonBDSP_BasicCatcher.h"

using namespace std::chrono_literals;

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            // ---------------------------------------------------------------------------
            // BattleDialogWatcher
            // ---------------------------------------------------------------------------

            BattleDialogWatcher::BattleDialogWatcher(Color color)
                : DetectorToFinder(
                    "BattleDialogWatcher",
                    std::chrono::milliseconds(250),
                    color
                )
            {
            }

            // ---------------------------------------------------------------------------
            // Time helper
            // ---------------------------------------------------------------------------

            void set_time_fast(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                const DateTime& target
            ) {
                env.log("[AutoStory] Fast date set starting...", COLOR_ORANGE);

                NintendoSwitch::go_home(env.console, context);

                NintendoSwitch::home_to_date_time(
                    env.console,
                    context,
                    true
                );

                pbf_press_button(
                    context,
                    BUTTON_A,
                    80ms,
                    240ms
                );

                NintendoSwitch::DateChangeWatcher date_reader(env.console);

                int ret = wait_until(
                    env.console,
                    context,
                    10s,
                    {
                        date_reader
                    }
                );

                if (ret < 0) {
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Failed to enter Date Change screen.",
                        env.console
                    );
                }

                {
                    VideoOverlaySet overlays(env.console.overlay());

                    date_reader.make_overlays(overlays);

                    date_reader.set_date(
                        env.program_info(),
                        env.console,
                        context,
                        target
                    );
                }

                pbf_press_button(
                    context,
                    BUTTON_A,
                    160ms,
                    240ms
                );

                pbf_press_button(
                    context,
                    BUTTON_HOME,
                    160ms,
                    ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0
                );

                NintendoSwitch::resume_game_from_home(
                    env.console,
                    context,
                    false
                );

                context.wait_for_all_requests();
                context.wait_for(500ms);
            }

            // ---------------------------------------------------------------------------
            // Starter selection
            // ---------------------------------------------------------------------------

            void select_starter_internal(
                VideoStream& stream,
                ProControllerContext& context,
                size_t right_presses,
                const std::string& name
            ) {
                stream.log(
                    "[AutoStory] Selecting " + name + "...",
                    COLOR_BLUE
                );

                for (size_t c = 0; c < right_presses; c++) {
                    pbf_press_dpad(
                        context,
                        DPAD_RIGHT,
                        160ms,
                        840ms
                    );
                }

                pbf_press_button(
                    context,
                    BUTTON_ZL,
                    160ms,
                    240ms
                );

                context.wait_for_all_requests();

                ShortDialogWatcher watcher(COLOR_CYAN);

                run_until<ProControllerContext>(
                    stream,
                    context,
                    [](ProControllerContext& ctx) {
                        ctx.wait_for(5000ms);
                    },
        {
            watcher
        }
                );

                pbf_wait(context, 400ms);

                pbf_press_dpad(
                    context,
                    DPAD_UP,
                    80ms,
                    400ms
                );

                pbf_press_button(
                    context,
                    BUTTON_ZL,
                    80ms,
                    5000ms
                );

                context.wait_for_all_requests();

                stream.log(
                    "[AutoStory] " + name + " selected.",
                    COLOR_GREEN
                );
            }

            void select_turtwig(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                select_starter_internal(stream, context, 0, "Turtwig");
            }

            void select_chimchar(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                select_starter_internal(stream, context, 1, "Chimchar");
            }

            void select_piplup(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                select_starter_internal(stream, context, 2, "Piplup");
            }

            // ---------------------------------------------------------------------------
            // Potion helper
            // ---------------------------------------------------------------------------

            void use_potion_first_pokemon(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                stream.log(
                    "[AutoStory] Using Potion...",
                    COLOR_BLUE
                );

                MenuWatcher menu(COLOR_RED);
                ShortDialogWatcher dialog(COLOR_YELLOW);

                pbf_press_button(
                    context,
                    BUTTON_X,
                    80ms,
                    500ms
                );

                int menu_ret = wait_until(
                    stream,
                    context,
                    5s,
                    {
                        {menu},
                    }
                    );

                if (menu_ret < 0) {
                    stream.log(
                        "[AutoStory] Failed to detect menu.",
                        COLOR_RED
                    );
                    return;
                }

                pbf_press_dpad(
                    context,
                    DPAD_RIGHT,
                    80ms,
                    200ms
                );

                pbf_press_button(
                    context,
                    BUTTON_A,
                    80ms,
                    1500ms
                );

                pbf_press_button(
                    context,
                    BUTTON_A,
                    80ms,
                    1000ms
                );

                pbf_press_button(
                    context,
                    BUTTON_A,
                    80ms,
                    500ms
                );

                pbf_press_button(
                    context,
                    BUTTON_A,
                    80ms,
                    1000ms
                );

                pbf_press_button(
                    context,
                    BUTTON_A,
                    80ms,
                    1500ms
                );

                int dialog_ret = wait_until(
                    stream,
                    context,
                    5s,
                    {
                        {dialog},
                    }
                    );

                if (dialog_ret >= 0) {
                    pbf_mash_button(
                        context,
                        BUTTON_B,
                        3000ms
                    );
                }

                context.wait_for_all_requests();

                pbf_press_button(context, BUTTON_B, 80ms, 300ms);
                pbf_press_button(context, BUTTON_B, 80ms, 300ms);
                pbf_press_button(context, BUTTON_B, 80ms, 300ms);

                context.wait_for_all_requests();

                pbf_wait(context, 1000ms);

                stream.log(
                    "[AutoStory] Potion complete.",
                    COLOR_GREEN
                );
            }

            // ---------------------------------------------------------------------------
            // Natalie battle
            // ---------------------------------------------------------------------------

            void fight_Natalie(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                stream.log(
                    "[AutoStory] Starting Natalie battle...",
                    COLOR_BLUE
                );

                BattleMenuWatcher battle_menu(BattleType::TRAINER);
                ExperienceGainWatcher exp_gain;

                int start_ret = wait_until(
                    stream,
                    context,
                    30s,
                    {
                        {battle_menu},
                    }
                    );

                if (start_ret < 0) {
                    stream.log(
                        "[AutoStory] Failed to detect battle menu.",
                        COLOR_RED
                    );
                    return;
                }

                while (true) {

                    pbf_press_button(
                        context,
                        BUTTON_A,
                        120ms,
                        500ms
                    );

                    pbf_press_dpad(
                        context,
                        DPAD_DOWN,
                        80ms,
                        200ms
                    );

                    pbf_press_dpad(
                        context,
                        DPAD_DOWN,
                        80ms,
                        300ms
                    );

                    pbf_press_button(
                        context,
                        BUTTON_A,
                        120ms,
                        1000ms
                    );

                    context.wait_for_all_requests();

                    int ret = wait_until(
                        stream,
                        context,
                        60s,
                        {
                            {battle_menu},
                            {exp_gain},
                        }
                        );

                    if (ret == 0) {
                        continue;
                    }

                    if (ret == 1) {

                        pbf_mash_button(
                            context,
                            BUTTON_B,
                            5000ms
                        );

                        context.wait_for_all_requests();

                        int next_ret = wait_until(
                            stream,
                            context,
                            10s,
                            {
                                {battle_menu},
                            }
                            );

                        if (next_ret == 0) {
                            continue;
                        }

                        break;
                    }

                    stream.log(
                        "[AutoStory] Battle timeout.",
                        COLOR_RED
                    );

                    return;
                }

                pbf_mash_button(
                    context,
                    BUTTON_B,
                    15000ms
                );

                context.wait_for_all_requests();

                pbf_wait(context, 3000ms);

                stream.log(
                    "[AutoStory] Natalie battle complete.",
                    COLOR_GREEN
                );
            }
            // ---------------------------------------------------------------------------
            // Starly battle
            // ---------------------------------------------------------------------------

            void fight_starly(
                VideoStream& stream,
                ProControllerContext& context
            ){
                stream.log("Starting Starly battle...");

                pbf_wait(context, 5000ms);

                pbf_mash_button(context, BUTTON_A, 5000ms);

                for (int i = 0; i < 6; i++){
                    pbf_press_button(context, BUTTON_A, 100ms, 2000ms);
                    pbf_wait(context, 3000ms);
                }

                pbf_mash_button(context, BUTTON_A, 8000ms);

                stream.log("Starly battle complete.");

                wait_for_dialogue(stream, context, "Phase 9 (post battle text)");

                pbf_mash_button(context, BUTTON_B, 20000ms);

                wait_for_dialogue(stream, context, "Phase 9 (post battle text)");

                pbf_mash_button(context, BUTTON_B, 5000ms);

                context.wait_for_all_requests();
            }

            // ---------------------------------------------------------------------------
            // Tristan battle
            // ---------------------------------------------------------------------------

            void fight_tristan(
                VideoStream& stream,
                ProControllerContext& context
            ){
                stream.log("Starting Tristan battle...");

                BattleMenuWatcher battle_menu(BattleType::TRAINER);
                ExperienceGainWatcher exp_gain;

                int start_ret = wait_until(
                    stream,
                    context,
                    30s,
                    {
                        {battle_menu},
                    }
                );

                if (start_ret < 0){
                    stream.log(
                        "[AutoStory] Failed to detect battle menu.",
                        COLOR_RED
                    );
                    return;
                }

                while (true){

                    pbf_press_button(context, BUTTON_A, 120ms, 1000ms);

                    context.wait_for_all_requests();

                    int ret = wait_until(
                        stream,
                        context,
                        60s,
                        {
                            {battle_menu},
                            {exp_gain},
                        }
                    );

                    if (ret == 0){
                        continue;
                    }

                    if (ret == 1){

                        pbf_mash_button(
                            context,
                            BUTTON_B,
                            5000ms
                        );

                        context.wait_for_all_requests();

                        int next_ret = wait_until(
                            stream,
                            context,
                            10s,
                            {
                                {battle_menu},
                            }
                        );

                        if (next_ret == 0){
                            continue;
                        }

                        break;
                    }

                    stream.log(
                        "[AutoStory] Battle timeout.",
                        COLOR_RED
                    );

                    return;
                }

                pbf_mash_button(
                    context,
                    BUTTON_B,
                    15000ms
                );

                context.wait_for_all_requests();

                pbf_wait(context, 3000ms);

                stream.log(
                    "[AutoStory] Tristan battle complete.",
                    COLOR_GREEN
                );
            }

            // ---------------------------------------------------------------------------
            // Catch 1 Pokemon
            // ---------------------------------------------------------------------------

            void catch_1_pokemon(
                VideoStream& stream,
                ProControllerContext& context
            ){
                stream.log("[AutoStory] Starting catch_1_pokemon helper...", COLOR_BLUE);

                save_game(context);

                context.wait_for_all_requests();
                pbf_wait(context, 1000ms);

                BlackScreenWatcher battle_start;
                BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
                BattleDialogWatcher battle_dialog(COLOR_YELLOW);

                bool encounter_found = false;

                while (!encounter_found){

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

                pbf_wait(context, 3000ms);

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

                pbf_mash_button(context, BUTTON_B, 10000ms);

                context.wait_for_all_requests();

                pbf_wait(context, 3000ms);
            }

            // ---------------------------------------------------------------------------
            // Movement Helpers
            // ---------------------------------------------------------------------------

            void walk_right_until_on_path(
                VideoStream& stream,
                ProControllerContext& context
            ){
                for (size_t step = 0; step < 18; step++){
                    pbf_move_left_joystick(context, {1, 0}, 200ms, 100ms);
                }

                context.wait_for_all_requests();

                stream.log("[AutoStory] Completed walk_right_until_on_path.", COLOR_GREEN);
            }

            void walk_up_through_grass_2(
                VideoStream& stream,
                ProControllerContext& context
            ){
                for (size_t step = 0; step < 18; step++){
                    pbf_move_left_joystick(context, {0, 1}, 200ms, 100ms);
                }

                context.wait_for_all_requests();

                stream.log("[AutoStory] Completed walk_up_through_grass_2.", COLOR_GREEN);
            }

            void walk_right_through_grass_2(
                VideoStream& stream,
                ProControllerContext& context
            ){
                for (size_t step = 0; step < 18; step++){
                    pbf_move_left_joystick(context, {1, 0}, 200ms, 100ms);
                }

                context.wait_for_all_requests();

                stream.log("[AutoStory] Completed walk_right_through_grass_2.", COLOR_GREEN);
            }

            void walk_left_through_grass_3(
                VideoStream& stream,
                ProControllerContext& context
            ){
                for (size_t step = 0; step < 18; step++){
                    pbf_move_left_joystick(context, {-1, 0}, 200ms, 100ms);
                }

                context.wait_for_all_requests();

                stream.log("[AutoStory] Completed walk_left_through_grass_3.", COLOR_GREEN);
            }

            void walk_up_through_grass_3(
                VideoStream& stream,
                ProControllerContext& context
            ){
                BlackScreenWatcher battle_start;
                BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
                BattleDialogWatcher battle_dialog(COLOR_YELLOW);

                for (size_t step = 0; step < 18; step++){

                    context.wait_for_all_requests();

                    pbf_move_left_joystick(context, {0, 1}, 200ms, 0ms);

                    context.wait_for_all_requests();

                    int battle_ret = wait_until(
                        stream,
                        context,
                        2500ms,
                        {
                            {battle_start},
                            {battle_menu},
                            {battle_dialog},
                        }
                    );

                    if (battle_ret >= 0){

                        stream.log(
                            "[AutoStory] battle detected during movement.",
                            COLOR_ORANGE
                        );

                        pbf_mash_button(context, BUTTON_B, 3000ms);

                        context.wait_for_all_requests();

                        pbf_wait(context, 1000ms);
                    }
                }

                stream.log("[AutoStory] Completed fixed up movement.", COLOR_GREEN);
            }

            void walk_right_to_first_trainer(
                VideoStream& stream,
                ProControllerContext& context
            ){
                BlackScreenWatcher battle_start;
                BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
                BattleDialogWatcher battle_dialog(COLOR_YELLOW);

                for (size_t step = 0; step < 5; step++){

                    context.wait_for_all_requests();

                    pbf_move_left_joystick(context, {1, 0}, 200ms, 0ms);

                    context.wait_for_all_requests();

                    ShortDialogWatcher trainer_dialog(COLOR_YELLOW);

                    int battle_ret = wait_until(
                        stream,
                        context,
                        2500ms,
                        {
                            {trainer_dialog},
                            {battle_start},
                            {battle_menu},
                        }
                    );

                    if (battle_ret >= 0){

                        stream.log(
                            "[AutoStory] Encounter detected.",
                            COLOR_ORANGE
                        );

                        if (battle_ret == 0){

                            stream.log(
                                "[AutoStory] Trainer dialogue detected.",
                                COLOR_YELLOW
                            );

                            auto deadline = std::chrono::steady_clock::now() + 15s;

                            while (std::chrono::steady_clock::now() < deadline){

                                pbf_press_button(context, BUTTON_A, 80ms, 200ms);

                                context.wait_for_all_requests();

                                int start_ret = wait_until(
                                    stream,
                                    context,
                                    1000ms,
                                    {
                                        {battle_start},
                                        {battle_menu},
                                    }
                                );

                                if (start_ret >= 0){
                                    break;
                                }
                            }
                        }

                        stream.log(
                            "[AutoStory] Starting Tristan battle...",
                            COLOR_ORANGE
                        );

                        context.wait_for_all_requests();

                        pbf_wait(context, 3000ms);

                        fight_tristan(stream, context);

                        context.wait_for_all_requests();

                        pbf_wait(context, 1500ms);

                        continue;
                    }
                }

                stream.log(
                    "[AutoStory] Completed fixed right movement.",
                    COLOR_GREEN
                );
            }
        }
    }
}
