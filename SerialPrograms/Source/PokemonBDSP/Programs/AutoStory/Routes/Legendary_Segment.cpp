#include "Legendary_Segment.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            static void approach_legendary(VideoStream& stream, ProControllerContext& context) {
                // Approach and interact with legendary Pokemon
                pbf_wait(context, 2000ms);
                pbf_press_button(context, BUTTON_A, 100ms, 300ms);
                pbf_wait(context, 2000ms);
            }

            static void enter_legendary_battle(VideoStream& stream, ProControllerContext& context) {
                // Enter the legendary battle
                pbf_press_button(context, BUTTON_A, 100ms, 100ms);
                pbf_wait(context, 3000ms);
            }

            static void weaken_legendary(VideoStream& stream, ProControllerContext& context) {
                // Use moves to weaken legendary Pokemon
                pbf_press_button(context, BUTTON_A, 100ms, 100ms);
                pbf_wait(context, 1500ms);
                pbf_press_dpad(context, DPAD_UP, 100ms, 100ms);
                pbf_press_button(context, BUTTON_A, 100ms, 100ms);
                pbf_wait(context, 2000ms);
            }

            static void throw_pokeball_at_legendary(VideoStream& stream, ProControllerContext& context) {
                // Throw Pokeballs to catch the legendary
                for (int i = 0; i < 3; i++) {
                    pbf_press_button(context, BUTTON_A, 100ms, 100ms);
                    pbf_wait(context, 1000ms);
                    pbf_press_dpad(context, DPAD_DOWN, 100ms, 100ms);
                    pbf_press_button(context, BUTTON_A, 100ms, 100ms);
                    pbf_wait(context, 2500ms);
                }
            }

            static void confirm_legendary_catch(VideoStream& stream, ProControllerContext& context) {
                // Confirm catching the legendary
                pbf_wait(context, 1500ms);
                pbf_press_button(context, BUTTON_A, 100ms, 100ms);
                pbf_wait(context, 2000ms);
            }

            static void exit_legendary_encounter(VideoStream& stream, ProControllerContext& context) {
                // Exit the legendary encounter area
                pbf_press_button(context, BUTTON_A, 100ms, 100ms);
                pbf_wait(context, 2000ms);
            }

            // Legendary catching segments
            const std::vector<void(*)(VideoStream&, ProControllerContext&)> legendary_segments = {
                approach_legendary,
                enter_legendary_battle,
                weaken_legendary,
                throw_pokeball_at_legendary,
                confirm_legendary_catch,
                exit_legendary_encounter
            };

        }
    }
}
