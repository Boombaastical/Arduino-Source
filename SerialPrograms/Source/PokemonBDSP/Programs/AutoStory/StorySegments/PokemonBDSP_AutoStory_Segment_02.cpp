/*  BDSP AutoStory - Segment 02
 *  From after acquiring the first badge until after acquiring the second badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment_02.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_02::name()       const{ return "02: First Badge to Second Badge"; }
std::string AutoStory_Segment_02::start_text() const{ return "Start: First Gym Badge acquired (Oreburgh City)."; }
std::string AutoStory_Segment_02::end_text()   const{ return "End: Second Gym Badge acquired (Eterna City)."; }

void AutoStory_Segment_02::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_02().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_02::name()       const{ return "002 - " + AutoStory_Segment_02().name(); }
std::string AutoStory_Checkpoint_02::start_text() const{ return "First Gym Badge acquired (Oreburgh City)."; }
std::string AutoStory_Checkpoint_02::end_text()   const{ return "Second Gym Badge acquired (Eterna City)."; }

void AutoStory_Checkpoint_02::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_02(env, context, options, stats);
}


void checkpoint_02(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            // TODO: implement Segment 02 gameplay logic
        }
    );
}


}
}
}
