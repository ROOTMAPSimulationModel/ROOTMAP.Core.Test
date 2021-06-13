#include "BoostPrecomp.h"

// *****************************************************************
// RootLength.cp This routine has been set up for ease of tracking root length of different root classes
//
// Author:             Vanessa Dunbabin
// Initial Development:     May 2000
// Latest Revision:         May 2000 VMD

#include "Process.h"
#include "RootLength.h"
#include "PlantSummaryRegistration.h"
#include "Utilities.h"
#include "CVoidPtrArrayIterator.h"
#include <stdlib.h>
#include <fp.h>
#include "LHorizonArray.h"
#include "simulation/common/BoostPrecomp.h"

TCL_DEFINE_CLASS_D1(RootLength, Process);

RootLength::RootLength()
: Process()
{
    time_between_wakings = PROCESS_HOURS(2);

    TCL_END_CONSTRUCTOR
}

/* ***************************************************************************
*/
long int RootLength::Register(ProcessActionDescriptor *action)
{
    // We start by declaring the object we will send to the
    // PlantCoordinator as request data
    Use_ProcessCoordinator;
    PlantSummaryHelper helper(processcoordinator, 0);
    PlantSummaryRegistration *psr = helper.MakePlantSummaryRegistration(action);
    // THIS is where the module chooses to fill in the blanks.
    psr->RegisterByPerBox(true);
    psr->RegisterAllPlantCombos();
    psr->RegisterAllBranchOrderCombos();
    psr->RegisterSummaryType(srRootLength);
    psr->RegisterModule((Process*)this);
    // try to send it off. If it wasn't successful, the PlantCoordinator is missing
    if (helper.SendPlantSummaryRegistration(psr, action, this))
    {
        // should return a partial failure error here
    }

    return kNoError;
}

long int RootLength::Initialise(ProcessActionDescriptor *action)
{
    Use_ReturnValue;
    Use_Time;
    Use_Scoreboard;
    Use_ScoreboardCoordinator;
    Use_ProcessCoordinator;
    Use_PostOffice;
    
    long num_x, num_y, num_z;                    // number of layers in each dimension
    Get_Number_Of_Layers(num_x,num_y,num_z);
//    long int x,y,z;
    tap1= first1= second1= third1= tap2= first2= second2= third2= tap3= first3= second3= third3= 0;
    tap4= first4= second4= third4= tap5= first5= second5= third5= tap6= first6= second6= third6= 0;
    tap7= first7= second7= third7= tap8= first8= second8= third8= tap9= first9= second9= third9= 0;
    tap10= first10= second10= third10= tap11= first11= second11= third11= tap12= first12= second12= third12= 0;
    tap13= first13= second13= third13= tap14= first14= second14= third14= tap15= first15= second15= third15= 0;
    tap16= first16= second16= third16= tap17= first17= second17= third17= tap18= first18= second18= third18= 0;
    tap19= first19= second19= third19= tap20= first20= second20= third20 = 0;
    
    total_tap_length1 = 0;
    total_first_length1 = 0; 
    total_second_length1 = 0; 
    total_third_length1 = 0;
    total_tap_length2 = 0; total_first_length2 = 0; total_second_length2 = 0; total_third_length2 = 0;
    total_tap_length3 = 0; total_first_length3 = 0; total_second_length3 = 0; total_third_length3 = 0;
    total_tap_length4 = 0; total_first_length4 = 0; total_second_length4 = 0; total_third_length4 = 0;
    tap19= first19= second19= third19= tap20= first20= second20= third20 = 0;
    
    RootLength_high_Index = GetCharacteristicIndex(ROOTLENGTH_HIGH_NUMBER);
    Model_Output_Index1 = GetCharacteristicIndex(MODEL_OUTPUT_NUMBER1);
    Model_Output_Index2 = GetCharacteristicIndex(MODEL_OUTPUT_NUMBER2);
    Model_Output_Index3 = GetCharacteristicIndex(MODEL_OUTPUT_NUMBER3);
    Model_Output_Index4 = GetCharacteristicIndex(MODEL_OUTPUT_NUMBER4);
    
    Combined_Length_Index1 = GetCharacteristicIndex(COMBINED_LENGTH_NUMBER1);
    Tap_Length_Index1 = GetCharacteristicIndex(TAP_LENGTH_NUMBER1);
    First_Length_Index1 = GetCharacteristicIndex(FIRST_LENGTH_NUMBER1);
    Second_Length_Index1 = GetCharacteristicIndex(SECOND_LENGTH_NUMBER1);
    Third_Length_Index1 = GetCharacteristicIndex(THIRD_LENGTH_NUMBER1);
    Combined_Length_Index2 = GetCharacteristicIndex(COMBINED_LENGTH_NUMBER2);
    Tap_Length_Index2 = GetCharacteristicIndex(TAP_LENGTH_NUMBER2);
    First_Length_Index2 = GetCharacteristicIndex(FIRST_LENGTH_NUMBER2);
    Second_Length_Index2 = GetCharacteristicIndex(SECOND_LENGTH_NUMBER2);
    Third_Length_Index2 = GetCharacteristicIndex(THIRD_LENGTH_NUMBER2);
    Combined_Length_Index3 = GetCharacteristicIndex(COMBINED_LENGTH_NUMBER3);
    Tap_Length_Index3 = GetCharacteristicIndex(TAP_LENGTH_NUMBER3);
    First_Length_Index3 = GetCharacteristicIndex(FIRST_LENGTH_NUMBER3);
    Second_Length_Index3 = GetCharacteristicIndex(SECOND_LENGTH_NUMBER3);
    Third_Length_Index3 = GetCharacteristicIndex(THIRD_LENGTH_NUMBER3);
    Combined_Length_Index4 = GetCharacteristicIndex(COMBINED_LENGTH_NUMBER4);
    Tap_Length_Index4 = GetCharacteristicIndex(TAP_LENGTH_NUMBER4);
    First_Length_Index4 = GetCharacteristicIndex(FIRST_LENGTH_NUMBER4);
    Second_Length_Index4 = GetCharacteristicIndex(SECOND_LENGTH_NUMBER4);
    Third_Length_Index4 = GetCharacteristicIndex(THIRD_LENGTH_NUMBER4);
        
    Str255 branch_lag_name = "\pBranch Lag Time 0";
    Branch_Lag_Index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume(branch_lag_name);
    
    /*
    for (y=1; y<=num_y; y++)
    {
        for (x=1; x<=num_x; x++)
        {
            for (z=1; z<=num_z; z++)
            {

                if (z > 6)
                {
                    Set_Concentration_Of_Box(Branch_Lag_Index, 216, x,y,z);
                }
                else
                {
                    Set_Concentration_Of_Box(Branch_Lag_Index, 168, x,y,z);
                }

            } //for (z=1; z<=num_x; z++)
        } //for (x=1; x<=num_x; x++)
    } //for (y=1; y<=num_y; y++) */
    
    // Otherwise, set the alarm clock
    SetPeriodicWaking(action);

    // Do the other, default stuff
    return (Process::Initialise(action));
}

/********************************************************************************************/

long int RootLength::DoNormalWakeUp(ProcessActionDescriptor *action)
{
    Use_ReturnValue;
    Use_Time;


    return_value = TrackRootLength(action);

    return_value |= Process::DoNormalWakeUp(action);
    return (return_value);
}

/********************************************************************************************/

long int RootLength::DoScoreboardPlantSummaryRegistered(ProcessActionDescriptor *action)
{
    Use_PlantSummaryRegistration;

    //TODO: This stuff has become bogus since SharedAttributes hijacked
    //      PlantSummaries.  This response needs replacing with something more
    //      helpful.
    rootlength_first_plant_summary_index = psr->GetStartIndex();
    rootlength_number_of_plants = psr->GetNumberOfPlants();
    rootlength_number_of_branch_orders = psr->GetNumberOfBranchOrders();

//    TCLForgetObject(psr);

    return (Process::DoScoreboardPlantSummaryRegistered(action));
}

/********************************************************************************************/

long int RootLength::TrackRootLength(ProcessActionDescriptor *action)
{    
    Use_ReturnValue;
    Use_Time;
    Use_Scoreboard;
    Use_ScoreboardCoordinator;
    Use_ProcessCoordinator;
    Use_PostOffice;
    
    long int x,y,z,plant_counter;
    long int root_length_summary_index, branch_order;
    double_cs tap_root_length = 0, first_root_length = 0, second_root_length = 0, total_root_length = 0;
    double_cs tap_rl_upper = 0, tap_rl_lower = 0, first_rl_upper = 0, first_rl_lower = 0, second_rl_upper = 0, second_rl_lower = 0;
    double_cs third_rl_upper = 0, third_rl_lower = 0, third_root_length = 0;
    double_cs combined_length = 0, local_tap_length = 0, local_first_length = 0, local_second_length = 0, local_third_length = 0;
    double_cs RLDy1 = 0, RLDy2 = 0, RLDy3 = 0, RLDy4 = 0, RLDy5 = 0, RLDy6 = 0, RLDy7 = 0, RLDy8 = 0;
    double_cs /*box_height, box_width, box_depth,*/ box_volume = 0;
    long num_x, num_y, num_z;                    // number of layers in each dimension
    Get_Number_Of_Layers(num_x,num_y,num_z); // How many boxes do we have?
    
    double_cs total_root_length_high1=0, total_root_length_high2=0, total_root_length_high3=0, total_root_length_high4=0;
    
    total_tap_length1 = 0, total_first_length1 = 0, total_second_length1 = 0, total_third_length1 = 0;
    total_tap_length2 = 0, total_first_length2 = 0, total_second_length2 = 0, total_third_length2 = 0;
    total_tap_length3 = 0, total_first_length3 = 0, total_second_length3 = 0, total_third_length3 = 0;
    total_tap_length4 = 0, total_first_length4 = 0, total_second_length4 = 0, total_third_length4 = 0;

    for (y=1; y<=num_y; y++)
    {
        for (z=1; z<=num_z; z++)
        {
            for (x=1; x<=num_x; x++)
            {    

                box_volume = Get_Box_Volume(x,y,z);
                root_length_summary_index = rootlength_first_plant_summary_index;
                combined_length = 0, local_first_length = 0, local_second_length = 0, local_third_length = 0, local_tap_length = 0;
                
                plant_counter = 1;
                while( plant_counter <= rootlength_number_of_plants)
                {
                    branch_order = 0;
                    combined_length = 0, local_first_length = 0, local_second_length = 0, local_third_length = 0, local_tap_length = 0;
                    
                    while (branch_order < rootlength_number_of_branch_orders)
                    {
                        total_root_length += scoreboard->GetCharacteristicValue(root_length_summary_index, x,y,z);
                        combined_length += scoreboard->GetCharacteristicValue(root_length_summary_index, x,y,z);

                        if (branch_order == 0)
                        {
                            tap_root_length += scoreboard->GetCharacteristicValue(root_length_summary_index, x,y,z);
                            local_tap_length += scoreboard->GetCharacteristicValue(root_length_summary_index, x,y,z);
                        }
                        if (branch_order == 1)
                        {
                            first_root_length += scoreboard->GetCharacteristicValue(root_length_summary_index, x,y,z);
                            local_first_length += scoreboard->GetCharacteristicValue(root_length_summary_index, x,y,z);
                        }
                        if (branch_order == 2)
                        {
                            second_root_length += scoreboard->GetCharacteristicValue(root_length_summary_index, x,y,z);
                            local_second_length += scoreboard->GetCharacteristicValue(root_length_summary_index, x,y,z);
                        }
                        if (branch_order == 3)
                        {
                            third_root_length += scoreboard->GetCharacteristicValue(root_length_summary_index, x,y,z);
                            local_third_length += scoreboard->GetCharacteristicValue(root_length_summary_index, x,y,z);
                        }

                        root_length_summary_index++;
                        branch_order++;
                    } //while (branch_order < rootlength_number_of_branch_orders)
                    
                    switch (plant_counter)
                    {
                        case 1:
                        Set_Concentration_Of_Box(Combined_Length_Index1, combined_length, x,y,z);
                        Set_Concentration_Of_Box(Tap_Length_Index1, local_tap_length, x,y,z);
                        Set_Concentration_Of_Box(First_Length_Index1, local_first_length, x,y,z);
                        Set_Concentration_Of_Box(Second_Length_Index1, local_second_length, x,y,z);
                        Set_Concentration_Of_Box(Third_Length_Index1, local_third_length, x,y,z);
                        total_root_length_high1 += local_second_length + local_third_length;
                        total_tap_length1 += local_tap_length;
                        total_first_length1 += local_first_length;
                        total_second_length1 += local_second_length;
                        total_third_length1 += local_third_length;
                        break;
                        case 2:
                        Set_Concentration_Of_Box(Combined_Length_Index2, combined_length, x,y,z);
                        Set_Concentration_Of_Box(Tap_Length_Index2, local_tap_length, x,y,z);
                        Set_Concentration_Of_Box(First_Length_Index2, local_first_length, x,y,z);
                        Set_Concentration_Of_Box(Second_Length_Index2, local_second_length, x,y,z);
                        Set_Concentration_Of_Box(Third_Length_Index2, local_third_length, x,y,z);
                        total_root_length_high2 += local_second_length + local_third_length;
                        total_tap_length2 += local_tap_length;
                        total_first_length2 += local_first_length;
                        total_second_length2 += local_second_length;
                        total_third_length2 += local_third_length;
                        break;
                        case 3:
                        Set_Concentration_Of_Box(Combined_Length_Index3, combined_length, x,y,z);
                        Set_Concentration_Of_Box(Tap_Length_Index3, local_tap_length, x,y,z);
                        Set_Concentration_Of_Box(First_Length_Index3, local_first_length, x,y,z);
                        Set_Concentration_Of_Box(Second_Length_Index3, local_second_length, x,y,z);
                        Set_Concentration_Of_Box(Third_Length_Index3, local_third_length, x,y,z);
                        total_root_length_high3 += local_second_length + local_third_length;
                        total_tap_length3 += local_tap_length;
                        total_first_length3 += local_first_length;
                        total_second_length3 += local_second_length;
                        total_third_length3 += local_third_length;
                        break;
                        case 4:
                        Set_Concentration_Of_Box(Combined_Length_Index4, combined_length, x,y,z);
                        Set_Concentration_Of_Box(Tap_Length_Index4, local_tap_length, x,y,z);
                        Set_Concentration_Of_Box(First_Length_Index4, local_first_length, x,y,z);
                        Set_Concentration_Of_Box(Second_Length_Index4, local_second_length, x,y,z);
                        Set_Concentration_Of_Box(Third_Length_Index4, local_third_length, x,y,z);
                        total_root_length_high4 += local_second_length + local_third_length;
                        total_tap_length4 += local_tap_length;
                        total_first_length4 += local_first_length;
                        total_second_length4 += local_second_length;
                        total_third_length4 += local_third_length;
                        break;
                    } //switch (plant_counter)
                    plant_counter++;
                } //while( i < rootlength_number_of_plants)
                
                switch (z)
                {
                    case 1:        RLDy1 += combined_length; break;
                    case 2:        RLDy1 += combined_length; break;
                    case 3:        RLDy2 += combined_length; break;
                    case 4:        RLDy2 += combined_length; break;
                    case 5:        RLDy3 += combined_length; break;
                    case 6:        RLDy4 += combined_length; break;
                    case 7:        RLDy5 += combined_length; break;
                    case 8:        RLDy6 += combined_length; break;
                    case 9:        RLDy6 += combined_length; break;
                    case 10:    RLDy7 += combined_length; break;
                    case 11:    RLDy7 += combined_length; break;
                    case 12:    RLDy8 += combined_length; break;
                    case 13:    RLDy8 += combined_length; break;
                    default:                              break;
                }
                

            }//for (x=1; x<=num_x; x++)
        }//for (z=1; z<=num_x; z++)
    }//for (y=1; y<=num_x; y++)
    
                        if ((time < 1728000))
                        {
                            tap4 = tap_root_length;
                            first4 = first_root_length;
                            second4 = second_root_length;
                            third4 = third_root_length;
                            Set_Concentration_Of_Box(Model_Output_Index1, total_tap_length1, 1,1,1);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_first_length1, 2,1,1);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_second_length1, 3,1,1);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_third_length1, 4,1,1);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_tap_length2, 1,1,1);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_first_length2, 2,1,1);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_second_length2, 3,1,1);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_third_length2, 4,1,1);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_tap_length3, 1,1,1);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_first_length3, 2,1,1);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_second_length3, 3,1,1);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_third_length3, 4,1,1);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_tap_length4, 1,1,1);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_first_length4, 2,1,1);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_second_length4, 3,1,1);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_third_length4, 4,1,1);
                        }
                        if ((time < 2160000))
                        {
                            tap5 = tap_root_length;
                            first5 = first_root_length;
                            second5 = second_root_length;
                            third5 = third_root_length;
                            Set_Concentration_Of_Box(Model_Output_Index1, total_tap_length1, 1,1,2);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_first_length1, 2,1,2);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_second_length1, 3,1,2);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_third_length1, 4,1,2);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_tap_length2, 1,1,2);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_first_length2, 2,1,2);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_second_length2, 3,1,2);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_third_length2, 4,1,2);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_tap_length3, 1,1,2);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_first_length3, 2,1,2);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_second_length3, 3,1,2);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_third_length3, 4,1,2);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_tap_length4, 1,1,2);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_first_length4, 2,1,2);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_second_length4, 3,1,2);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_third_length4, 4,1,2);
                        }
                        if ((time < 2592000))
                        {
                            tap6 = tap_root_length;
                            first6 = first_root_length;
                            second6 = second_root_length;
                            third6 = third_root_length;
                            Set_Concentration_Of_Box(Model_Output_Index1, total_tap_length1, 1,1,3);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_first_length1, 2,1,3);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_second_length1, 3,1,3);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_third_length1, 4,1,3);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_tap_length2, 1,1,3);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_first_length2, 2,1,3);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_second_length2, 3,1,3);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_third_length2, 4,1,3);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_tap_length3, 1,1,3);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_first_length3, 2,1,3);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_second_length3, 3,1,3);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_third_length3, 4,1,3);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_tap_length4, 1,1,3);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_first_length4, 2,1,3);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_second_length4, 3,1,3);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_third_length4, 4,1,3);
                        }
                        if ((time < 3024000))
                        {
                            tap7 = tap_root_length;
                            first7 = first_root_length;
                            second7 = second_root_length;
                            third7 = third_root_length;
                            Set_Concentration_Of_Box(Model_Output_Index1, total_tap_length1, 1,1,4);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_first_length1, 2,1,4);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_second_length1, 3,1,4);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_third_length1, 4,1,4);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_tap_length2, 1,1,4);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_first_length2, 2,1,4);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_second_length2, 3,1,4);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_third_length2, 4,1,4);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_tap_length3, 1,1,4);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_first_length3, 2,1,4);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_second_length3, 3,1,4);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_third_length3, 4,1,4);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_tap_length4, 1,1,4);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_first_length4, 2,1,4);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_second_length4, 3,1,4);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_third_length4, 4,1,4);
                        }
                        if ((time < 3456000))
                        {
                            tap8 = tap_root_length;
                            first8 = first_root_length;
                            second8 = second_root_length;
                            third8 = third_root_length;
                            Set_Concentration_Of_Box(Model_Output_Index1, total_tap_length1, 1,1,5);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_first_length1, 2,1,5);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_second_length1, 3,1,5);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_third_length1, 4,1,5);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_tap_length2, 1,1,5);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_first_length2, 2,1,5);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_second_length2, 3,1,5);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_third_length2, 4,1,5);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_tap_length3, 1,1,5);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_first_length3, 2,1,5);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_second_length3, 3,1,5);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_third_length3, 4,1,5);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_tap_length4, 1,1,5);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_first_length4, 2,1,5);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_second_length4, 3,1,5);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_third_length4, 4,1,5);
                        }
                        if ((time < 3888000))
                        {
                            tap9 = tap_root_length;
                            first9 = first_root_length;
                            second9 = second_root_length;
                            third9 = third_root_length;
                            Set_Concentration_Of_Box(Model_Output_Index1, total_tap_length1, 1,1,6);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_first_length1, 2,1,6);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_second_length1, 3,1,6);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_third_length1, 4,1,6);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_tap_length2, 1,1,6);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_first_length2, 2,1,6);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_second_length2, 3,1,6);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_third_length2, 4,1,6);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_tap_length3, 1,1,6);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_first_length3, 2,1,6);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_second_length3, 3,1,6);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_third_length3, 4,1,6);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_tap_length4, 1,1,6);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_first_length4, 2,1,6);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_second_length4, 3,1,6);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_third_length4, 4,1,6);
                        }
                        if ((time < 4320000))
                        {
                            tap10 = tap_root_length;
                            first10 = first_root_length;
                            second10 = second_root_length;
                            third10 = third_root_length;
                            Set_Concentration_Of_Box(Model_Output_Index1, total_tap_length1, 1,1,7);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_first_length1, 2,1,7);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_second_length1, 3,1,7);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_third_length1, 4,1,7);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_tap_length2, 1,1,7);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_first_length2, 2,1,7);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_second_length2, 3,1,7);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_third_length2, 4,1,7);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_tap_length3, 1,1,7);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_first_length3, 2,1,7);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_second_length3, 3,1,7);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_third_length3, 4,1,7);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_tap_length4, 1,1,7);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_first_length4, 2,1,7);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_second_length4, 3,1,7);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_third_length4, 4,1,7);
                        }
                        if ((time < 4752000))
                        {
                            tap11 = tap_root_length;
                            first11 = first_root_length;
                            second11 = second_root_length;
                            third11 = third_root_length;
                            Set_Concentration_Of_Box(Model_Output_Index1, total_tap_length1, 1,1,8);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_first_length1, 2,1,8);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_second_length1, 3,1,8);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_third_length1, 4,1,8);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_tap_length2, 1,1,8);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_first_length2, 2,1,8);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_second_length2, 3,1,8);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_third_length2, 4,1,8);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_tap_length3, 1,1,8);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_first_length3, 2,1,8);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_second_length3, 3,1,8);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_third_length3, 4,1,8);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_tap_length4, 1,1,8);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_first_length4, 2,1,8);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_second_length4, 3,1,8);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_third_length4, 4,1,8);
                        }
                        if ((time < 5616000))
                        {
                            tap12 = tap_root_length;
                            first12 = first_root_length;
                            second12 = second_root_length;
                            third12 = third_root_length;
                            Set_Concentration_Of_Box(Model_Output_Index1, total_tap_length1, 1,1,9);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_first_length1, 2,1,9);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_second_length1, 3,1,9);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_third_length1, 4,1,9);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_tap_length2, 1,1,9);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_first_length2, 2,1,9);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_second_length2, 3,1,9);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_third_length2, 4,1,9);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_tap_length3, 1,1,9);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_first_length3, 2,1,9);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_second_length3, 3,1,9);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_third_length3, 4,1,9);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_tap_length4, 1,1,9);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_first_length4, 2,1,9);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_second_length4, 3,1,9);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_third_length4, 4,1,9);
                        }
                        if ((time < 6912000))
                        {
                            tap13 = tap_root_length;
                            first13 = first_root_length;
                            second13 = second_root_length;
                            third13 = third_root_length;
                            Set_Concentration_Of_Box(Model_Output_Index1, total_tap_length1, 1,1,10);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_first_length1, 2,1,10);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_second_length1, 3,1,10);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_third_length1, 4,1,10);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_tap_length2, 1,1,10);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_first_length2, 2,1,10);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_second_length2, 3,1,10);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_third_length2, 4,1,10);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_tap_length3, 1,1,10);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_first_length3, 2,1,10);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_second_length3, 3,1,10);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_third_length3, 4,1,10);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_tap_length4, 1,1,10);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_first_length4, 2,1,10);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_second_length4, 3,1,10);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_third_length4, 4,1,10);
                        }
                        if ((time < 7862400))
                        {
                            tap14 = tap_root_length;
                            first14 = first_root_length;
                            second14 = second_root_length;
                            third14 = third_root_length;
                            Set_Concentration_Of_Box(Model_Output_Index1, total_tap_length1, 1,1,11);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_first_length1, 2,1,11);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_second_length1, 3,1,11);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_third_length1, 4,1,11);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_tap_length2, 1,1,11);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_first_length2, 2,1,11);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_second_length2, 3,1,11);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_third_length2, 4,1,11);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_tap_length3, 1,1,11);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_first_length3, 2,1,11);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_second_length3, 3,1,11);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_third_length3, 4,1,11);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_tap_length4, 1,1,11);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_first_length4, 2,1,11);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_second_length4, 3,1,11);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_third_length4, 4,1,11);
                        }
                        if ((time < 9072000))
                        {
                            tap15 = tap_root_length;
                            first15 = first_root_length;
                            second15 = second_root_length;
                            third15 = third_root_length;
                            Set_Concentration_Of_Box(Model_Output_Index1, total_tap_length1, 1,1,12);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_first_length1, 2,1,12);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_second_length1, 3,1,12);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_third_length1, 4,1,12);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_tap_length2, 1,1,12);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_first_length2, 2,1,12);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_second_length2, 3,1,12);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_third_length2, 4,1,12);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_tap_length3, 1,1,12);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_first_length3, 2,1,12);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_second_length3, 3,1,12);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_third_length3, 4,1,12);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_tap_length4, 1,1,12);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_first_length4, 2,1,12);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_second_length4, 3,1,12);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_third_length4, 4,1,12);
                        }
                        if ((time < 9936000))
                        {
                            tap16 = tap_root_length;
                            first16 = first_root_length;
                            second16 = second_root_length;
                            third16 = third_root_length;
                            Set_Concentration_Of_Box(Model_Output_Index1, total_tap_length1, 1,1,13);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_first_length1, 2,1,13);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_second_length1, 3,1,13);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_third_length1, 4,1,13);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_tap_length2, 1,1,13);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_first_length2, 2,1,13);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_second_length2, 3,1,13);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_third_length2, 4,1,13);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_tap_length3, 1,1,13);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_first_length3, 2,1,13);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_second_length3, 3,1,13);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_third_length3, 4,1,13);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_tap_length4, 1,1,13);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_first_length4, 2,1,13);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_second_length4, 3,1,13);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_third_length4, 4,1,13);
                        }
                        if ((time < 10800000))
                        {
                            tap17 = tap_root_length;
                            first17 = first_root_length;
                            second17 = second_root_length;
                            third17 = third_root_length;
                            Set_Concentration_Of_Box(Model_Output_Index1, total_tap_length1, 1,1,14);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_first_length1, 2,1,14);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_second_length1, 3,1,14);
                            Set_Concentration_Of_Box(Model_Output_Index1, total_third_length1, 4,1,14);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_tap_length2, 1,1,14);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_first_length2, 2,1,14);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_second_length2, 3,1,14);
                            Set_Concentration_Of_Box(Model_Output_Index2, total_third_length2, 4,1,14);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_tap_length3, 1,1,14);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_first_length3, 2,1,14);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_second_length3, 3,1,14);
                            Set_Concentration_Of_Box(Model_Output_Index3, total_third_length3, 4,1,14);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_tap_length4, 1,1,14);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_first_length4, 2,1,14);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_second_length4, 3,1,14);
                            Set_Concentration_Of_Box(Model_Output_Index4, total_third_length4, 4,1,14);
                        }

    RLDy1 = RLDy1 / 21600;
    RLDy2 = RLDy2 / 21600;
    RLDy3 = RLDy3 / 21600;
    RLDy4 = RLDy4 / 21600;
    RLDy5 = RLDy5 / 21600;
    RLDy6 = RLDy6 / 43200;
    RLDy7 = RLDy7 / 43200;
    RLDy8 = RLDy8 / 64800;
    Set_Concentration_Of_Box(RootLength_high_Index, (total_root_length_high1), 1,1,1);
    Set_Concentration_Of_Box(RootLength_high_Index, (total_root_length_high2), 2,1,1);
    Set_Concentration_Of_Box(RootLength_high_Index, (total_root_length_high3), 3,1,1);
    Set_Concentration_Of_Box(RootLength_high_Index, (total_root_length_high4), 4,1,1);

    
    
    return(return_value);    
} //long int RootLength::TrackRootLength(ProcessActionDescriptor *action)
    
    
    
    
    
    
    
