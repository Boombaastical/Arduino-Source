

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

BurmyFinder_Descriptor::SingleTreeReset()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:Single Tree Reset",
        STRING_POKEMON + " LA", "Single Tree Reset",
        "Programs/PokemonLA/SingleTreeReset.html",
        "Select a single tree to hunt repeatedly.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class BurmyFinder_Descriptor::Stats : public StatsTracker{
public:
    Stats()
            : attempts(m_stats["Attempts"])
            , trees_knocked(m_stats["Trees knocked"])
            , battles(m_stats["Battles"])
            , errors(m_stats["Errors"])
            , shinies(m_stats["Shinies"])
            , alphas(m_stats["Alphas"])
        {
            m_display_order.emplace_back("Attempts");
            m_display_order.emplace_back("Trees knocked");
            m_display_order.emplace_back("Battles");
            m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
            m_display_order.emplace_back("Shinies", HIDDEN_IF_ZERO);
            m_display_order.emplace_back("Alphas", HIDDEN_IF_ZERO);
        }
        std::atomic<uint64_t>& attempts;
        std::atomic<uint64_t>& trees_knocked;
        std::atomic<uint64_t>& battles;
        std::atomic<uint64_t>& errors;
        std::atomic<uint64_t>& shinies;
        std::atomic<uint64_t>& alphas;
    };
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

std::unique_ptr<StatsTracker> SingleTreeHunter_Descriptor::make_stats() const
{
    return std::unique_ptr<StatsTracker>(new Stats());
}



SingleTreeReset::SingleTreeReset()
    : TREE_SELECT(
        "<b>Choose the code of the tree to be farmed</b><br>"
        "Refer to https://lincoln-lm.github.io/JS-Finder/Gen8/PLA-Landmark-Map/ for the tree code",
        LockMode::LOCK_WHILE_RUNNING,
        0623
    )
    , KNOCKS(
        "<b>Farm this tree this many times</b>",
        LockMode::LOCK_WHILE_RUNNING,
        1000
    )
    , FOUND_SHINY_OR_ALPHA(
        "Found Shiny or Alpha",
        true, true,
        ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATIONS({
        &SHINY_DETECTED_ENROUTE.NOTIFICATIONS,
        &FOUND_SHINY_OR_ALPHA,
        &MATCH_DETECTED_OPTIONS.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(TREE_SELECT);
    PA_ADD_OPTION(KNOCKS);
    PA_ADD_OPTION(FOUND_SHINY_OR_ALPHA);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void SingleTreeHunter::travel_to_tree(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int tree)
{
    // According to the tree number, travel from camp to the tree
    switch (tree)
    {
    case 0623:

        break;
    default:
        throw OperationFailedException(env.console, "Invalid tree number.");
    }
    context.wait_for_all_requests();
}

void SingleTreeHunter::return_to_base(SingleSwitchProgramEnvironment& env, ProControllerContext& context)
{
    
}

void SingleTreeHunter::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context)
{
    SingleTreeHunter_Descriptor::Stats& stats = env.current_stats<SingleTreeHunter_Descriptor::Stats>();
    stats.attempts++;

    //  Connect the controller
    pbf_press_button(context, BUTTON_LCLICK, 40ms, 40ms);
    
    // Fast travel to specific camp + fly to the selected tree
    
    // Knock down the tree

    bool battle_found = knock_tree(env.console, context);
    // If a battle starts, exit it
    if (battle_found) {
        handle_battle(env.console, context);
    };
    // Fly back to base camp
    fly_back_to_base_camp(env.console,context);
    // Wait 20min
    pbf_wait(1200000);
    
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}
