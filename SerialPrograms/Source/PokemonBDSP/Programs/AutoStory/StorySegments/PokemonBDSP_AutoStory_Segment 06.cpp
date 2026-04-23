/*  BDSP AutoStory - Segment 06
 *  From after acquiring the fifth badge until after acquiring the sixth badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment 06.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_06::name()       const{ return "06: Fifth Badge to Sixth Badge"; }
std::string AutoStory_Segment_06::start_text() const{ return "Start: Fifth Gym Badge acquired (Veilstone City)."; }
std::string AutoStory_Segment_06::end_text()   const{ return "End: Sixth Gym Badge acquired (Canalave City)."; }

void AutoStory_Segment_06::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_06().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_06::name()       const{ return "006 - " + AutoStory_Segment_06().name(); }
std::string AutoStory_Checkpoint_06::start_text() const{ return "Fifth Gym Badge acquired (Veilstone City)."; }
std::string AutoStory_Checkpoint_06::end_text()   const{ return "Sixth Gym Badge acquired (Canalave City)."; }

void AutoStory_Checkpoint_06::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_06(env, context, options, stats);
}


void checkpoint_06(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            // TODO: implement Segment 06 gameplay logic
        }
    );
}


}
}
}
