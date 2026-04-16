#pragma once

#include <vector>
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            // Route type modifiers for "No Outside Help" path
            // These segments handle the story progression when the player doesn't use outside help

            extern const std::vector<void(*)(VideoStream&, ProControllerContext&)> no_outside_help_segments;

        }
    }
}
