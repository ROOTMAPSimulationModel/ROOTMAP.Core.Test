#ifndef PossibleAxis_H
#define PossibleAxis_H
#include "core/common/Types.h"

namespace rootmap
{
    class PossibleAxis
    {
        TCL_DECLARE_CLASS

    public:
        PossibleAxis();
        PossibleAxis(double start_lag, double probability);

        double GetStartLag() const;
        double GetProbability() const;
        void SetStartLag(double startLag);
        void SetProbability(double probability);

        void Push(PossibleAxis* new_axis);
        PossibleAxis* Pop();

        double possibleaxis_start_lag;
        double possibleaxis_probability;

        PossibleAxis* possibleaxis_next;
        PossibleAxis* GetNext();
    };

    inline double PossibleAxis::GetStartLag() const
    {
        return (possibleaxis_start_lag);
    }

    inline double PossibleAxis::GetProbability() const
    {
        return (possibleaxis_probability);
    }

    inline void PossibleAxis::SetStartLag(double startLag)
    {
        possibleaxis_start_lag = startLag;
    }

    inline void PossibleAxis::SetProbability(double probability)
    {
        possibleaxis_probability = probability;
    }

    inline PossibleAxis* PossibleAxis::GetNext()
    {
        return possibleaxis_next;
    }
} /* namespace rootmap */

#endif // #ifndef PossibleAxis_H
