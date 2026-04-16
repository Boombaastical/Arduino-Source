#include "TutorialPart2.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            // Helper function to wait for dialogue box to appear (tries multiple colors)
            static void wait_for_dialogue(VideoStream& stream, ProControllerContext& context, const std::string& phase_name) {
                // Try cyan first (most common)
                ShortDialogWatcher dialog_watcher(COLOR_CYAN);
                int dialog_ret = run_until<ProControllerContext>(
                    stream, context,
                    [&](ProControllerContext& context) {
                        context.wait_for(std::chrono::milliseconds(5000));
                    },
                    { dialog_watcher }
                );

                if (dialog_ret == 0) {
                    return;
                }

                // Fallback: Try red if cyan failed
                ShortDialogWatcher dialog_watcher_red(COLOR_RED);
                dialog_ret = run_until<ProControllerContext>(
                    stream, context,
                    [&](ProControllerContext& context) {
                        context.wait_for(std::chrono::milliseconds(5000));
                    },
                    { dialog_watcher_red }
                );

                if (dialog_ret == 0) {
                    return;
                }

                stream.log("[WARNING] " + phase_name + ": Dialogue detection failed with both colors, proceeding anyway");
            }

            // Post-starter selection navigation through initial gameplay
            void tutorial_part_2_navigation(VideoStream& stream, ProControllerContext& context) {
                // Navigate from starter selection location to professor meeting point
                // This segment routes through early map progression
                
                pbf_press_dpad(context, DPAD_DOWN, 20, 105);
                pbf_press_dpad(context, DPAD_DOWN, 20, 105);
                pbf_press_dpad(context, DPAD_DOWN, 20, 105);
                pbf_press_dpad(context, DPAD_DOWN, 20, 105);
                pbf_press_dpad(context, DPAD_DOWN, 20, 105);
                pbf_press_dpad(context, DPAD_DOWN, 20, 105);
                pbf_press_dpad(context, DPAD_DOWN, 20, 105);
                pbf_press_dpad(context, DPAD_DOWN, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);

                // Wait for professor dialogue
                wait_for_dialogue(stream, context, "Phase 1 (Professor Introduction)");

                // Mash through professor's initial greeting
                pbf_mash_button(context, BUTTON_A, 200);
                pbf_wait(context, 1000ms); // Wait for dialogue progression

                // Navigate through subsequent areas and dialogue sequences
                // TODO: Continue implementing navigation and dialogue handling for the rest of tutorial_part_2
                // This section would include:
                // - Meeting with Professor and receiving Pokedex
                // - Initial dialogue sequences
                // - Navigation to early game areas
                // - First trainer encounters
            }

        }
    }
}
