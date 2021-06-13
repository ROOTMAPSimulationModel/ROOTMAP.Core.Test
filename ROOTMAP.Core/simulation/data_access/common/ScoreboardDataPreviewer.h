#ifndef ScoreboardDataPreviewer_H
#define ScoreboardDataPreviewer_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardDataPreviewer.h
// Purpose:     Declaration of the ScoreboardDataPreviewer class
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/DataPreviewer.h"
#include "core/scoreboard/BoundaryArray.h"

namespace rootmap
{
    class ScoreboardDAI;
    class ScoreboardStratum;

    class ScoreboardDataPreviewer : public DataPreviewer
    {
    public:
        ScoreboardDataPreviewer();
        virtual ~ScoreboardDataPreviewer();

        virtual void Populate(ScoreboardDAI& scoreboardData);

    protected:
        BoundaryValueVector m_XBoundaries;
        BoundaryValueVector m_YBoundaries;
        BoundaryValueVector m_ZBoundaries;
        std::string m_name;
        ScoreboardStratum m_stratum;
    };
} /* namespace rootmap */

#endif // #ifndef ScoreboardDataPreviewer_H
