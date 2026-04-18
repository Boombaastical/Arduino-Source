/*  BDSP AutoStory Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonBDSP_AutoStory_Options.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
namespace BDSPAutoStoryInternal{


RouteOptions::RouteOptions()
    : GroupOption("Route Options", LockMode::LOCK_WHILE_RUNNING)
    , ROUTE_TYPE(
          "<b>Route Type:</b>",
          {
              {BDSPRouteType::OutsideHelp,    "outside_help",    "Outside Help"},
              {BDSPRouteType::NoOutsideHelp,  "no_outside_help", "No Outside Help"},
          },
          LockMode::LOCK_WHILE_RUNNING,
          BDSPRouteType::NoOutsideHelp
          )
    , STARTER_ENABLED(
          "<b>Enable Starter Selection:</b>",
          LockMode::LOCK_WHILE_RUNNING,
          false
          )
    , STARTER_SELECT(
          "<b>Starter:</b>",
          {
              {BDSPStarter::Chimchar, "chimchar", "Chimchar"},
              {BDSPStarter::Piplup,   "piplup",   "Piplup"},
              {BDSPStarter::Turtwig,  "turtwig",  "Turtwig"},
          },
          LockMode::LOCK_WHILE_RUNNING,
          BDSPStarter::Chimchar
          )
    , STARTER_SHINY(
          "<b>Shiny Starter:</b>",
          LockMode::LOCK_WHILE_RUNNING,
          false
          )
    , CATCH_LEGENDARY(
          "<b>Catch Box Legendary:</b>",
          LockMode::LOCK_WHILE_RUNNING,
          false
          )
{
    PA_ADD_OPTION(ROUTE_TYPE);
    PA_ADD_OPTION(STARTER_ENABLED);
    PA_ADD_OPTION(STARTER_SELECT);
    PA_ADD_OPTION(STARTER_SHINY);
    PA_ADD_OPTION(CATCH_LEGENDARY);
}


}
}
}
}
