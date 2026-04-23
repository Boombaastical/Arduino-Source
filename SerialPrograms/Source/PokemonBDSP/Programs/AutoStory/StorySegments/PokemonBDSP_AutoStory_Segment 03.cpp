/*  BDSP AutoStory - Segment 03
 *  From after acquiring the second badge until after acquiring the third badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment 03.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_03::name()       const{ return "03: Second Badge to Third Badge"; }
std::string AutoStory_Segment_03::start_text() const{ return "Start: Second Gym Badge acquired (Eterna City)."; }
std::string AutoStory_Segment_03::end_text()   const{ return "End: Third Gym Badge acquired (Hearthome City)."; }

void AutoStory_Segment_03::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_03().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_03::name()       const{ return "003 - " + AutoStory_Segment_03().name(); }
std::string AutoStory_Checkpoint_03::start_text() const{ return "Second Gym Badge acquired (Eterna City)."; }
std::string AutoStory_Checkpoint_03::end_text()   const{ return "Third Gym Badge acquired (Hearthome City)."; }

void AutoStory_Checkpoint_03::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_03(env, context, options, stats);
}


void checkpoint_03(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            // TODO: implement Segment 03 gameplay logic
        }
    );
}


}
}
}
