/*  BDSP AutoStory - Battle Utilities
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_ExperienceGainDetector.h"
#include "PokemonBDSP_AutoStory_Battle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


void run_battle_press_A(
    const ProgramInfo& /*info*/,
    VideoStream& stream,
    ProControllerContext& context
){
    // TODO: implement
    //  1. Wait for BattleMenuWatcher (STANDARD type)
    //  2. Mash A to select first move, repeat until EndBattleWatcher fires
    //  3. Mash A through experience gain (ExperienceGainWatcher → ShortDialogWatcher)
    //  4. Return when overworld is detected (OverworldDetector from Detect/)
    (void)stream; (void)context;
}

void handle_unexpected_battle(
    const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context,
    std::function<void(const ProgramInfo&, VideoStream&, ProControllerContext&)>&& action,
    std::function<void(const ProgramInfo&, VideoStream&, ProControllerContext&)>&& recovery_action,
    size_t max_attempts
){
    // TODO: implement
    //  - Run action inside run_until<> with StartBattleDetector as the stop condition
    //  - If StartBattleDetector fires: call run_battle_press_A, then recovery_action
    //  - Retry action; throw if max_attempts exceeded
    (void)info; (void)stream; (void)context;
    (void)action; (void)recovery_action; (void)max_attempts;
}


}
}
}
