#pragma once

#include <vector>
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            // Tutorial Part 1: Initial Game Navigation
            // Routes the player from their starting house to the starter Pokémon selection screen.
            // This segment handles dialogue with Mom and Barry, navigating through Twinleaf Town,
            // and reaching the point where the player selects their starting Pokémon.

            extern const std::vector<void(*)(VideoStream&, ProControllerContext&)> tutorial_part_1;

        }
    }
}
