#include "OutsideHelp_Modifiers.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            static void access_outside_help_option(VideoStream& stream, ProControllerContext& context) {
                // Navigate and select outside help option
                pbf_wait(context, 1000ms);
                pbf_press_dpad(context, DPAD_DOWN, 100ms, 300ms);
                pbf_press_button(context, BUTTON_A, 100ms, 300ms);
                pbf_wait(context, 1500ms);
            }

            static void select_help_npc(VideoStream& stream, ProControllerContext& context) {
                // Select an NPC to help
                pbf_wait(context, 500ms);
                pbf_press_button(context, BUTTON_A, 100ms, 300ms);
                pbf_wait(context, 2000ms);
            }

            static void execute_npc_sequence(VideoStream& stream, ProControllerContext& context) {
                // Execute the NPC help sequence
                pbf_press_button(context, BUTTON_A, 100ms, 100ms);
                pbf_wait(context, 2000ms);
                pbf_press_button(context, BUTTON_A, 100ms, 100ms);
                pbf_wait(context, 1500ms);
            }

            static void continue_with_help(VideoStream& stream, ProControllerContext& context) {
                // Continue story after outside help
                pbf_press_button(context, BUTTON_A, 100ms, 100ms);
                pbf_wait(context, 2000ms);
            }

            // Outside help route segments
            const std::vector<void(*)(VideoStream&, ProControllerContext&)> outside_help_segments = {
                access_outside_help_option,
                select_help_npc,
                execute_npc_sequence,
                continue_with_help
            };

        }
    }
}
