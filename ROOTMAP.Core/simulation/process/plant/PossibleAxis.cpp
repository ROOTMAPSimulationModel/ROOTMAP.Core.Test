#include "simulation/process/plant/PossibleAxis.h"

namespace rootmap
{
    PossibleAxis::PossibleAxis()
        : possibleaxis_start_lag(0)
        , possibleaxis_probability(1)
        , possibleaxis_next(0)
    {
    }

    PossibleAxis::PossibleAxis(double start_lag, double probability)
        : possibleaxis_start_lag(start_lag)
        , possibleaxis_probability(probability)
        , possibleaxis_next(0)
    {
    }

    void PossibleAxis::Push(PossibleAxis* new_axis)
    {
        if (possibleaxis_next != 0)
        {
            possibleaxis_next->Push(new_axis);
        }
        else
        {
            possibleaxis_next = new_axis;
        }
    }

    PossibleAxis* PossibleAxis::Pop()
    {
        PossibleAxis* return_axis = 0;

        if (possibleaxis_next != 0)
        {
            return_axis = possibleaxis_next->Pop();
            if (return_axis == possibleaxis_next)
            {
                possibleaxis_next = 0;
            }
        }
        else
        {
            return_axis = this;
        }

        return (return_axis);
    }
} /* namespace rootmap */


