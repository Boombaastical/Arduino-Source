/*  Tree Shake Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA_TreeShakeDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


bool is_tree_shaking(
    Logger& logger,
    VideoFeed& feed,
    ProControllerContext& context,
    const ImageFloatBox& box,
    uint16_t snapshot_interval_ms,
    double rmsd_threshold
){
    VideoSnapshot frame1 = feed.snapshot();
    pbf_wait(context, Milliseconds(snapshot_interval_ms));
    context.wait_for_all_requests();
    VideoSnapshot frame2 = feed.snapshot();

    double rmsd = ImageMatch::pixel_RMSD(
        extract_box_reference(frame1, box),
        extract_box_reference(frame2, box)
    );
    logger.log("Tree shake RMSD: " + std::to_string(rmsd));
    return rmsd > rmsd_threshold;
}


}
}
}
