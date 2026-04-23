/*  BDSP AutoStory - Segment 05
 *  From after acquiring the fourth badge until after acquiring the fifth badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment 05.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_05::name()       const{ return "05: Fourth Badge to Fifth Badge"; }
std::string AutoStory_Segment_05::start_text() const{ return "Start: Fourth Gym Badge acquired (Pastoria City)."; }
std::string AutoStory_Segment_05::end_text()   const{ return "End: Fifth Gym Badge acquired (Veilstone City)."; }

void AutoStory_Segment_05::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_05().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_05::name()       const{ return "005 - " + AutoStory_Segment_05().name(); }
std::string AutoStory_Checkpoint_05::start_text() const{ return "Fourth Gym Badge acquired (Pastoria City)."; }
std::string AutoStory_Checkpoint_05::end_text()   const{ return "Fifth Gym Badge acquired (Veilstone City)."; }

void AutoStory_Checkpoint_05::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_05(env, context, options, stats);
}


void checkpoint_05(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            // TODO: implement Segment 05 gameplay logic
        }
    );
}


}
}
}
