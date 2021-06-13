//Heritage
#include "simulation/process/common/Process.h"
#include "simulation/data_access/dummyconfig/DummyProcessWith2Characteristics.h"


namespace rootmap
{
    IMPLEMENT_DYNAMIC_CLASS(DummyProcessWith2Characteristics, Process)

        DummyProcessWith2Characteristics::DummyProcessWith2Characteristics()
        : Process()
    {
    }

    DummyProcessWith2Characteristics::~DummyProcessWith2Characteristics()
    {
    }

    bool DummyProcessWith2Characteristics::DoesOverride() const { return (false); }
} /* namespace rootmap */

