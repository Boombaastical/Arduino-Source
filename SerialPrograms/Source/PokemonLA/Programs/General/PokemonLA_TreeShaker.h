/*  Tree Shaker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_TreeShaker_H
#define PokemonAutomation_PokemonLA_TreeShaker_H

#include "Common/Cpp/Options/BoxFloatOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class TreeShaker_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TreeShaker_Descriptor();
};


class TreeShaker : public SingleSwitchProgramInstance{
public:
    TreeShaker();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    FloatingPointOption SHAKE_RMSD_THRESHOLD;
    SimpleIntegerOption<uint16_t> SNAPSHOT_INTERVAL_MS;
    SimpleIntegerOption<uint16_t> NUM_SAMPLES;
    BoxFloatOption DETECTION_BOX;

    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
