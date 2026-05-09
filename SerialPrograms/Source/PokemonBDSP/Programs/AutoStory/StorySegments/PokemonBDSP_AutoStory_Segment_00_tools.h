#pragma once

#include "PokemonBDSP_AutoStory_Segment_00.h"
#include "CommonTools/Async/InferenceRoutines.h"

#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"

#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            // ---------------------------------------------------------------------------
            // Utility Watchers
            // ---------------------------------------------------------------------------

            class BattleDialogWatcher : public DetectorToFinder<BattleDialogDetector> {
            public:
                BattleDialogWatcher(Color color = COLOR_RED);
            };

            // ---------------------------------------------------------------------------
            // Time / setup helpers
            // ---------------------------------------------------------------------------

            void set_time_fast(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                const DateTime& target
            );

            // ---------------------------------------------------------------------------
            // Starter selection
            // ---------------------------------------------------------------------------

            void select_turtwig(
                VideoStream& stream,
                ProControllerContext& context
            );

            void select_chimchar(
                VideoStream& stream,
                ProControllerContext& context
            );

            void select_piplup(
                VideoStream& stream,
                ProControllerContext& context
            );

            // ---------------------------------------------------------------------------
            // Battles
            // ---------------------------------------------------------------------------

            void fight_starly(
                VideoStream& stream,
                ProControllerContext& context
            );

            void fight_tristan(
                VideoStream& stream,
                ProControllerContext& context
            );

            void fight_Natalie(
                VideoStream& stream,
                ProControllerContext& context
            );

            // ---------------------------------------------------------------------------
            // Catching / healing
            // ---------------------------------------------------------------------------

            void catch_1_pokemon(
                VideoStream& stream,
                ProControllerContext& context
            );

            void use_potion_first_pokemon(
                VideoStream& stream,
                ProControllerContext& context
            );

            // ---------------------------------------------------------------------------
            // Grass movement helpers
            // ---------------------------------------------------------------------------

            void walk_right_until_on_path(
                VideoStream& stream,
                ProControllerContext& context
            );

            void walk_up_through_grass_2(
                VideoStream& stream,
                ProControllerContext& context
            );

            void walk_right_through_grass_2(
                VideoStream& stream,
                ProControllerContext& context
            );

            void walk_left_through_grass_3(
                VideoStream& stream,
                ProControllerContext& context
            );

            void walk_up_through_grass_3(
                VideoStream& stream,
                ProControllerContext& context
            );

            // ---------------------------------------------------------------------------
            // Trainer navigation
            // ---------------------------------------------------------------------------

            void walk_right_to_first_trainer(
                VideoStream& stream,
                ProControllerContext& context
            );

        }
    }
}