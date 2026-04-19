#include "PokemonBDSP_RouteBuilder.h"

#include "Routes/BaseRoute.h"
#include "Routes/TutorialPart2.h"

#include "Routes/Starter_Chimchar.h"
#include "Routes/Starter_Piplup.h"
#include "Routes/Starter_Turtwig.h"

#include "Routes/OutsideHelp_Modifiers.h"
#include "Routes/NoOutsideHelp_Modifiers.h"

#include "Routes/Legendary_Segment.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            std::vector<void(*)(VideoStream&, ProControllerContext&)> BDSPRouteBuilder::build_route(
                const BDSPRouteConfig& config
            ) {
                std::vector<void(*)(VideoStream&, ProControllerContext&)> route;

                // Date/time setting happens in the main program before route execution
                // This is handled in PokemonBDSP_AutoStory::program()

                apply_base_route(route);

                apply_tutorial_part_2(route);

                apply_route_type(route, config.route_type);

                // Apply starter - default to Chimchar if not selected
                BDSPStarter starter_to_use = config.starter_selected ? config.starter : BDSPStarter::Chimchar;
                apply_starter(route, starter_to_use);

                apply_legendary(route, config.catch_box_legendary);

                return route;
            }

            void BDSPRouteBuilder::apply_base_route(
                std::vector<void(*)(VideoStream&, ProControllerContext&)>& route
            ) {
                route.insert(
                    route.end(),
                    tutorial_part_1.begin(),
                    tutorial_part_1.end()
                );
            }

            void BDSPRouteBuilder::apply_tutorial_part_2(
                std::vector<void(*)(VideoStream&, ProControllerContext&)>& route
            ) {
                //route.push_back(tutorial_part_2_navigation);
            }

            void BDSPRouteBuilder::apply_route_type(
                std::vector<void(*)(VideoStream&, ProControllerContext&)>& route,
                BDSPRouteType type
            ) {

                if (type == BDSPRouteType::OutsideHelp) {

                    route.insert(
                        route.end(),
                        outside_help_segments.begin(),
                        outside_help_segments.end()
                    );

                }
                else {

                    route.insert(
                        route.end(),
                        no_outside_help_segments.begin(),
                        no_outside_help_segments.end()
                    );

                }
            }

            void BDSPRouteBuilder::apply_starter(
                std::vector<void(*)(VideoStream&, ProControllerContext&)>& route,
                BDSPStarter starter
            ) {

                switch (starter) {

                case BDSPStarter::Chimchar:

                    route.insert(
                        route.end(),
                        chimchar_segments.begin(),
                        chimchar_segments.end()
                    );

                    break;

                case BDSPStarter::Piplup:

                    route.insert(
                        route.end(),
                        piplup_segments.begin(),
                        piplup_segments.end()
                    );

                    break;

                case BDSPStarter::Turtwig:

                    route.insert(
                        route.end(),
                        turtwig_segments.begin(),
                        turtwig_segments.end()
                    );

                    break;

                default:
                    break;
                }

            }

            void BDSPRouteBuilder::apply_legendary(
                std::vector<void(*)(VideoStream&, ProControllerContext&)>& route,
                bool enabled
            ) {

                if (enabled) {

                    route.insert(
                        route.end(),
                        legendary_segments.begin(),
                        legendary_segments.end()
                    );

                }

            }

        }
    }
}