#include <algorithm>
#include <memory>

#include "PokemonBDSP_AutoStory.h"

#include "CommonFramework/Globals.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"

#include "StorySegments/PokemonBDSP_AutoStory_Segment_00.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_01.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_02.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_03.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_04.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_05.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_06.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_07.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_08.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_09.h"

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
    OUTSIDE_HELP_POKEMON.remove_listener(*this);
    HELP_MODE.remove_listener(*this);
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

    , OUTSIDE_HELP_POKEMON(
          "<b>Pokemon:</b>",
          {
              {OutsideHelpPokemon::TORTERRA, "torterra", "Torterra"},
          },
          LockMode::LOCK_WHILE_RUNNING,
          OutsideHelpPokemon::TORTERRA
          )

    , OUTSIDE_HELP_ATTACK_1(
          "<b>Attack 1:</b>",
          {
              {TorterraMove::EARTHQUAKE,  "earthquake",  "Earthquake"},
              {TorterraMove::WOOD_HAMMER, "wood_hammer", "Wood Hammer"},
              {TorterraMove::RAZOR_LEAF,  "razor_leaf",  "Razor Leaf"},
              {TorterraMove::CRUNCH,      "crunch",      "Crunch"},
              {TorterraMove::GIGA_DRAIN,  "giga_drain",  "Giga Drain"},
              {TorterraMove::IRON_TAIL,   "iron_tail",   "Iron Tail"},
              {TorterraMove::ROCK_TOMB,   "rock_tomb",   "Rock Tomb"},
              {TorterraMove::FACADE,      "facade",      "Facade"},
              {TorterraMove::STONE_EDGE,  "stone_edge",  "Stone Edge"},
              {TorterraMove::ROCK_SLIDE,  "rock_slide",  "Rock Slide"},
              {TorterraMove::BULLDOZE,    "bulldoze",    "Bulldoze"},
              {TorterraMove::STRENGTH,    "strength",    "Strength"},
              {TorterraMove::ROCK_SMASH,  "rock_smash",  "Rock Smash"},
              {TorterraMove::ROCK_CLIMB,  "rock_climb",  "Rock Climb"},
          },
          LockMode::LOCK_WHILE_RUNNING,
          TorterraMove::EARTHQUAKE
          )

    , OUTSIDE_HELP_ATTACK_2(
          "<b>Attack 2:</b>",
          {
              {TorterraMove::EARTHQUAKE,  "earthquake",  "Earthquake"},
              {TorterraMove::WOOD_HAMMER, "wood_hammer", "Wood Hammer"},
              {TorterraMove::RAZOR_LEAF,  "razor_leaf",  "Razor Leaf"},
              {TorterraMove::CRUNCH,      "crunch",      "Crunch"},
              {TorterraMove::GIGA_DRAIN,  "giga_drain",  "Giga Drain"},
              {TorterraMove::IRON_TAIL,   "iron_tail",   "Iron Tail"},
              {TorterraMove::ROCK_TOMB,   "rock_tomb",   "Rock Tomb"},
              {TorterraMove::FACADE,      "facade",      "Facade"},
              {TorterraMove::STONE_EDGE,  "stone_edge",  "Stone Edge"},
              {TorterraMove::ROCK_SLIDE,  "rock_slide",  "Rock Slide"},
              {TorterraMove::BULLDOZE,    "bulldoze",    "Bulldoze"},
              {TorterraMove::STRENGTH,    "strength",    "Strength"},
              {TorterraMove::ROCK_SMASH,  "rock_smash",  "Rock Smash"},
              {TorterraMove::ROCK_CLIMB,  "rock_climb",  "Rock Climb"},
          },
          LockMode::LOCK_WHILE_RUNNING,
          TorterraMove::ROCK_SLIDE
          )

    , OUTSIDE_HELP_ATTACK_3(
          "<b>Attack 3:</b>",
          {
              {TorterraMove::EARTHQUAKE,  "earthquake",  "Earthquake"},
              {TorterraMove::WOOD_HAMMER, "wood_hammer", "Wood Hammer"},
              {TorterraMove::RAZOR_LEAF,  "razor_leaf",  "Razor Leaf"},
              {TorterraMove::CRUNCH,      "crunch",      "Crunch"},
              {TorterraMove::GIGA_DRAIN,  "giga_drain",  "Giga Drain"},
              {TorterraMove::IRON_TAIL,   "iron_tail",   "Iron Tail"},
              {TorterraMove::ROCK_TOMB,   "rock_tomb",   "Rock Tomb"},
              {TorterraMove::FACADE,      "facade",      "Facade"},
              {TorterraMove::STONE_EDGE,  "stone_edge",  "Stone Edge"},
              {TorterraMove::ROCK_SLIDE,  "rock_slide",  "Rock Slide"},
              {TorterraMove::BULLDOZE,    "bulldoze",    "Bulldoze"},
              {TorterraMove::STRENGTH,    "strength",    "Strength"},
              {TorterraMove::ROCK_SMASH,  "rock_smash",  "Rock Smash"},
              {TorterraMove::ROCK_CLIMB,  "rock_climb",  "Rock Climb"},
          },
          LockMode::LOCK_WHILE_RUNNING,
          TorterraMove::WOOD_HAMMER
          )

    , OUTSIDE_HELP_ATTACK_4(
          "<b>Attack 4:</b>",
          {
              {TorterraMove::EARTHQUAKE,  "earthquake",  "Earthquake"},
              {TorterraMove::WOOD_HAMMER, "wood_hammer", "Wood Hammer"},
              {TorterraMove::RAZOR_LEAF,  "razor_leaf",  "Razor Leaf"},
              {TorterraMove::CRUNCH,      "crunch",      "Crunch"},
              {TorterraMove::GIGA_DRAIN,  "giga_drain",  "Giga Drain"},
              {TorterraMove::IRON_TAIL,   "iron_tail",   "Iron Tail"},
              {TorterraMove::ROCK_TOMB,   "rock_tomb",   "Rock Tomb"},
              {TorterraMove::FACADE,      "facade",      "Facade"},
              {TorterraMove::STONE_EDGE,  "stone_edge",  "Stone Edge"},
              {TorterraMove::ROCK_SLIDE,  "rock_slide",  "Rock Slide"},
              {TorterraMove::BULLDOZE,    "bulldoze",    "Bulldoze"},
              {TorterraMove::STRENGTH,    "strength",    "Strength"},
              {TorterraMove::ROCK_SMASH,  "rock_smash",  "Rock Smash"},
              {TorterraMove::ROCK_CLIMB,  "rock_climb",  "Rock Climb"},
          },
          LockMode::LOCK_WHILE_RUNNING,
          TorterraMove::CRUNCH
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
    PA_ADD_OPTION(OUTSIDE_HELP_POKEMON);
    PA_ADD_OPTION(OUTSIDE_HELP_ATTACK_1);
    PA_ADD_OPTION(OUTSIDE_HELP_ATTACK_2);
    PA_ADD_OPTION(OUTSIDE_HELP_ATTACK_3);
    PA_ADD_OPTION(OUTSIDE_HELP_ATTACK_4);
    PA_ADD_OPTION(NOTIFICATIONS);

    HELP_MODE.add_listener(*this);
    OUTSIDE_HELP_POKEMON.add_listener(*this);

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

    // Show outside-help options only when OUTSIDE_HELP is selected
    bool show_help = ((HelpMode)HELP_MODE == HelpMode::OUTSIDE_HELP);
    OUTSIDE_HELP_POKEMON.set_visibility(show_help ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);

    bool show_attacks = show_help && ((OutsideHelpPokemon)OUTSIDE_HELP_POKEMON == OutsideHelpPokemon::TORTERRA);
    ConfigOptionState attack_state = show_attacks ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN;
    OUTSIDE_HELP_ATTACK_1.set_visibility(attack_state);
    OUTSIDE_HELP_ATTACK_2.set_visibility(attack_state);
    OUTSIDE_HELP_ATTACK_3.set_visibility(attack_state);
    OUTSIDE_HELP_ATTACK_4.set_visibility(attack_state);

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
        NOTIFICATION_STATUS_UPDATE,
        (HelpMode)HELP_MODE,
        OutsideHelpConfig{
            (OutsideHelpPokemon)OUTSIDE_HELP_POKEMON,
            {
                (TorterraMove)OUTSIDE_HELP_ATTACK_1,
                (TorterraMove)OUTSIDE_HELP_ATTACK_2,
                (TorterraMove)OUTSIDE_HELP_ATTACK_3,
                (TorterraMove)OUTSIDE_HELP_ATTACK_4,
            }
        }
    };

    const auto& segments = ALL_SEGMENTS();
    for (size_t i = 0; i < segments.size(); i++){
        if ((bool)*SEGMENT_CHECKBOXES[i]){
            segments[i]->run_segment(env, context, options, stats);
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}



bool activate_repel(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("Activating repel");
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_X, 80ms, 300ms);
    pbf_wait(context, 1000ms);
    pbf_press_button(context, BUTTON_PLUS, 80ms, 300ms);
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);
    pbf_press_button(context, BUTTON_B, 80ms, 300ms);
    pbf_wait(context, 1000ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2, false);
    pbf_press_button(context, BUTTON_A, 80ms, 300ms);
    context.wait_for_all_requests();

    pbf_wait(context, 1000ms);
    {
        auto repel_tab_ref = std::make_shared<const ImageRGB32>(
            RESOURCE_PATH() + "PokemonBDSP/AutoStory/Bag_RepelTab.png"
        );

        const ImageFloatBox repel_tab_box{0.735000, 0.055000, 0.045000, 0.080000};
        const double repel_tab_rmsd = 50.0;

        // BDSP bag has 8 compartment tabs; check before each press so zero-press
        // detection works when the bag already starts on the repel tab.
        bool found = false;
        for (int press = 0; press <= 8; press++){
            context.wait_for_all_requests();

            ImageMatchWatcher watcher(repel_tab_ref, repel_tab_box, repel_tab_rmsd);
            int ret = wait_until(stream, context, 400ms, {{watcher}});
            if (ret == 0){
                stream.log("[AutoStory] activate_repel: repel tab found after "
                           + std::to_string(press) + " press(es).", COLOR_GREEN);
                found = true;
                break;
            }

            if (press < 8){
                stream.log("[AutoStory] activate_repel: tab not found, pressing right ("
                           + std::to_string(press + 1) + "/8).");
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
            }
        }

        if (!found){
            stream.log("[AutoStory] activate_repel: failed to find repel compartment tab.", COLOR_RED);
            return false;
        }
    }
    context.wait_for_all_requests();
    pbf_wait(context, 400ms);
    pbf_mash_button(context, BUTTON_A, 500ms);
    pbf_mash_button(context, BUTTON_B, 2000ms);
    return true;
}


}
}
}
