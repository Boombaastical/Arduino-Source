/*  BDSP AutoStory Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStory_Options_H
#define PokemonAutomation_PokemonBDSP_AutoStory_Options_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "../PokemonBDSP_RouteConfig.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
namespace BDSPAutoStoryInternal{


class RouteOptions : public GroupOption{
public:
    RouteOptions();

    EnumDropdownOption<BDSPRouteType> ROUTE_TYPE;
    BooleanCheckBoxOption STARTER_ENABLED;
    EnumDropdownOption<BDSPStarter> STARTER_SELECT;
    BooleanCheckBoxOption STARTER_SHINY;
    BooleanCheckBoxOption CATCH_LEGENDARY;
};


}
}
}
}
#endif
