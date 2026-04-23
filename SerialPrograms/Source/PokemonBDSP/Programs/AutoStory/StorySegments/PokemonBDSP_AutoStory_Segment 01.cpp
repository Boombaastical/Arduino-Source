/*  BDSP AutoStory - Segment 01
 *  From after choosing the starter Pokemon until after acquiring the first badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment 01.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_01::name()       const{ return "01: Starter Received to First Badge"; }
std::string AutoStory_Segment_01::start_text() const{ return "Start: Starter Pokemon chosen and received."; }
std::string AutoStory_Segment_01::end_text()   const{ return "End: First Gym Badge acquired (Oreburgh City)."; }

void AutoStory_Segment_01::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_01().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_01::name()       const{ return "001 - " + AutoStory_Segment_01().name(); }
std::string AutoStory_Checkpoint_01::start_text() const{ return "Starter Pokemon chosen and received."; }
std::string AutoStory_Checkpoint_01::end_text()   const{ return "First Gym Badge acquired (Oreburgh City)."; }

void AutoStory_Checkpoint_01::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_01(env, context, options, stats);
}


void checkpoint_01(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            // TODO: implement Segment 01 gameplay logic
        }
    );
}


}
}
}
