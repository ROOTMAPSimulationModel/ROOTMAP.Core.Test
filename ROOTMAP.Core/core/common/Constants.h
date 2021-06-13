#ifndef CropSimConstants_H
#define CropSimConstants_H
/* ••••••••••••••••••••••••••••••••••••••••••••••••
•• type :  INTERFACE
•• Date Created    : Monday 25th October, 1993
•• Last Modified    : Friday January 21st 1994
•• Defines        : ID numbers of  resources used explicitly
••                 : Items numbers within Dialogs and Alerts if globally useful
•••••••••••••••••••••••••••••••••••••••••••••••• */
#include "core/common/Types.h"

// #include <fp.h>
namespace rootmap
{
    /* _______________________________________
    Command number naming format :
    “cmd[MenuName][ItemName][TertiaryQualifications]”
                        OR
    “cmd[ClassName][CommandDescriptor]”
    _______________________________________
    Menu number naming formaat :
    “MENU[MenuName][ItemName][TertiaryQualifications]”
    _______________________________________ */

    /*    •    •    •    •    •    •    •    •    •    •    •
    ALRT's */
#define    ALRTAboutCropSim            130

#define    ALRTBoxStatus                502
#define    ALRTUnimplementedCommand    503
#define ALRTCheckBlocksFailed        505
#define ALRTBufferError                506

    /*    •    •    •    •    •    •    •    •    •    •    •
    'CURS's */
#define CURSCrossHair               128
#define CURSResizeHorizontal        129
#define CURSResizeVertical          130
#define CURSResizeTopLeftToBottomRight 131
#define CURSResizeBottomLeftToTopRight 132
#define CURSTick                    133
#define CURSResizeCross             134
#define CURSResizeDividerHorizontal 135
#define CURSResizeDividerVertical   136
#define CURSCommandPeriod           139
#define CURSZoomIn                  140
#define CURSZoomOut                 141

    /*    •    •    •    •    •    •    •    •    •    •    •
    'DLOG's */
#define    DLOGValueValidation            501
#define    DLOGSplash                    504
#define DLOGBatchRunInProgress      552

#define    DLOGViewCharacteristicDetails_old    702
#define    DLOGViewCharacteristicDetails        703
#define DLOGQuickStatus                        704
#define DLOGIOInformationDisplay            705
#define DLOGViewProcessDetails                706

#define    DLOGFileStatus                32222

    /*    •    •    •    •    •    •    •    •    •    •    •
    'MBAR's
    MBAR number 1 is the only one, and is numbered
    as 1 for the CBartender class to automatically handle it */
#define    MBARstandard                1


    /* _______________________________________
    Menu number naming formaat :
    “MENU[MenuName][SubmenuItemDescription][TertiaryQualifications]”
    _______________________________________ */
    /*    •    •    •    •    •    •    •    •    •    •    •
    'MENU's
    --------------------------------------------------------
    ID's 1, 2, 3, 10, 11 reserved by TCL for standard Mac Menus
    MENUapple=1
    MENUfile=2
    MENUedit=3
    */
#define MENUgo 5
    /*
    MENUfont=10
    MENUsize=11
    --------------------------------------------------------
    These first Menu ID's identify menus which can appear
    on the actual MenuBar */
#define    MENUview 20

    // MENU's 21..36 reserved for table menus.
    // use:
    // 
    //     ScoreboardStratum ss = 2;
    //     short menuID = ss.getMenuID("Table","");
    // 

#define    MENUfilter 40
#define    MENUplant 41
#define    MENUclock 42
#define    MENUwindows 43
#define    MENUgraph 44

    /* -----------------------------------------------------
    MENUs 51..66 reserved for Edit>Layer submenus.

    use:

        ScoreboardStratum ss = 2;
        short menuID = ss.getMenuID("Edit","Layers");

    Examples :
    Edit Air Layers submenu ID = 51
    Edit Soil Layers submenu ID = 53
    ----------------------------------------------------- */

    /* _______________________________
    MENUs 67..82 reserved for Plant Characteristic and Summary submenus.
    use :

        ScoreboardStratum ss = 2;
        short menuID = ss.getMenuID("Table","Plant");

    */

    /*  MENUs 83..130 reserved for “Layers” submenus on the Table menu.

    use :

        ScoreboardStratum ss = 2;
        short menuID = ss.getMenuID("Table","Layers");

     Examples :
    Layers submenu for gvt 0 (Air) X Dimension (vFront) =83
    Layers submenu for gvt 0 (Air) Y Dimension (vSide) =84
    Layers submenu for gvt 0 (Air) Z Dimension (vTop) =85
    Layers submenu for gvt 2 (Soil) X Dimension (vFront) =89
    Layers submenu for gvt 2 (Soil) Y Dimension (vSide) =90
    Layers submenu for gvt 2 (Soil) Z Dimension (vTop) =91
    */


    /* _______________________________
    MENUs 131..138 Plant Menu sub-menus */
#define    MENUplantChangeCharacteristicsub 131            // Plant•Delete Characteristic
#define    MENUplantDeleteCharacteristicsub 132            // Plant•Delete Characteristic
#define    MENUplantDuplicatePlantsub 133        // Plant•Duplicate a Plant
#define    MENUplantChangePlantsub 134            // Plant•Change a Plant
#define    MENUplantDeletePlantsub 135            // Plant•Delete a Plant
#define    MENUplantDuplicatePlantTypessub 136    // Plant•Duplicate a Plant Type
#define    MENUplantChangePlantTypessub 137        // Plant•Change a Plant Type
#define    MENUplantDeletePlantTypessub 138        // Plant•Delete a Plant Type

    /* _______________________________
    MENUs 139-253 (or so) reserved for Multi-characteristic Processes
    extra characteristics' submenus. These MENU IDs are independent of GVT,
    and hence not computable. */
    // DEPRECATED since they all go on the same menu, since they were giving just waaaayyy
    // much trouble and had a peculiar anti-user-interface aspect to them

    // The View Menu's only submenu, the ‘Scoreboard’ item
#define MENUViewScoreboards 254

    // The Table Menu(s) sub-menus
#define    MENUNewTableTypes 255

    // Volume Types Pop-Up, used for some dialogs
    // (i think, i haven't used it yet)
    //
    //#define MENUPopUpScoreboardStrata 999


    // ∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞ 
    // 'STR 's 
#define    STRPrefsFileTitle 613
#define    STRUntitledFileTitle 617
#define    STRdefaultPlantName 621
#define    STRdefaultPlantTypeName 622


    // ∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞ 
    // 'STR#'s and some of the indices into them <STRL=STR List> SWTI = StrlWindowTitles Index 
#define STRLWindowTitles 400

    enum ESTRLWindowTitles
    {
        STRLwtView = 0,
        STRLwtStatus,
        STRLwtAir,
        STRLwtLitter,
        STRLwtSoil,
        STRLwtDrainage
    };

#define STRLwtFirstGVT 3

    // STR# 401 - Logged Error strings
#define STRLErr 401

    enum ESTRLErr
    {
        STRLErrFileTooRecent = 0,
        STRLErrProcessDoesntExist,
        STRLErrDidntRecognizeFloatingPointData,
        STRLErrDidntRecognizeDataType,
        STRLErrCouldntFindString
    };

    // STR# 402 - View Direction names
#define STRLViewDirections 402

    enum ESTRLViewDirections
    {
        STRLvdNoDirection = 0,
        STRLvdFront = 1,
        STRLvdSide = 2,
        STRLvdTop = 3
    };

    // STR# 403 - Various Table Axis Labels
#define STRLTableAxisLabels 403

    enum ESTRLTableAxisLabels
    {
        STRLtalNONE = 0,
        STRLtalFront,
        STRLtalSide,
        STRLtalDepth,
        STRLtalHeight,
        STRLtalIntervalAlongSide,
        STRLtalIntervalAlongFront,
        STRLtalDepthInterval,
        STRLtalHeightInterval
    };

#define kTableLabelAxisNONE STRLtalNONE
#define kTableLabelIntervalNONE STRLtalHeight

    // STR# 404 - Miscellaneous strings
#define STRLMiscellaneous 404

    enum ESTRLMiscellaneous
    {
        STRLmCopyOf = 0,
        STRLmTotalRootLength,
        STRLmNewPlant,
        STRLmViewedFrom,
        STRLmLayerAlong,
        STRLmInputs,
        STRLmOutputs,
        STRLmGeneralInformation
    };

    // STR# 405 - Used by sprintGroundVolumeType
    // volume type, display strings
#define STRL_GVTDisplay 405

    // STR# 406 - Used by StringToVolumeType
#define STRL_GVTInternal 406
    /*
    enum {
        STRLgiAbove = 1,
        STRLgiLitter,
        STRLgiBelow,
        STRLgiDrainage,
        STRLgiALL,
        STRLgiNONE
        };
    */

    // STR# 407 - Plant Summary Routine Names; see sprintSummaryRoutine()
#define STRL_SummaryRoutines 407

    enum ESTRL_SummaryRoutines
    {
        STRLsrNone = 0,
        STRLsrRootLength,
        STRLsrRootLengthDensity,
        STRLsrTipCount,
        STRLsrTipCountDensity,
        STRLsrBranchCount,
        STRLsrBranchCountDensity,
        STRLsrVolume,
        STRLsrSurfaceArea
    };

    // STR# 408 - buffer error descriptions
#define STRL_BufferErrors 408

    enum ESTRL_BufferErrors
    {
        STRLbeMakingProcessBuffers = 0,
        STRLbeChangingViewDirection,
        STRLbeTurningOffWrap,
        STRLbeTurningOnWrap,
        STRLbeTurningOnBoxes,
        STRLbeTurningOnBoxColours,
        STRLbeTurningOnAScoreboard,
        STRLbeResizing,
        STRLbeChangingScale,
        STRLbeMakingWindow
    };

#define STRL_PlantBrowserButtonNames 409
#define STRL_PlantTypeBrowserButtonNames 410
#define STRL_PlantSummaryBrowserButtonNames 411

    // the list of labels for input association information, shown in the left
    // column of the right pane of the IO Display Window when an
    // input association is selected in the left pane of the window.
#define STRLJInputAssociationInformationLabels 450

    // the list of labels for output association information, shown in the left
    // column of the right pane of the IO Display Window when an
    // output association is selected in the left pane of the window.
#define STRLJOutputAssociationInformationLabels 451

    // label list for the meta-association information, shown in the left
    // column of the right pane of the IO Display Window when the
    // meta-association is selected in the left pane of the window.
#define STRLJMetaAssociationInformationLabels 452

    // label list for association label information. Just one string.
#define STRLJAssociationLabelInformationLabel 453

    // STR# 900 - process names
#define STRLProcessNames 900

    enum ESTRLProcessNames
    {
        STRLmnPlant = 1,
        STRLmnPlantKeeper,
        STRLmnNull
    };

    // STRL 901 for gvt characteristic names
#define STRLCharacteristicNames 901

    enum ESTRLCharacteristicNames
    {
        STRLxnNull = 1,
        STRLxnTotal,
        STRLxnOrder,
        STRLxnRootLength,
        STRLxnRootTips,
        STRLxnRootVolume,
        STRLxnRootSurfaceArea
    };

    // STRL 902 for gvt unit names
#define STRLUnitStrings 902

    enum ESTRLUnitStrings
    {
        STRLxuCentimetresPerMl = 0,
        STRLxuMicrogramsPerGram,
        STRLxuPercent,
        STRLxuPercentDryWeight,
        STRLxuGramsPerMl,
        STRLxuDegreesCelsius,
        STRLxuNull,
        STRLxuCentimetres,
        STRLxuMicrogramsPerMlPerDay,
        STRLxuDegrees,
        STRLxuHours,
        STRLxuCentimetersPerHour,
        STRLxuTipCount,
        STRLxuBranchCount,
        STRLxuMl,
        STRLxuMillilitres,
        STRLxuCubicCentimetres,
        STRLxuSquareCentimetres,
        STRLxuCentimetresPerCubicCentimetre,
        STRLxuMicromolesPointZeroOneCalciumChloride,
        STRLxuBars,
        STRLxuMicrogramsPerGram2,
        STRLxuMicrogramsPerCubicCentimetre,
        STRLxuCentimetresCubedWaterPerCentimetresCubedSoil,
        STRLxuMigrogramsNitratePerCubicCentimetre
    };

#define kCollectedErrorsStringList 32035

    enum ECollectedErrorsStringList
    {
        kCouldntFindVitalResource = 1,
        kCouldntFindInputFolder,
        kCouldntMakeOutputFolder,
        kCouldntMakeDebugFolder,
        kCouldntFindInputFile,
        kCouldntMakeOutputFile,
        kAuxWhileFindingInputFiles,
        kAuxWhileMakingOutputFiles,
        kAuxStatusContinuingWithoutFolder,
        kAuxStatusContinuingWithoutFile,
        kAuxStatusUsingDefaults,
        kCouldntGetDirectoryID,
        kCouldntAllocateMemory,
        kUnrecognisedFileErrorOccurred,
        kCouldntReadFromFile,
        kCouldntWriteToFile,
        kOSErrorOccurred,
        kMemoryErrorOccurred,
        kResErrorOccurred,
        kAuxWhileLoadingFile,
        kCouldntFindProcess
    };

    // ∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞ 
    // 'WIND's 
#define    WINDmessagedebug 898
#define    WINDdebug 899
#define    WINDviewNew 900
#define    WINDtableNew 901
#define    WINDquickstatus 902
#define    WINDstatus 903
#define    WINDerrorscollected 904
#define    WINDscoreboardview 905

    /* ------------------------------------------------------------
    Other constants */
    // for infinite/infinity, use Utility::isInfinite(double) or Utility::infinity()
#define FMOD std::fmodf
#define FLOOR std::floorf

#define kIsNotBelow false
#define kIsNew true
#define kIsRestored false
#define kIsNotNew false

#define kInsertItem true
#define kDontInsertItem false
#define kReplaceItem false

#define kRepeat true
#define kNotRepeat false
#define kWrap true
#define kNotWrap false

    // #define kDefaultSeedLocationSloc 128    // index number of 'Sloc' resource

    // #define kBackspaceKey ( char )( 0x08 )
    // #define kTabKey ( char )( 0x09 )
    // #define kReturnKey ( char )( 0x13 )

    enum ECharacteristicColour
    {
        kCyanCharacteristic = 0,
        kMagentaCharacteristic = 1,
        kYellowCharacteristic = 2
    };

    //#define kCyanCharacteristic 0
    //#define kMagentaCharacteristic 1
    //#define kYellowCharacteristic 2

    /*
    Note: on PowerPC extended is undefined.
        on 68K, when mc68881 is    : on, extended is 96 bits.
                            : off, extended is 80 bits.
          Some old toolbox routines require an 80 bit extended so we define extended80 */

#ifdef RM_FIXED_SIZEOF_DOUBLET_STUFF
          /* NOTE THAT double for PowerPC and extended (long double) are defined as
          "double_t" for both platforms. The reason we do stuff here is because when
          doing some stuff elsewhere (generating long double random numbers, for instance)
          we need the size of the double_t. We like this to be known with*/
#if GENERATING68K 
          /* on 68K, long double (a.k.a. extended) is always the fastest.  It is 80 or 96-bits */
#if GENERATING68881
#define bit_sizeof_double_t 96
#define byte_sizeof_double_t 12
#define word_sizeof_double_t 6
#else //IS 68k, IS NOT 68881
#define bit_sizeof_double_t 80
#define byte_sizeof_double_t 10
#define word_sizeof_double_t 5
#endif // #if GENERATING68881
#else // IS NOT 68k, ie. GENERATINGPOWERPC is true
          /* on PowerPC, double = 64-bit which is fastest.  float = 32-bit */
#define bit_sizeof_double_t 64
#define byte_sizeof_double_t 8
#define word_sizeof_double_t 4
#endif // #if GENERATING68K
#endif // #ifdef RM_FIXED_SIZEOF_DOUBLET_STUFF

    const double PI = 3.14159265358979323846;
} /* namespace rootmap */

#endif // #ifndef CropSimConstants_H
