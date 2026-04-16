#pragma once

#include <vector>
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            // Turtwig-specific story segments

            extern const std::vector<void(*)(VideoStream&, ProControllerContext&)> turtwig_segments;

        }
    }
}
