#include "NoOutsideHelp_Modifiers.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            static void skip_help_prompt(VideoStream& stream, ProControllerContext& context) {
                // Skip or decline outside help option
                pbf_wait(context, 1000ms);
                pbf_press_button(context, BUTTON_B, 100ms, 300ms);
                pbf_wait(context, 1500ms);
            }

            static void proceed_without_help(VideoStream& stream, ProControllerContext& context) {
                // Proceed with the story without outside help
                pbf_press_button(context, BUTTON_A, 100ms, 100ms);
                pbf_wait(context, 2000ms);
            }

            static void solo_progression(VideoStream& stream, ProControllerContext& context) {
                // Solo progression through story events
                pbf_press_button(context, BUTTON_A, 100ms, 100ms);
                pbf_wait(context, 2000ms);
                pbf_press_button(context, BUTTON_A, 100ms, 100ms);
                pbf_wait(context, 1500ms);
            }

            static void continue_solo_story(VideoStream& stream, ProControllerContext& context) {
                // Continue story after solo progression
                pbf_press_button(context, BUTTON_A, 100ms, 100ms);
                pbf_wait(context, 2000ms);
            }

            // No outside help route segments
            const std::vector<void(*)(VideoStream&, ProControllerContext&)> no_outside_help_segments = {
                skip_help_prompt,
                proceed_without_help,
                solo_progression,
                continue_solo_story
            };

        }
    }
}
