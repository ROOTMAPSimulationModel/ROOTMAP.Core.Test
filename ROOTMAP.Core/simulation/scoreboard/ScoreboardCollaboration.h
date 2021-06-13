//
//
// CCollaborator MECHANISM STRUCTURES
//
//
// EditCellInfo
// For the purposes of Broadcasting Changes to CScoreboard & CScoreboardTable
//
// Used as the 'info' parameter of BroadcastChange() and ProviderChanged() member functions
#ifndef ScoreboardCollaboration_H
#define ScoreboardCollaboration_H

#include "core/scoreboard/ScoreboardBox.h"
#include "core/scoreboard/ScoreboardStratum.h"
#include "core/macos_compatibility/CCollaborator.h"

namespace rootmap
{
    struct EditCellInfo : public ::CCollaboratorInfo
    {
        long characteristicIndex;
        double value;
        BoxCoordinate box;
        ScoreboardStratum stratum;
    };

    // enumerations for the BroadcastChange() "reason"
    enum
    {
        scoreboardEdittedCell,
        scoreboardSimulationStepComplete
    };
} /* namespace rootmap */

#endif // #ifndef ScoreboardCollaboration_H
