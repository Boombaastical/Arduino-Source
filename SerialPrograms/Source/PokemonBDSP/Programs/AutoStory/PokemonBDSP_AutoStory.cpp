#include <algorithm>
#include <memory>

#include "PokemonBDSP_AutoStory.h"

#include "CommonFramework/Globals.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"

#include "Story segments/PokemonBDSP_AutoStory_Segment 00.h"
#include "Story segments/PokemonBDSP_AutoStory_Segment 01.h"
#include "Story segments/PokemonBDSP_AutoStory_Segment 02.h"
#include "Story segments/PokemonBDSP_AutoStory_Segment 03.h"
#include "Story segments/PokemonBDSP_AutoStory_Segment 04.h"
#include "Story segments/PokemonBDSP_AutoStory_Segment 05.h"
#include "Story segments/PokemonBDSP_AutoStory_Segment 06.h"
#include "Story segments/PokemonBDSP_AutoStory_Segment 07.h"
#include "Story segments/PokemonBDSP_AutoStory_Segment 08.h"
#include "Story segments/PokemonBDSP_AutoStory_Segment 09.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace std::chrono_literals;


// ---------------------------------------------------------------------------
//  Segment list
// ---------------------------------------------------------------------------

static std::vector<std::unique_ptr<AutoStory_Segment>> make_segment_list(){
    std::vector<std::unique_ptr<AutoStory_Segment>> list;
    list.emplace_back(std::make_unique<AutoStory_Segment_00>());
    list.emplace_back(std::make_unique<AutoStory_Segment_01>());
    list.emplace_back(std::make_unique<AutoStory_Segment_02>());
    list.emplace_back(std::make_unique<AutoStory_Segment_03>());
    list.emplace_back(std::make_unique<AutoStory_Segment_04>());
    list.emplace_back(std::make_unique<AutoStory_Segment_05>());
    list.emplace_back(std::make_unique<AutoStory_Segment_06>());
    list.emplace_back(std::make_unique<AutoStory_Segment_07>());
    list.emplace_back(std::make_unique<AutoStory_Segment_08>());
    list.emplace_back(std::make_unique<AutoStory_Segment_09>());
    return list;
}

static const std::vector<std::unique_ptr<AutoStory_Segment>>& ALL_SEGMENTS(){
    static std::vector<std::unique_ptr<AutoStory_Segment>> list = make_segment_list();
    return list;
}


// ---------------------------------------------------------------------------
//  Descriptor
// ---------------------------------------------------------------------------

BDSPAutoStory_Descriptor::BDSPAutoStory_Descriptor()
    : SingleSwitchProgramDescriptor(
          "PokemonBDSP:AutoStory",
          "Pokemon BDSP",
          "Auto Story",
          "Programs/PokemonBDSP/AutoStory.html",
          "Automatically completes BDSP story segments.",
          ProgramControllerClass::StandardController_NoRestrictions,
          FeedbackType::REQUIRED,
          AllowCommandsWhenRunning::DISABLE_COMMANDS
          )
{}


std::unique_ptr<SingleSwitchProgramInstance>
BDSPAutoStory_Descriptor::make_instance() const{
    return std::make_unique<BDSPAutoStory>();
}

std::unique_ptr<StatsTracker>
BDSPAutoStory_Descriptor::make_stats() const{
    return std::make_unique<AutoStoryStats>();
}


// ---------------------------------------------------------------------------
//  Program Instance
// ---------------------------------------------------------------------------

BDSPAutoStory::~BDSPAutoStory(){
    for (auto& cb : SEGMENT_CHECKBOXES){
        cb->remove_listener(*this);
    }
}

BDSPAutoStory::BDSPAutoStory()
    : SingleSwitchProgramInstance()

    , STARTERCHOICE(
          "<b>Starter Pokemon:</b>",
          {
           {StarterChoice::TURTWIG,  "turtwig",  "Turtwig"},
           {StarterChoice::CHIMCHAR, "chimchar", "Chimchar"},
           {StarterChoice::PIPLUP,   "piplup",   "Piplup"},
           },
          LockMode::LOCK_WHILE_RUNNING,
          StarterChoice::CHIMCHAR
          )

    , STARTER_SHINY(
          "<b>Shiny Starter:</b>",
          LockMode::LOCK_WHILE_RUNNING,
          false
          )

    , CATCH_LEGENDARY(
          "<b>Catch Box Legendary:</b>",
          LockMode::LOCK_WHILE_RUNNING,
          false
          )

    , HELP_MODE(
          "<b>Help Mode:</b>",
          {
              {HelpMode::NO_OUTSIDE_HELP, "no_outside_help", "No Outside Help"},
              {HelpMode::OUTSIDE_HELP,    "outside_help",    "Outside Help"},
          },
          LockMode::LOCK_WHILE_RUNNING,
          HelpMode::NO_OUTSIDE_HELP
          )

    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(30))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })

{
    // Build one checkbox per segment, all checked by default
    const auto& segments = ALL_SEGMENTS();
    for (size_t i = 0; i < segments.size(); i++){
        std::string num = std::to_string(i);
        if (num.size() < 2) num = "0" + num;
        SEGMENT_CHECKBOXES.emplace_back(std::make_unique<BooleanCheckBoxOption>(
            "<b>Segment " + num + ":</b> " + segments[i]->name(),
            LockMode::LOCK_WHILE_RUNNING,
            true
        ));
        add_option(*SEGMENT_CHECKBOXES[i], "RUN_SEGMENT_" + num);
        SEGMENT_CHECKBOXES[i]->add_listener(*this);
    }

    PA_ADD_OPTION(STARTERCHOICE);
    PA_ADD_OPTION(STARTER_SHINY);
    PA_ADD_OPTION(CATCH_LEGENDARY);
    PA_ADD_OPTION(HELP_MODE);
    PA_ADD_OPTION(NOTIFICATIONS);

    BDSPAutoStory::on_config_value_changed(this);
}


// ---------------------------------------------------------------------------
//  Range-select checkbox logic
// ---------------------------------------------------------------------------

void BDSPAutoStory::on_config_value_changed(void* object){
    if (m_updating) return;
    m_updating = true;

    // Find which checkbox fired
    size_t changed = SIZE_MAX;
    for (size_t i = 0; i < SEGMENT_CHECKBOXES.size(); i++){
        if (SEGMENT_CHECKBOXES[i].get() == object){
            changed = i;
            break;
        }
    }

    if (changed != SIZE_MAX){
        bool new_val = (bool)*SEGMENT_CHECKBOXES[changed];

        // Find current range [range_start, range_end]
        size_t range_start = SIZE_MAX, range_end = SIZE_MAX;
        for (size_t i = 0; i < SEGMENT_CHECKBOXES.size(); i++){
            if ((bool)*SEGMENT_CHECKBOXES[i]){
                if (range_start == SIZE_MAX) range_start = i;
                range_end = i;
            }
        }

        if (new_val){
            // Extend range to include `changed`, filling any gap
            if (range_start != SIZE_MAX){
                size_t new_start = std::min(range_start, changed);
                size_t new_end   = std::max(range_end,   changed);
                for (size_t i = new_start; i <= new_end; i++)
                    *SEGMENT_CHECKBOXES[i] = true;
            }
        } else {
            if (changed == range_start){
                // Uncheck all
                for (auto& cb : SEGMENT_CHECKBOXES) *cb = false;
            } else if (range_start != SIZE_MAX && changed <= range_end){
                // Truncate: uncheck changed..range_end
                for (size_t i = changed; i <= range_end; i++)
                    *SEGMENT_CHECKBOXES[i] = false;
            }
        }
    }

    // Show STARTERCHOICE/STARTER_SHINY only when segment 00 is in the range
    bool show_starter = !SEGMENT_CHECKBOXES.empty() && (bool)*SEGMENT_CHECKBOXES[0];
    ConfigOptionState starter_state = show_starter ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN;
    STARTERCHOICE.set_visibility(starter_state);
    STARTER_SHINY.set_visibility(starter_state);

    m_updating = false;
}


// ---------------------------------------------------------------------------
//  Program logic
// ---------------------------------------------------------------------------

void BDSPAutoStory::program(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
    ){

    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();

    AutoStoryOptions options{
        Language::English,
        STARTERCHOICE,
        NOTIFICATION_STATUS_UPDATE
    };

    const auto& segments = ALL_SEGMENTS();
    for (size_t i = 0; i < segments.size(); i++){
        if ((bool)*SEGMENT_CHECKBOXES[i]){
            segments[i]->run_segment(env, context, options, stats);
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
