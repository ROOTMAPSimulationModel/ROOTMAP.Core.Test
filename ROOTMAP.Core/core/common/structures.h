#ifndef Structures_H
#define Structures_H
/////////////////////////////////////////////////////////////////////////////
// Name:        Structures.h
// Purpose:     Declaration of structures and Types used by RootMap
// Created:     25/10/1993 (Monday!)
// Author:      RvH
// $Date: 2009-02-25 04:10:39 +0900 (Wed, 25 Feb 2009) $
// $Revision: 36 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/macos_compatibility/macos_compatibility.h"
#include "core/common/Types.h"


namespace rootmap
{
    enum ViewDirection
    { // corresponds to the...
        vSide = 0, // X
        vFront = 1, // Y
        vTop = 2, // Z
        // ...ScoreboardDimension

        vNONE = -1
    };

    //#define NUM_VIEWDIRECTIONS 3
    const long int NumberOfViewDirections = 3;

    enum ScoreboardDimension
    {
        sdNONE = -1,
        sdFrontBack = 0,
        sdLeftRight = 1,
        sdTopBottom = 2
    };

    enum Plane
    {
        planeNONE = -1,
        planeXZ = 0,
        planeYZ = 1,
        planeXY = 2
    };


    enum EPosNegSign
    {
        negative = -1,
        positive = +1
    };

    // ROOTMAP TODO: This is a bit of a pointless enumeration
    // ROOTMAP TODO: Even if it weren't pointless, it would belong in the
    //               simulation layer (not core)
    typedef OSType ProcessActivity;

    enum EProcessActivity
    {
        paNONE, // ='NONE',
        paLeaching, // ='Leac',
        paGrowing, // ='Grow',
        paMiscellaneous, // ='Misc',
        paIPCommunicator // ='Comm'
    };


    // PlantCoordinator types
    enum EAxisType
    {
        Seminal,
        Nodal
    };

    typedef long AxisType;

#define SUMMARISE_ALL_PLANTS (-1)
#define SUMMARISE_ALL_ORDERS (-1)

    /**
     *
     */
    enum SummaryRoutine
    {
        srNone = 'NONE',
        srRootLength = 'RLen',
        srRootLengthDensity = 'RLDy',
        srRootTipCount = 'RTCt',
        srRootTipCountDensity = 'RTCD',
        srRootBranchCount = 'TBCt',
        srRootBranchCountDensity = 'RBCD',
        srRootVolume = 'RVol',
        srRootSurfaceArea = 'RSAr',

        srFirstRootSummary = srRootLength,
        srLastRootSummary = srRootSurfaceArea
    };

    const signed long int NumberOfSummaryRoutineTypes = 8;


    enum SummaryRoutineGroup
    {
        summary_type_root_length_dependent = 0,
        summary_type_tip_count_dependent = 1,
        summary_type_branch_count_dependent = 2
    };

#define NUMBER_OF_SUMMARY_TYPES 3
#define SUMMARY_TYPES_MAX_ARRAY_INDEX (NUMBER_OF_SUMMARY_TYPES-1)

    // these constants are what they are for "speed" purposes
    enum EWrapDirection
    {
        wraporderNone = 0L,
        wraporderX = 1L,
        wraporderY = wraporderX << 1,
        wraporderBoth = wraporderX | wraporderY
    };

    typedef long WrapDirection;
} /* namespace rootmap */

#endif // #ifndef Structures_H
