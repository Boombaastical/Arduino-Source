#pragma once

#include <vector>
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            // Route type modifiers for "Outside Help" path
            // These segments handle the story progression when the player uses NPCs/outside help

            extern const std::vector<void(*)(VideoStream&, ProControllerContext&)> outside_help_segments;

        }
    }
}
