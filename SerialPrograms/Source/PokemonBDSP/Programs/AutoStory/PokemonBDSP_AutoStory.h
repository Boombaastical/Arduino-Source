/*  BDSP AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStory_H
#define PokemonAutomation_PokemonBDSP_AutoStory_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Options/PokemonBDSP_AutoStory_Options.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class BDSPAutoStory_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BDSPAutoStory_Descriptor();

    virtual std::unique_ptr<SingleSwitchProgramInstance> make_instance() const override;
};


class BDSPAutoStory : public SingleSwitchProgramInstance{
public:
    BDSPAutoStory();

    virtual void program(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context
        ) override;

private:
    BDSPAutoStoryInternal::RouteOptions ROUTE_OPTIONS;
};


}
}
}
#endif
