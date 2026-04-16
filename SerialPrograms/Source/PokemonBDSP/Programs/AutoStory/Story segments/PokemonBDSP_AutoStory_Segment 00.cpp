/*  BDSP AutoStory - Segment 00
 *  From beginning cinematic until after choosing the starter Pokemon.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment 00.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_00::name()       const{ return "00: Intro Cinematic to Starter Choice"; }
std::string AutoStory_Segment_00::start_text() const{ return "Start: Game launched, beginning cinematic playing."; }
std::string AutoStory_Segment_00::end_text()   const{ return "End: Starter Pokemon chosen and received."; }

void AutoStory_Segment_00::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_00().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_00::name()       const{ return "000 - " + AutoStory_Segment_00().name(); }
std::string AutoStory_Checkpoint_00::start_text() const{ return "Game launched, beginning cinematic playing."; }
std::string AutoStory_Checkpoint_00::end_text()   const{ return "Starter Pokemon chosen and received."; }

void AutoStory_Checkpoint_00::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_00(env, context, options, stats);
}


void checkpoint_00(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            // TODO: implement Segment 00 gameplay logic
        }
    );
}


}
}
}
