/*  BDSP AutoStory - Segment 08
 *  From after acquiring the seventh badge until after acquiring the eighth badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment 08.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_08::name()       const{ return "08: Seventh Badge to Eighth Badge"; }
std::string AutoStory_Segment_08::start_text() const{ return "Start: Seventh Gym Badge acquired (Snowpoint City)."; }
std::string AutoStory_Segment_08::end_text()   const{ return "End: Eighth Gym Badge acquired (Sunyshore City)."; }

void AutoStory_Segment_08::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_08().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_08::name()       const{ return "008 - " + AutoStory_Segment_08().name(); }
std::string AutoStory_Checkpoint_08::start_text() const{ return "Seventh Gym Badge acquired (Snowpoint City)."; }
std::string AutoStory_Checkpoint_08::end_text()   const{ return "Eighth Gym Badge acquired (Sunyshore City)."; }

void AutoStory_Checkpoint_08::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_08(env, context, options, stats);
}


void checkpoint_08(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            // TODO: implement Segment 08 gameplay logic
        }
    );
}


}
}
}
