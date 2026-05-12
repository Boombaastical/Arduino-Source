/*  Tree Shaker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/PokemonLA_TreeShakeDetector.h"
#include "PokemonLA_TreeShaker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Pokemon;


TreeShaker_Descriptor::TreeShaker_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:TreeShaker",
        STRING_POKEMON + " LA", "Tree Shaker",
        "",
        "Visit all trees in the Obsidian Fieldlands, shake every non-shaking tree, "
        "wait 20 minutes, and repeat until all trees are shaking.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}


TreeShaker::TreeShaker()
    : SHAKE_RMSD_THRESHOLD(
        "<b>Shake Detection RMSD Threshold:</b><br>"
        "Pixel RMSD between two snapshots above this value indicates a shaking tree. "
        "Tune empirically by observing logged RMSD values for shaking vs. static trees.",
        LockMode::LOCK_WHILE_RUNNING,
        10.0, 0.0, 255.0
    )
    , SNAPSHOT_INTERVAL_MS(
        "<b>Snapshot Interval (ms):</b><br>"
        "Milliseconds between the two snapshots used for shake detection. "
        "Try values between 200 and 400 to catch the tree mid-oscillation.",
        LockMode::LOCK_WHILE_RUNNING,
        300, 50, 1000
    )
    , NUM_SAMPLES(
        "<b>Number of Samples:</b><br>"
        "How many RMSD measurements to take per run. "
        "Use this to compare a shaking tree vs. a static tree: "
        "run once on each and compare the logged RMSD values to choose a threshold.",
        LockMode::LOCK_WHILE_RUNNING,
        5, 1, 100
    )
    , DETECTION_BOX(
        "<b>Detection Box:</b><br>"
        "Screen region (x, y, width, height — all 0 to 1) used to compare frames for shake detection. "
        "Narrow this to the tree's trunk or branches to exclude moving background elements.",
        LockMode::LOCK_WHILE_RUNNING,
        0.1, 0.05, 0.8, 0.85
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(SHAKE_RMSD_THRESHOLD);
    PA_ADD_OPTION(SNAPSHOT_INTERVAL_MS);
    PA_ADD_OPTION(NUM_SAMPLES);
    PA_ADD_OPTION(DETECTION_BOX);
    PA_ADD_OPTION(NOTIFICATIONS);
}

// Tree shaking typical box: 0.200000, 0.010000, 0.500000, 0.400000


void TreeShaker::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    const ImageFloatBox detection_box(
        DETECTION_BOX.x(), DETECTION_BOX.y(),
        DETECTION_BOX.width(), DETECTION_BOX.height()
    );

    uint16_t interval_ms  = SNAPSHOT_INTERVAL_MS;
    double   threshold    = SHAKE_RMSD_THRESHOLD;
    uint16_t num_samples  = NUM_SAMPLES;

    env.log("TreeShaker tuning mode: taking " + std::to_string(num_samples) + " sample(s). "
            "Point camera at the tree and watch the RMSD values in the log.");

    for (uint16_t i = 0; i < num_samples; i++){
        bool shaking = is_tree_shaking(
            env.console.logger(), env.console.video(), context,
            detection_box, interval_ms, threshold
        );
        env.log("Sample " + std::to_string(i + 1) + "/" + std::to_string(num_samples)
                + ": tree is " + (shaking ? "SHAKING" : "static"));

        if (i + 1 < num_samples){
            pbf_wait(context, 1000ms);
            context.wait_for_all_requests();
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
