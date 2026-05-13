/*  Tree Shaker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_TreeShaker_H
#define PokemonAutomation_PokemonLA_TreeShaker_H

#include "Common/Cpp/Options/BoxFloatOption.h"
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
    SimpleIntegerOption<uint16_t> MAX_THROW_ATTEMPTS;
    BoxFloatOption ITEM_NOTIFICATION_BOX;

    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
