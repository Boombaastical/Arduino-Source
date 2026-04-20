#pragma once

#include <memory>
#include <vector>
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonBDSP_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

class BDSPAutoStory_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BDSPAutoStory_Descriptor();

    virtual std::unique_ptr<SingleSwitchProgramInstance> make_instance() const override;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class BDSPAutoStory : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    ~BDSPAutoStory();
    BDSPAutoStory();

    virtual void program(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context
        ) override;

private:
    virtual void on_config_value_changed(void* object) override;

private:
    std::vector<std::unique_ptr<BooleanCheckBoxOption>> SEGMENT_CHECKBOXES;
    bool m_updating = false;

    EnumDropdownOption<StarterChoice>      STARTERCHOICE;
    BooleanCheckBoxOption                  STARTER_SHINY;
    BooleanCheckBoxOption                  CATCH_LEGENDARY;
    EnumDropdownOption<HelpMode>           HELP_MODE;
    EnumDropdownOption<OutsideHelpPokemon> OUTSIDE_HELP_POKEMON;
    EnumDropdownOption<TorterraMove>       OUTSIDE_HELP_ATTACK_1;
    EnumDropdownOption<TorterraMove>       OUTSIDE_HELP_ATTACK_2;
    EnumDropdownOption<TorterraMove>       OUTSIDE_HELP_ATTACK_3;
    EnumDropdownOption<TorterraMove>       OUTSIDE_HELP_ATTACK_4;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
