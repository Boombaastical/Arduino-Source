#pragma once

#include <vector>
#include "Common/Cpp/DateTime.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            // Date/Time setup segment for AutoStory
            // Navigates to system date/time settings and sets the date/time

            typedef void (*DateTimeSegmentFunction)(VideoStream&, ProControllerContext&, const DateTime&);

            extern DateTimeSegmentFunction set_datetime_segment;

        }
    }
}
