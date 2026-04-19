#pragma once

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

#include "PokemonBDSP_RouteConfig.h"

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

    EnumDropdownOption<BDSPRouteType> ROUTE_TYPE;

    BooleanCheckBoxOption STARTER_ENABLED;

    EnumDropdownOption<BDSPStarter> STARTER_SELECT;

    BooleanCheckBoxOption STARTER_SHINY;

    BooleanCheckBoxOption CATCH_LEGENDARY;

};

}
}
}
