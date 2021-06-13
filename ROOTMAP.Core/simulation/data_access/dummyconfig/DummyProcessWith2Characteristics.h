#ifndef DummyProcessWith2Characteristics_H
#define DummyProcessWith2Characteristics_H

#include "simulation/process/common/Process.h"

namespace rootmap
{
    class DummyProcessWith2Characteristics : public Process
    {
        DECLARE_DYNAMIC_CLASS(DummyProcessWith2Characteristics)

    public:


    public:
        DummyProcessWith2Characteristics();
        ~DummyProcessWith2Characteristics();

        virtual bool DoesOverride() const;

    private:
    };
} /* namespace rootmap */

#endif // #ifndef DummyProcessWith2Characteristics_H
