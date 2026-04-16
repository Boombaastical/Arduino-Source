/*  BDSP AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "CommonTools/Options/StringSelectOption.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
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
#include "PokemonBDSP_AutoStory.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;


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

const std::vector<std::unique_ptr<AutoStory_Segment>>& ALL_AUTO_STORY_SEGMENT_LIST(){
    static std::vector<std::unique_ptr<AutoStory_Segment>> list = make_segment_list();
    return list;
}


// ---------------------------------------------------------------------------
//  Checkpoint list
// ---------------------------------------------------------------------------

static std::vector<std::unique_ptr<AutoStory_Checkpoint>> make_checkpoint_list(){
    std::vector<std::unique_ptr<AutoStory_Checkpoint>> list;
    list.emplace_back(std::make_unique<AutoStory_Checkpoint_00>());
    list.emplace_back(std::make_unique<AutoStory_Checkpoint_01>());
    list.emplace_back(std::make_unique<AutoStory_Checkpoint_02>());
    list.emplace_back(std::make_unique<AutoStory_Checkpoint_03>());
    list.emplace_back(std::make_unique<AutoStory_Checkpoint_04>());
    list.emplace_back(std::make_unique<AutoStory_Checkpoint_05>());
    list.emplace_back(std::make_unique<AutoStory_Checkpoint_06>());
    list.emplace_back(std::make_unique<AutoStory_Checkpoint_07>());
    list.emplace_back(std::make_unique<AutoStory_Checkpoint_08>());
    list.emplace_back(std::make_unique<AutoStory_Checkpoint_09>());
    return list;
}

const std::vector<std::unique_ptr<AutoStory_Checkpoint>>& ALL_AUTO_STORY_CHECKPOINT_LIST(){
    static std::vector<std::unique_ptr<AutoStory_Checkpoint>> list = make_checkpoint_list();
    return list;
}


// ---------------------------------------------------------------------------
//  UI databases
// ---------------------------------------------------------------------------

static StringSelectDatabase make_segments_database(){
    StringSelectDatabase db;
    int i = 0;
    for (const auto& seg : ALL_AUTO_STORY_SEGMENT_LIST()){
        db.add_entry(StringSelectEntry(std::to_string(i), seg->name()));
        i++;
    }
    return db;
}
static const StringSelectDatabase& SEGMENTS_DATABASE(){
    static StringSelectDatabase db = make_segments_database();
    return db;
}

static StringSelectDatabase make_checkpoints_database(){
    StringSelectDatabase db;
    int i = 0;
    for (const auto& cp : ALL_AUTO_STORY_CHECKPOINT_LIST()){
        db.add_entry(StringSelectEntry(std::to_string(i), cp->name()));
        i++;
    }
    return db;
}
static const StringSelectDatabase& CHECKPOINTS_DATABASE(){
    static StringSelectDatabase db = make_checkpoints_database();
    return db;
}


// ---------------------------------------------------------------------------
//  Descriptor
// ---------------------------------------------------------------------------

AutoStory_Descriptor::AutoStory_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:AutoStory",
        STRING_POKEMON + " BDSP", "Auto Story",
        "Programs/PokemonBDSP/AutoStory.html",
        "Automatically progress through the story of Brilliant Diamond and Shining Pearl.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

std::unique_ptr<StatsTracker> AutoStory_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new AutoStoryStats());
}


// ---------------------------------------------------------------------------
//  AutoStory constructor / destructor
// ---------------------------------------------------------------------------

AutoStory::~AutoStory(){
    STARTPOINT.remove_listener(*this);
    ENDPOINT.remove_listener(*this);
    START_CHECKPOINT.remove_listener(*this);
    END_CHECKPOINT.remove_listener(*this);
    ENABLE_ADVANCED_MODE.remove_listener(*this);
}

AutoStory::AutoStory()
    : STARTERCHOICE(
        "<b>Starter " + STRING_POKEMON + ":</b>",
        {
            {StarterChoice::TURTWIG,  "turtwig",  "Turtwig (Grass)"},
            {StarterChoice::CHIMCHAR, "chimchar", "Chimchar (Fire)"},
            {StarterChoice::PIPLUP,   "piplup",   "Piplup (Water)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        StarterChoice::TURTWIG
    )
    , STARTPOINT(
        "<b>Start Segment:</b>",
        SEGMENTS_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        "0"
    )
    , ENDPOINT(
        "<b>End Segment:</b>",
        SEGMENTS_DATABASE(),
        LockMode::UNLOCK_WHILE_RUNNING,
        std::to_string(ALL_AUTO_STORY_SEGMENT_LIST().size() - 1)
    )
    , START_DESCRIPTION("")
    , END_DESCRIPTION("")
    , START_CHECKPOINT(
        "<b>Start Checkpoint:</b>",
        CHECKPOINTS_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        "0"
    )
    , END_CHECKPOINT(
        "<b>End Checkpoint:</b>",
        CHECKPOINTS_DATABASE(),
        LockMode::UNLOCK_WHILE_RUNNING,
        std::to_string(ALL_AUTO_STORY_CHECKPOINT_LIST().size() - 1)
    )
    , START_CHECKPOINT_DESCRIPTION("")
    , END_CHECKPOINT_DESCRIPTION("")
    , ENABLE_ADVANCED_MODE(
        "<b>Advanced mode:</b><br>"
        "Select individual start/end checkpoints instead of segments.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(30))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(STARTERCHOICE);
    PA_ADD_OPTION(STARTPOINT);
    PA_ADD_OPTION(ENDPOINT);
    PA_ADD_OPTION(START_DESCRIPTION);
    PA_ADD_OPTION(END_DESCRIPTION);

    PA_ADD_OPTION(ENABLE_ADVANCED_MODE);
    PA_ADD_OPTION(START_CHECKPOINT);
    PA_ADD_OPTION(END_CHECKPOINT);
    PA_ADD_OPTION(START_CHECKPOINT_DESCRIPTION);
    PA_ADD_OPTION(END_CHECKPOINT_DESCRIPTION);

    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);

    AutoStory::on_config_value_changed(this);

    STARTPOINT.add_listener(*this);
    ENDPOINT.add_listener(*this);
    START_CHECKPOINT.add_listener(*this);
    END_CHECKPOINT.add_listener(*this);
    ENABLE_ADVANCED_MODE.add_listener(*this);
}


// ---------------------------------------------------------------------------
//  Config listener — update description text when dropdowns change
// ---------------------------------------------------------------------------

void AutoStory::on_config_value_changed(void* /*object*/){
    ConfigOption::LockGuard lg(STARTPOINT);

    START_DESCRIPTION.set_text(start_segment_description());
    END_DESCRIPTION.set_text(end_segment_description());
    START_CHECKPOINT_DESCRIPTION.set_text(start_checkpoint_description());
    END_CHECKPOINT_DESCRIPTION.set_text(end_checkpoint_description());

    // Show/hide checkpoint options based on mode
    bool adv = ENABLE_ADVANCED_MODE;
    START_CHECKPOINT.set_visibility(adv ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    END_CHECKPOINT.set_visibility(adv ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    START_CHECKPOINT_DESCRIPTION.set_visibility(adv ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    END_CHECKPOINT_DESCRIPTION.set_visibility(adv ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);

    STARTPOINT.set_visibility(adv ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED);
    ENDPOINT.set_visibility(adv ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED);
    START_DESCRIPTION.set_visibility(adv ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED);
    END_DESCRIPTION.set_visibility(adv ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED);
}


// ---------------------------------------------------------------------------
//  Description helpers
// ---------------------------------------------------------------------------

std::string AutoStory::start_segment_description(){
    size_t i = get_start_segment_index();
    return "    " + ALL_AUTO_STORY_SEGMENT_LIST()[i]->start_text();
}
std::string AutoStory::end_segment_description(){
    size_t i = get_end_segment_index();
    return "    " + ALL_AUTO_STORY_SEGMENT_LIST()[i]->end_text();
}
std::string AutoStory::start_checkpoint_description(){
    size_t i = get_start_checkpoint_index();
    return "    Start: " + ALL_AUTO_STORY_CHECKPOINT_LIST()[i]->start_text();
}
std::string AutoStory::end_checkpoint_description(){
    size_t i = get_end_checkpoint_index();
    return "    End: " + ALL_AUTO_STORY_CHECKPOINT_LIST()[i]->end_text();
}


// ---------------------------------------------------------------------------
//  Index helpers
// ---------------------------------------------------------------------------

size_t AutoStory::get_start_segment_index(){
    return STARTPOINT.index();
}
size_t AutoStory::get_end_segment_index(){
    return ENDPOINT.index();
}
size_t AutoStory::get_start_checkpoint_index(){
    return START_CHECKPOINT.index();
}
size_t AutoStory::get_end_checkpoint_index(){
    return END_CHECKPOINT.index();
}


// ---------------------------------------------------------------------------
//  Main run loop
// ---------------------------------------------------------------------------

void AutoStory::run_autostory(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    AutoStoryOptions options{
        Language::None,     // BDSP does not require OCR language selection yet
        STARTERCHOICE,
        NOTIFICATION_STATUS_UPDATE
    };

    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    context.wait_for_all_requests();

    if (ENABLE_ADVANCED_MODE){
        if (get_start_checkpoint_index() > get_end_checkpoint_index()){
            throw UserSetupError(env.logger(), "The start checkpoint cannot be later than the end checkpoint.");
        }
        for (size_t i = get_start_checkpoint_index(); i <= get_end_checkpoint_index(); i++){
            env.console.log("Start Checkpoint " + ALL_AUTO_STORY_CHECKPOINT_LIST()[i]->name(), COLOR_ORANGE);
            ALL_AUTO_STORY_CHECKPOINT_LIST()[i]->run_checkpoint(env, context, options, stats);
        }
    }else{
        if (get_start_segment_index() > get_end_segment_index()){
            throw UserSetupError(env.logger(), "The start segment cannot be later than the end segment.");
        }
        for (size_t i = get_start_segment_index(); i <= get_end_segment_index(); i++){
            ALL_AUTO_STORY_SEGMENT_LIST()[i]->run_segment(env, context, options, stats);
        }
    }
}


void AutoStory::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    // Connect controller
    pbf_press_button(context, BUTTON_L, 160ms, 160ms);

    run_autostory(env, context);

    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
