/*base rout to*/

#include "BaseRoute.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {
            // Tutorial Part 1: Initial Game Navigation
            // Routes the player from their starting house to the starter Pokémon selection screen.
            // This segment handles dialogue with Mom and Barry, navigating through Twinleaf Town,
            // and reaching the point where the player selects their starting Pokémon.
            // Begins from player's room after character creation is complete.
            
                        
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

            // Initial game navigation from player's room to starter selection
            static void intro_navigation(VideoStream& stream, ProControllerContext& context) {
                // Navigate from player's room to starter selection screen
                // This uses precise button timing and is player-specific to their room layout
                // Increased timing for reliable button registration
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

                // Wait for mom to start talking
                wait_for_dialogue(stream, context, "Phase 2 (Mom)");

                pbf_mash_button(context, BUTTON_A, 5000ms);
                pbf_wait(context, 5000ms); // Wait for dialogue box to disappear

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

                // mom warning about grass - wait for dialogue
                wait_for_dialogue(stream, context, "Phase 4 (Mom Warning)");

                pbf_mash_button(context, BUTTON_A, 2500ms);
                pbf_wait(context, 800ms); // Wait for dialogue box to disappear

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

                // Barry talking at door - wait for dialogue
                wait_for_dialogue(stream, context, "Phase 6 (Barry 2nd)");

				pbf_mash_button(context, BUTTON_A, 500ms);
				pbf_wait(context, 800ms); // Wait for dialogue box to disappear

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

				// barry talking upstairs section - wait for dialogue
				wait_for_dialogue(stream, context, "Phase 8 (Barry Final)");

				pbf_mash_button(context, BUTTON_A, 500ms);
				pbf_wait(context, 800ms); // Wait for dialogue box to disappear

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

				// Barry on Route 201 dialogue - wait for dialogue
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

            // Tutorial Part 2: Starter Selection and Early Game Navigation
            // This segment handles the starter Pokémon selection and the initial gameplay
            // after leaving the starting town. Add specific navigation and dialogue sequences here.
            static void starter_selection_navigation(VideoStream& stream, ProControllerContext& context) {
                // TODO: Add navigation for starter selection screen
                // Example: Movement to select starter, button presses to confirm selection

                stream.log("Tutorial Part 2: Starter selection and early game navigation");

                // Add specific button sequences for your starter selection sequence here
                // pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                // pbf_mash_button(context, BUTTON_A, 500ms);
                // wait_for_dialogue(stream, context, "Phase X (Description)");
            }

            const std::vector<void(*)(VideoStream&, ProControllerContext&)> tutorial_part_1 = {
                intro_navigation
            };

            const std::vector<void(*)(VideoStream&, ProControllerContext&)> tutorial_part_2 = {
                starter_selection_navigation
            };

        }
    }
}
