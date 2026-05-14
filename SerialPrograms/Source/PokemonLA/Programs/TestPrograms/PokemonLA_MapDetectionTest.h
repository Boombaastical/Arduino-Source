/*  Map Detection Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_MapDetectionTest_H
#define PokemonAutomation_PokemonLA_MapDetectionTest_H

#include <atomic>
#include "Common/Cpp/Options/ButtonOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/BoxOption.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


enum class ActiveRegion{ CURSOR = 0, PIN = 1, OPEN = 2 };


class MapDetectionTest_Descriptor : public SingleSwitchProgramDescriptor{
public:
    MapDetectionTest_Descriptor();
};


class MapDetectionTest : public SingleSwitchProgramInstance{
public:
    MapDetectionTest();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    class RegionDrawer;  // defined in .cpp, instantiated locally in program()

    class SnapshotButton : public ButtonOption{
    public:
        SnapshotButton();
        void set_source(VideoFeed* feed, BoxOption* cursor, BoxOption* pin, BoxOption* open);
        virtual void press_button() override;
    private:
        VideoFeed* m_feed       = nullptr;
        BoxOption* m_cursor_box = nullptr;
        BoxOption* m_pin_box    = nullptr;
        BoxOption* m_open_box   = nullptr;
    };

    EnumDropdownOption<ActiveRegion> ACTIVE_REGION;
    BoxOption CURSOR_BOX;
    BoxOption PIN_BOX;
    BoxOption OPEN_BOX;

    // Detection thresholds — all unlock while running for live tuning
    FloatingPointOption         PIN_RMSD_THRESHOLD;
    FloatingPointOption         PIN_MIN_AREA;
    FloatingPointOption         DIALOG_RMSD_THRESHOLD;
    FloatingPointOption         DIALOG_MIN_AREA;
    FloatingPointOption         OPEN_RMSD_THRESHOLD;
    SimpleIntegerOption<uint16_t> LOG_INTERVAL_MS;

    SnapshotButton SAVE_BUTTON;
};


}
}
}
#endif
