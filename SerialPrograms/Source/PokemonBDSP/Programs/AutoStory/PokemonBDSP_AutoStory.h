/*  BDSP AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStory_H
#define PokemonAutomation_PokemonBDSP_AutoStory_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/StringSelectOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonBDSP_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class AutoStory_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoStory_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class AutoStory : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    ~AutoStory();
    AutoStory();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    virtual void on_config_value_changed(void* object) override;

    std::string start_segment_description();
    std::string end_segment_description();
    std::string start_checkpoint_description();
    std::string end_checkpoint_description();

    size_t get_start_segment_index();
    size_t get_end_segment_index();
    size_t get_start_checkpoint_index();
    size_t get_end_checkpoint_index();

    void run_autostory(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    EnumDropdownOption<StarterChoice> STARTERCHOICE;

    StringSelectOption STARTPOINT;
    StringSelectOption ENDPOINT;

    StaticTextOption START_DESCRIPTION;
    StaticTextOption END_DESCRIPTION;

    StringSelectOption START_CHECKPOINT;
    StringSelectOption END_CHECKPOINT;

    StaticTextOption START_CHECKPOINT_DESCRIPTION;
    StaticTextOption END_CHECKPOINT_DESCRIPTION;

    BooleanCheckBoxOption ENABLE_ADVANCED_MODE;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};


// Global singletons — defined in .cpp
const std::vector<std::unique_ptr<AutoStory_Segment>>&    ALL_AUTO_STORY_SEGMENT_LIST();
const std::vector<std::unique_ptr<AutoStory_Checkpoint>>& ALL_AUTO_STORY_CHECKPOINT_LIST();


}
}
}
#endif
