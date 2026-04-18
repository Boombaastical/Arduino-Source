#include "PokemonBDSP_AutoStory.h"

#include "BDSP_RouteBuilder.h"

#include "CommonFramework/Globals.h"

#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace std::chrono_literals;


/*
 * Descriptor
 */

BDSPAutoStory_Descriptor::BDSPAutoStory_Descriptor()
    : SingleSwitchProgramDescriptor(
          "PokemonBDSP:AutoStory",
          "Pokemon BDSP",
          "Auto Story",
          "Programs/PokemonBDSP/AutoStory.html",
          "Automatically completes BDSP story segments.",
          ProgramControllerClass::StandardController_NoRestrictions,
          FeedbackType::REQUIRED,
          AllowCommandsWhenRunning::DISABLE_COMMANDS
          )
{}


std::unique_ptr<SingleSwitchProgramInstance>
BDSPAutoStory_Descriptor::make_instance() const{
    return std::make_unique<BDSPAutoStory>();
}


/*
 * Program Instance
 */

BDSPAutoStory::BDSPAutoStory()
    : SingleSwitchProgramInstance()

    , ROUTE_TYPE(
          "<b>Route Type:</b>",
          {
           {BDSPRouteType::OutsideHelp, "outside_help", "Outside Help"},
           {BDSPRouteType::NoOutsideHelp, "no_outside_help", "No Outside Help"},
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
           {BDSPStarter::Piplup, "piplup", "Piplup"},
           {BDSPStarter::Turtwig, "turtwig", "Turtwig"},
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


/*
 * Program Logic
 */

void BDSPAutoStory::program(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
    ){

    BDSPRouteConfig config;

    // Fixed date: June 9, 18:30
    config.set_datetime = DateTime{2025, 6, 9, 18, 30, 0};

    config.route_type = ROUTE_TYPE;

    config.starter_selected = STARTER_ENABLED;
    config.starter = STARTER_SELECT;
    config.starter_shiny = STARTER_SHINY;

    config.catch_box_legendary = CATCH_LEGENDARY;


    DateTime target_datetime = config.set_datetime;

    // TROUBLESHOOTING: Temporarily disabled date setting to speed up route testing
    // Uncomment the entire if block below when date setting is needed
    /*
    if (target_datetime.year > 0){

        env.console.log("Setting system date and time to June 9, 18:30...");

        home_to_date_time(env.console, context, true);

        // Press A to enter the date change screen
        pbf_press_button(context, BUTTON_A, 80ms, 240ms);
        context.wait_for_all_requests();

        DateReader reader(env.console);

        reader.set_date(
            env.program_info(),
            env.console,
            context,
            target_datetime
            );

        // Confirm the date change
        pbf_press_button(context, BUTTON_A, 160ms, 340ms);
        context.wait_for_all_requests();

        env.console.log("Date set successfully. Returning to game...");

        // Return to the game from home
        resume_game_from_home(env.console, context);
        context.wait_for_all_requests();

        // Wait for the game to fully load - player should be in their room
        env.console.log("Waiting for game to fully load...");
        pbf_wait(context, 5000ms);
        context.wait_for_all_requests();

    }
    */

    env.console.log("Beginning route execution from player's room...");
    env.console.log("Note: Player should be standing in their bedroom ready to exit.");

    auto route = BDSPRouteBuilder::build_route(config);

    for (auto segment : route){
        segment(env.console, context);
    }

}


}
}
}