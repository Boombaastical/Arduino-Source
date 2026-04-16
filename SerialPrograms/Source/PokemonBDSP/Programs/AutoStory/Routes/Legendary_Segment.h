#pragma once

#include <vector>
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            // Legendary Pokemon catching segments
            // These segments handle catching the box legendary Pokemon

            extern const std::vector<void(*)(VideoStream&, ProControllerContext&)> legendary_segments;

        }
    }
}
