/*  BDSP AutoStory Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStoryTools_H
#define PokemonAutomation_PokemonBDSP_AutoStoryTools_H

#include <functional>
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


// ---------------------------------------------------------------------------
//  Stats
// ---------------------------------------------------------------------------

struct AutoStoryStats : public StatsTracker{
    AutoStoryStats()
        : m_checkpoint(m_stats["Checkpoint"])
        , m_segment(m_stats["Segment"])
        , m_reset(m_stats["Reset"])
    {
        m_display_order.emplace_back("Checkpoint");
        m_display_order.emplace_back("Segment");
        m_display_order.emplace_back("Reset");
    }
    std::atomic<uint64_t>& m_checkpoint;
    std::atomic<uint64_t>& m_segment;
    std::atomic<uint64_t>& m_reset;
};


// ---------------------------------------------------------------------------
//  Options passed to every segment/checkpoint
// ---------------------------------------------------------------------------

enum class StarterChoice{
    TURTWIG,
    CHIMCHAR,
    PIPLUP,
};

struct AutoStoryOptions{
    Language language;
    StarterChoice starter_choice;
    EventNotificationOption& notif_status_update;
};


// ---------------------------------------------------------------------------
//  Base classes
// ---------------------------------------------------------------------------

class AutoStory_Segment{
public:
    virtual ~AutoStory_Segment() = default;
    virtual std::string name()       const = 0;
    virtual std::string start_text() const = 0;
    virtual std::string end_text()   const = 0;
    virtual void run_segment(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) const = 0;
};

class AutoStory_Checkpoint{
public:
    virtual ~AutoStory_Checkpoint() = default;
    virtual std::string name()       const = 0;
    virtual std::string start_text() const = 0;
    virtual std::string end_text()   const = 0;
    virtual void run_checkpoint(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) const = 0;
};


// ---------------------------------------------------------------------------
//  Shared utility functions (implementations in .cpp)
// ---------------------------------------------------------------------------

//  Save the game from the overworld, increment the checkpoint counter,
//  update stats display, and send a status notification.
void checkpoint_save(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);

//  Save before the first attempt, then run `action(attempt_number)`.
//  On any thrown exception, increment the reset counter and retry.
//  Throws a fatal error after too many consecutive failures.
void checkpoint_reattempt_loop(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    std::function<void(size_t attempt_number)>&& action
);


}
}
}
#endif
