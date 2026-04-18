#include "Starter_Chimchar.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            static void select_chimchar(VideoStream& stream, ProControllerContext& context) {
                stream.log("[DEBUG] Chimchar Selection: Starting starter selection");

                // Scroll to Chimchar (index 1: right once from Turtwig)
                stream.log("[DEBUG] Chimchar Selection: Scrolling to Chimchar");
                pbf_press_dpad(context, DPAD_RIGHT, 160ms, 840ms);

                // Select the starter using ZL
                stream.log("[DEBUG] Chimchar Selection: Pressing ZL to select");
                pbf_press_button(context, BUTTON_ZL, 160ms, 240ms);
                context.wait_for_all_requests();

                // Wait for selection dialogue to appear (try cyan then red)
                stream.log("[DEBUG] Chimchar Selection: Waiting for confirmation dialogue");
                ShortDialogWatcher dialog_watcher(COLOR_CYAN);
                int dialog_ret = run_until<ProControllerContext>(
                    stream, context,
                    [&](ProControllerContext& context) {
                        context.wait_for(std::chrono::milliseconds(5000));
                    },
                    { dialog_watcher }
                );

                if (dialog_ret != 0) {
                    stream.log("[DEBUG] Chimchar Selection: Cyan detection failed, trying RED...");
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
                    stream.log("[DEBUG] Chimchar Selection: Confirmation dialogue detected");
                } else {
                    stream.log("[WARNING] Chimchar Selection: Dialogue detection failed with both colors, proceeding anyway");
                }

                pbf_wait(context, 400ms);

                // Confirm selection by pressing up and ZL
                stream.log("[DEBUG] Chimchar Selection: Confirming selection");
                pbf_press_dpad(context, DPAD_UP, 80ms, 400ms);
                pbf_press_button(context, BUTTON_ZL, 80ms, 5000ms);
                context.wait_for_all_requests();

                stream.log("[DEBUG] Chimchar Selection: Selection complete");
            }

            // Chimchar route segments
            const std::vector<void(*)(VideoStream&, ProControllerContext&)> chimchar_segments = {
                select_chimchar
            };

        }
    }
}
