/*  Overworld Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonLA_OverworldDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



OverworldDetector::OverworldDetector(Logger& logger, VideoOverlay& overlay)
    : VisualInferenceCallback("OverworldDetector")
    , m_arc_phone(logger, overlay, std::chrono::milliseconds(100), true)
{}

void OverworldDetector::make_overlays(VideoOverlaySet& items) const{
    m_arc_phone.make_overlays(items);
    m_mount.make_overlays(items);
}
bool OverworldDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    m_arc_phone.process_frame(frame, timestamp);
    if (!m_arc_phone.detected()){
        return false;
    }
    return m_mount.detect(frame) != MountState::NOTHING;
}




bool is_pokemon_selection(VideoOverlay& overlay, const ImageViewRGB32& frame){
    static const ImageFloatBox box0(0.850000, 0.878000, 0.006000, 0.021000);
    static const ImageFloatBox box1(0.905000, 0.878000, 0.006000, 0.021000);

    OverlayBoxScope scope0(overlay, box0);
    OverlayBoxScope scope1(overlay, box1);

    ImageStats stats0 = image_stats(extract_box_reference(frame, box0));
    ImageStats stats1 = image_stats(extract_box_reference(frame, box1));

    cout << "box0: avg=" << stats0.average << " stddev=" << stats0.stddev << endl;
    cout << "box1: avg=" << stats1.average << " stddev=" << stats1.stddev << endl;

    // Pokemon menu: semi-transparent overlay, avg sum ~258, stddev ~170 (background shows through).
    // Items menu: solid yellow/orange overlay, avg sum ~554, stddev ~20.
    // is_grey checks: min_sum < avg_sum < max_sum AND stddev < max_stddev.
    return is_grey(stats0, 50, 380, 200) && is_grey(stats1, 50, 380, 200);
}




}
}
}
