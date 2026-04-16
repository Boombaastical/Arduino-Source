#pragma once

#include <vector>

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

#include "BDSP_RouteConfig.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            class BDSPRouteBuilder {

            public:

                static std::vector<void(*)(VideoStream&, ProControllerContext&)> build_route(
                    const BDSPRouteConfig& config
                );

            private:

                static void apply_base_route(
                    std::vector<void(*)(VideoStream&, ProControllerContext&)>& route
                );

                static void apply_tutorial_part_2(
                    std::vector<void(*)(VideoStream&, ProControllerContext&)>& route
                );

                static void apply_route_type(
                    std::vector<void(*)(VideoStream&, ProControllerContext&)>& route,
                    BDSPRouteType type
                );

                static void apply_starter(
                    std::vector<void(*)(VideoStream&, ProControllerContext&)>& route,
                    BDSPStarter starter
                );

                static void apply_legendary(
                    std::vector<void(*)(VideoStream&, ProControllerContext&)>& route,
                    bool enabled
                );

            };

        }
    }
}