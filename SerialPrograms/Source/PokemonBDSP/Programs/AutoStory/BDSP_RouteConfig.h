#pragma once

#include <string>
#include "Common/Cpp/DateTime.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            enum class BDSPRouteType {
                OutsideHelp,
                NoOutsideHelp
            };

            enum class BDSPStarter {
                Chimchar,
                Piplup,
                Turtwig,
                None
            };

            struct BDSPRouteConfig {

                DateTime set_datetime = {2023, 1, 1, 0, 0, 0};

                BDSPRouteType route_type = BDSPRouteType::NoOutsideHelp;

                bool starter_selected = false;
                BDSPStarter starter = BDSPStarter::None;
                bool starter_shiny = false;

                bool catch_box_legendary = false;

            };

        }
    }
}