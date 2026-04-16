#include "Starter_Turtwig.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            static void select_turtwig(VideoStream& stream, ProControllerContext& context) {
                stream.log("[DEBUG] Turtwig Selection: Starting starter selection");

                // Turtwig is at index 0 (no scrolling needed - already selected)
                stream.log("[DEBUG] Turtwig Selection: Turtwig already selected (index 0)");

                // Select the starter using ZL
                stream.log("[DEBUG] Turtwig Selection: Pressing ZL to select");
                pbf_press_button(context, BUTTON_ZL, 160ms, 240ms);
                context.wait_for_all_requests();

                // Wait for selection dialogue to appear (try cyan then red)
                stream.log("[DEBUG] Turtwig Selection: Waiting for confirmation dialogue");
                ShortDialogWatcher dialog_watcher(COLOR_CYAN);
                int dialog_ret = run_until<ProControllerContext>(
                    stream, context,
                    [&](ProControllerContext& context) {
                        context.wait_for(std::chrono::milliseconds(5000));
                    },
                    { dialog_watcher }
                );

                if (dialog_ret != 0) {
                    stream.log("[DEBUG] Turtwig Selection: Cyan detection failed, trying RED...");
                    ShortDialogWatcher dialog_watcher_red(COLOR_RED);
                    dialog_ret = run_until<ProControllerContext>(
                        stream, context,
                        [&](ProControllerContext& context) {
                            context.wait_for(std::chrono::milliseconds(5000));
                        },
                        { dialog_watcher_red }
                    );
                }

                if (dialog_ret == 0) {
                    stream.log("[DEBUG] Turtwig Selection: Confirmation dialogue detected");
                } else {
                    stream.log("[WARNING] Turtwig Selection: Dialogue detection failed with both colors, proceeding anyway");
                }

                pbf_wait(context, 400ms);

                // Confirm selection by pressing up and ZL
                stream.log("[DEBUG] Turtwig Selection: Confirming selection");
                pbf_press_dpad(context, DPAD_UP, 80ms, 400ms);
                pbf_press_button(context, BUTTON_ZL, 80ms, 5000ms);
                context.wait_for_all_requests();

                stream.log("[DEBUG] Turtwig Selection: Selection complete");
            }

            // Turtwig route segments
            const std::vector<void(*)(VideoStream&, ProControllerContext&)> turtwig_segments = {
                select_turtwig
            };

        }
    }
}
