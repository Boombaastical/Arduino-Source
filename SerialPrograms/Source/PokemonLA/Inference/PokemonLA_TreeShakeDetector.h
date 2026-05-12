/*  Tree Shake Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Detects whether a tree is shaking by comparing pixel RMSD between two
 *  consecutive snapshots of the tree's screen region taken ~300 ms apart.
 *  A high RMSD indicates movement (tree is shaking); a low RMSD means static.
 */

#ifndef PokemonAutomation_PokemonLA_TreeShakeDetector_H
#define PokemonAutomation_PokemonLA_TreeShakeDetector_H

#include <cstdint>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


// Returns true if the tree region on screen is moving (shaking).
// Takes two snapshots `snapshot_interval_ms` ms apart and compares pixel RMSD
// over the given `box`. RMSD above `rmsd_threshold` means the tree is shaking.
bool is_tree_shaking(
    Logger& logger,
    VideoFeed& feed,
    ProControllerContext& context,
    const ImageFloatBox& box,
    uint16_t snapshot_interval_ms,
    double rmsd_threshold
);


}
}
}
#endif
