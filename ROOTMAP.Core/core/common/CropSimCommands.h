// ••••••••••••••••••••••••••••••••••••••••••••••••
// •• 
// •• File:
// ••   CropSimCommands.h
// •• 
// •• Description:
// ••   Declares menu and other command numbers used by DoCommand methods,
// ••   and IsCommand... functions.
// •• 
// •• 
// •• Created:
// ••   Monday 25th October, 1993
// •• 
// •• Modified:
// ••   Friday January 21st 1994 RvH added some commands
// ••   20020804 RvH changed some C comments to C++, added IsCommand functions
// •• 
// •• 
// •• 
// •• 
// ••••••••••••••••••••••••••••••••••••••••••••••••••
#ifndef CropSimCommands_H
#define CropSimCommands_H
//#error CropSimCommands deprecated
#include "core/macos_compatibility/macos_compatibility.h"

/* _______________________________________
Command number naming format :
“cmd[MenuName][ItemName][TertiaryQualifications]”
                    OR
“cmd[ClassName][CommandDescriptor]”
_______________________________________
Command number naming format : (also described at the top of this file)
“cmd[MenuName][ItemName][TertiaryQualifications]”
                    OR
“cmd[ClassName][CommandDescriptor]”

-------------------------------------------------------------

The Apple Menu
     cmdAbout = 256     predefined by THINK Pascal

-------------------------------------------------------------
The CMY Popup Menu 1050 ≤ cmd# ≤ 1052 */
#define cmdPopupCMYSelectCyan 1050
#define cmdPopupCMYSelectMagenta 1051
#define cmdPopupCMYSelectYellow 1052

/* -------------------------------------------------------------
The Edit Menu 1500 ≤ #'s ≤ 1899 */
#define    cmdEditRainfall 1501
#define    cmdEditLeaching 1502

/* Command #'s 1510-1525 reserved for ‘VolumeType Layers’
editting submenus. Examples :
#define    cmdEditAirLayers cmdEditLayersSubFirstGVT+0 (equals 1510)
#define    cmdEditLitterLayers cmdEditLayersSubFirstGVT+1 (equals 1511)
#define    cmdEditSoilLayers cmdEditLayersSubFirstGVT+2 (equals 1512)
#define    cmdEditDrainageLayers cmdEditLayersSubFirstGVT+3 (equals 1513)

OR PERHAPS

#define cmdEditLayersStratum0 1510
#define cmdEditLayersStratum1 1511
#define cmdEditLayersStratum2 1512
#define cmdEditLayersStratum3 1513

*/

#define cmdDemoModeDialog 1526
#define cmdDoDemoMode 1527

#define cmdEditPlants 1530
#define cmdEditPlantTypes 1531
#define cmdEditPlantSummaries 1532


/* -------------------------------------------------------------
The Run Menu 1900 ≤ #'s ≤ 1999 */
#define cmdRunGo 1900
#define cmdRunStop 1902
#define cmdRunReset 1903
#define    cmdRunSingleStep 1904
#define    cmdClockSetTime 1905
#define cmdRunBatch 1921

//#pragma mark "Commands - Views"
/* -------------------------------------------------------------
The View Menu
#'s from 2000-2999 */
#define    cmdViewNew 2000
#define    cmdViewFront 2010
#define    cmdViewSide 2011
#define    cmdViewTop 2012
// ————————————————————
#define    cmdViewRepeat 2020
#define    cmdViewWrap 2021
// ————————————————————
#define    cmdViewZoomIn 2022
#define cmdViewZoomOut 2023
// ————————————————————
//
#define    cmdViewBoundary 2024
#define    cmdViewBoxes 2025
#define    cmdViewBoxColours 2026
#define    cmdViewScoreboard 2027
#define    cmdViewDetails 2028
#define    cmdViewCharacteristicDetailsDialog 2029
#define    cmdViewDisplayedProcessDialog 2030
//
// ——————————————————————————————
//
// cmd#s 2031..2047 reserved for items in the ‘Scoreboard’ submenu.
// Use “gvt2TableMenu_Scoreboard_Cmd(theVT)”
// (#include "GroundVolumeTypes.h")
//
// Examples :
// View Air Scoreboard = 2031
// View Litter Scoreboard = 2032
// View Soil Scoreboard = 2033
// View Drainage Scoreboard = 2034
// View All Scoreboards = 2047
//
// ——————————————————————————————
//
#define cmdViewCharacteristicDetailsApply 2100
#define cmdViewDisplayedProcessesApply 2101
#define cmdViewDisplayedProcessesAllOn 2102
#define cmdViewDisplayedProcessesAllOff 2103

//#pragma mark "Commands - Tables"
//
// ——————————————————————————————
// The Table Menus
// ——————————————————————————————
//
// cmd#s 2950..2965 reserved for items in the ‘New Table’ submenu.
// Use “ScoreboardVolumeStrata::TableMenuNewTableSubenuItemCommandNumber()”
//
// Examples :
// New Air Table = 2950
// New Litter Table = 2951
// New Soil Table = 2952
// New Drainage Table = 2953
//

// ——————————————————————————————
//
// cmd#s 2966..2981 reserved for table menus' ‘Layers’ submenu item.
// Use “gvt2TableMenu_Layers_Cmd(theVT)”
// (#include "GroundVolumeTypes.h")
//
// Examples :
// Air Table Menu's Layers item cmd# = 2966
// Litter Table Menu's Layers item cmd# = 2967
// Soil Table Menu's Layers item cmd# = 2968
// Drainage Table Menu's Layers item cmd# = 2969
// ——————————————————————————————

#define    cmdTableNextLayer 2900
#define    cmdTableLastLayer 2901
#define    cmdTableFromFront 2902
#define    cmdTableFromSide 2903
#define    cmdTableFromTop 2904
// 
// ———————————————————— 
// 
#define    cmdTableDetails 2905
#define    cmdTableDoBoxStatusAlert 2906
// 
// ———————————————————— 
// cmd#s 2982..2997 reserved for table menus ‘Plant’ xtic submenu item.
// Use “gvt2TableMenu_Plant_Cmd(theVT)” (#include "GroundVolumeTypes.h")
// NO. Use :
//  ScoreboardStratum ss(theVT);
//  ss.getMenuCommandNumber("Table", "Plant");
// 
// Examples :
// Air Table Menu Plant item = 2982
// Litter Table Menu Plant item = 2983
// Soil Table Menu Plant item = 2984
// Drainage Table Menu Plant item = 2985
// ——————————————————————————————

//#pragma mark "Commands - Filter Menu"
// ——————————————————————————————
//  The Filter MENU 
//  #s 3000..3049 */
#define    cmdFilterNew 3000
#define    cmdFilterOff 3001
#define    cmdFilterChoose 3002
#define    cmdFilterChange 3003
#define    cmdFilterDelete 3004
#define    cmdFilterStepForward 3005
#define    cmdFilterStepBackward 3006

//#pragma mark "Commands - Clock Menu"
// ——————————————————————————————
// The Clock MENU 
// WAS 3050..3099, is now the Run Menu
//

/* ------------------------------------------------------------
The Graph MENU
3100..3149*/

//#pragma mark "Commands - Plants"
// -------------------------- Plants
#define    cmdNewPlant 3150
#define    cmdDuplicatePlant 3151
#define    cmdChangePlant 3152
#define    cmdDeletePlant 3153
#define cmdExportPlant 3154
#define cmdImportPlant 3155
// -------------------------- Plant Types
#define    cmdNewPlantType 3160
#define    cmdDuplicatePlantType 3161
#define    cmdChangePlantType 3162
#define    cmdDeletePlantType 3163
#define    cmdExportPlantType 3164
#define    cmdImportPlantType 3165
// -------------------------- Plant Summaries
#define    cmdNewPlantSummary 3170
#define    cmdDuplicatePlantSummary 3171
#define    cmdChangePlantSummary 3172
#define    cmdDeletePlantSummary 3173

// --------------------------

// #define    cmdChangePlant_FirstPlant                        0x00200000
// #define    cmdChangePlant_LastPlant                        0x00207FFF
// #define    cmdDuplicatePlant_FirstPlant                    0x00208000
// #define    cmdDuplicatePlant_LastPlant                    0x0020FFFF
// #define    cmdDeletePlant_FirstPlant                        0x00300000
// #define    cmdDeletePlant_LastPlant                        0x00307FFF

// #define    cmdChangePlantType_FirstPlantType                0x03008000
// #define    cmdChangePlantType_LastPlantType                0x0300FFFF
// #define    cmdDuplicatePlantType_FirstPlantType            0x00400000
// #define    cmdDuplicatePlantType_LastPlantType            0x00407FFF
// #define    cmdDeletePlantType_FirstPlantType                0x00408000
// #define    cmdDeletePlantType_LastPlantType                0x0040FFFF

// #define    cmdChangePlantSummary_FirstPlantSummary        0x00500000
// #define    cmdChangePlantSummary_LastPlantSummary        0x00507FFF
// #define    cmdDuplicatePlantSummary_FirstPlantSummary        0x00508000
// #define    cmdDuplicatePlantSummary_LastPlantSummary        0x0050FFFF
// #define    cmdDeletePlantSummary_FirstPlantSummary        0x00600000
// #define    cmdDeletePlantSummary_LastPlantSummary        0x00607FFF

//#pragma mark "Commands - Windows"
/* ------------------------------------------------------------
The Windows MENU
#s 3200..3999 */
// 3200..3209 for predefined windows eg. ‘Clock’, ‘Status’, ‘Filter’, …
#define cmdClock 3200
#define cmdErrors 3201
#define cmdStatus 3202
#define cmdIODisplayWindow 3203

// 3210..3299 for Table and View windows
#define    cmdFirstWindow 3210

//#pragma mark "ID's"
/* ------------------------------------------------------------
The PlantCoordinator Process ID and related commands/actions
numbers 30000..32767 */

/* ------------------------------------------------------------
Process IDs and MultiCharacteristics' extra characteristic IDs
numbers 32 768..557 056 (0x8000..0x8 FFFF)
See "GroundVolumeTypes.h" */

/* ------------------------------------------------------------
Plant (Process) ID's
numbers 589 824.. 622 592 (0x9 000..0x9 7FFF)
See "GroundVolumeTypes.h" */

/* ------------------------------------------------------------
Plant (Characteristic) ID's
numbers 1 048 576..1 605 631 (0x10 0000..0x18 7FFF)
See "GroundVolumeTypes.h" */

#if defined ROOTMAP_ISCOMMAND_FUNCTIONS
namespace rootmap
{

    // __IN RANGE
    // Check if the Command is in the View menu's ‘Scoreboard’ gvt range
    Boolean IsCommand_Scoreboard(long int theCmd, long int * volume);

    // Check if the Command is in the Table menu's ‘New Table’ gvt range
    Boolean IsCommand_NewTable(long int theCmd, long int * volume);

    // Check if the Command is a Plant Characteristic Summary
    Boolean IsCommand_PlantCharacteristic(long int theCmd, long int * volume);
    Boolean IsCommand_Plant(long theCmd);

    // Check if the Command is a Process ID or PMXt's extra Characteristic ID
    Boolean IsCommand_ProcessCharacteristic(long int theCmd, long int * volume);

} /* namespace rootmap */
#endif // #if defined ROOTMAP_ISCOMMAND_FUNCTIONS

#endif // #ifndef CropSimCommands_H
