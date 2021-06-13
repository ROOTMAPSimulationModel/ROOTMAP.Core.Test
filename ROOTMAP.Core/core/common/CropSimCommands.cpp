//
// File:
//  CropSimCommands.cp
//
// Description:
//  Defines the IsCommand_XXX functions declared in CropSimCommands.h
//
#include "core/common/CropSimCommands.h"
#include "core/scoreboard/ScoreboardStratum.h"
//#include "core/scoreboard/GroundVolumeTypes.h"

/*
RANGE TESTING
*/
#include "core/common/ExtraStuff.h"

namespace rootmap
{
#if defined ROOTMAP_ISCOMMAND_FUNCTIONS


    Boolean IsCommand_NewTable(long theCmd,
        GroundVolumeType* volume)
    {
        long int firstNewTableItemCmdNumber = ScoreboardStratum::getFirstItemCommandNumber("Table", "New Table");
        long int lastNewTableItemCmdNumber = ScoreboardStratum::getLastItemCommandNumber("Table", "New Table");

        if ((theCmd >= firstNewTableItemCmdNumber) &&
            (theCmd <= lastNewTableItemCmdNumber)
            )
        {
            *volume = theCmd - firstNewTableItemCmdNumber;
            return true;
        }

        return false;
    }


    /*
    */
    Boolean IsCommand_Scoreboard(long theCmd, GroundVolumeType* volume)
    {
        long int firstScoreboardItemCmdNumber = ScoreboardStratum::getFirstItemCommandNumber("View", "Scoreboard");
        long int lastScoreboardItemCmdNumber = ScoreboardStratum::getLastItemCommandNumber("View", "Scoreboard");

        if ((theCmd >= firstScoreboardItemCmdNumber) &&
            (theCmd <= lastScoreboardItemCmdNumber + 1)
            )
        {
            *volume = theCmd - firstScoreboardItemCmdNumber;
            return true;
        }

        return false;
    }

    /*
    */
    Boolean IsCommand_PlantCharacteristic(long theCmd, GroundVolumeType* volume)
    {
        //#pragma unused (theCmd,volume)
#pragma message (HERE " fix IsCommand_PlantCharacteristic")
#if 0
        if ((theCmd >= FirstPlantCharacteristicID_FirstGVT) &&
            (theCmd <= GetLastUsedPlantCharacteristicID(vtALL)))
        {
            *volume = (GroundVolumeType)((theCmd - FirstPlantCharacteristicID_FirstGVT)
                / FirstPlantCharacteristicIDGVTincrement);
            return true;
        }
        else
        {
            return false;
        }
#endif // #if 0
        return false;
    }

    /*
    */
    Boolean IsCommand_Plant(long theCmd)
    {
#pragma message (HERE " fix IsCommand_Plant")
        return false; //    return (    (theCmd >= FirstPlantID) /*&& (theCmd <= GetLastUsedPlantID(vtALL)) */);
    }

    /*
    */
    Boolean IsCommand_ProcessCharacteristic(long int theCmd, GroundVolumeType* volume)
    {
#pragma message (HERE " fix IsCommand_ProcessCharacteristic")
#if defined ROOTMAP_TODO
        if ((theCmd >= FirstProcessCharacteristicID_FirstGVT) &&
            (theCmd <= GetLastUsedProcessCharacteristicID(vtALL)))
        {
            *volume = (GroundVolumeType)((theCmd - FirstProcessCharacteristicID_FirstGVT)
                / FirstProcessCharacteristicIDGVTincrement);
            return true;
        }
        else
        {
            return false;
        }
#endif // #if defined ROOTMAP_TODO
        return false;
    }
#endif // #if defined ROOTMAP_ISCOMMAND_FUNCTIONS
} /* namespace rootmap */
