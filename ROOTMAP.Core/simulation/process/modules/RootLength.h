#ifndef RootLength_H
#define RootLength_H
// ***********************************************************
//RootLength.h  This routine has been set up for ease of tracking root
//length of different root classes
//
// Author:             Vanessa Dunbabin
// Initial Development:     May 2000
// Latest Revision:         May 2000 VMD
//************************************************************

//other files whose definitions we use in this file
#include "Process.h"
//#include "DoubleCoordinates.h"

namespace rootmap
{

#define ROOTLENGTH_HIGH_NUMBER 1
#define COMBINED_LENGTH_NUMBER1 2
#define TAP_LENGTH_NUMBER1 3
#define FIRST_LENGTH_NUMBER1 4
#define SECOND_LENGTH_NUMBER1 5
#define THIRD_LENGTH_NUMBER1 6
#define MODEL_OUTPUT_NUMBER1 7
#define COMBINED_LENGTH_NUMBER2 8
#define TAP_LENGTH_NUMBER2 9
#define FIRST_LENGTH_NUMBER2 10
#define SECOND_LENGTH_NUMBER2 11
#define THIRD_LENGTH_NUMBER2 12
#define COMBINED_LENGTH_NUMBER3 13
#define TAP_LENGTH_NUMBER3 14
#define FIRST_LENGTH_NUMBER3 15
#define SECOND_LENGTH_NUMBER3 16
#define THIRD_LENGTH_NUMBER3 17
#define COMBINED_LENGTH_NUMBER4 18
#define TAP_LENGTH_NUMBER4 19
#define FIRST_LENGTH_NUMBER4 20
#define SECOND_LENGTH_NUMBER4 21
#define THIRD_LENGTH_NUMBER4 22
#define MODEL_OUTPUT_NUMBER2 23
#define MODEL_OUTPUT_NUMBER3 24
#define MODEL_OUTPUT_NUMBER4 25

class RootLength : public Process {
public:
    //TCL_DECLARE_CLASS
    
    
    private:
    long int time_between_wakings;
    
    public:
    RootLength();    // called implicitly by new_by_name(), which is used
                    // to create processes which override functionality
    ~RootLength() {};  // pWater destructor - use default
    
    virtual long int GetPeriodicDelay(long int /*characteristic_number*/) { return (time_between_wakings); }
    virtual long int DoNormalWakeUp(ProcessActionDescriptor *action);
    virtual long int Register(ProcessActionDescriptor *action);
    virtual long int Initialise(ProcessActionDescriptor *action);
    virtual long int DoScoreboardPlantSummaryRegistered(ProcessActionDescriptor *action);
    
    long int TrackRootLength(ProcessActionDescriptor *action);
    
    long rootlength_first_plant_summary_index;


    // the number of plants, branch orders, and summaries
    long rootlength_number_of_plants;
    long rootlength_number_of_branch_orders;
    
    long int RootLength_high_Index;
    long int Branch_Lag_Index;
    long int Model_Output_Index1;
    long int Model_Output_Index2;
    long int Model_Output_Index3;
    long int Model_Output_Index4;
    long int Combined_Length_Index1;
    long int Tap_Length_Index1;
    long int First_Length_Index1;
    long int Second_Length_Index1;
    long int Third_Length_Index1;
    long int Combined_Length_Index2;
    long int Tap_Length_Index2;
    long int First_Length_Index2;
    long int Second_Length_Index2;
    long int Third_Length_Index2;
    long int Combined_Length_Index3;
    long int Tap_Length_Index3;
    long int First_Length_Index3;
    long int Second_Length_Index3;
    long int Third_Length_Index3;
    long int Combined_Length_Index4;
    long int Tap_Length_Index4;
    long int First_Length_Index4;
    long int Second_Length_Index4;
    long int Third_Length_Index4;
    
    double_cs tap1, first1, second1, third1, tap2, first2, second2, third2, tap3, first3, second3, third3;
    double_cs tap4, first4, second4, third4, tap5, first5, second5, third5, tap6, first6, second6, third6;
    double_cs tap7, first7, second7, third7, tap8, first8, second8, third8, tap9, first9, second9, third9;
    double_cs tap10, first10, second10, third10, tap11, first11, second11, third11, tap12, first12, second12, third12;
    double_cs tap13, first13, second13, third13, tap14, first14, second14, third14, tap15, first15, second15, third15;
    double_cs tap16, first16, second16, third16, tap17, first17, second17, third17, tap18, first18, second18, third18;
    double_cs tap19, first19, second19, third19, tap20, first20, second20, third20;
    
    double_cs total_tap_length1, total_first_length1, total_second_length1, total_third_length1;
    double_cs total_tap_length2, total_first_length2, total_second_length2, total_third_length2;
    double_cs total_tap_length3, total_first_length3, total_second_length3, total_third_length3;
    double_cs total_tap_length4, total_first_length4, total_second_length4, total_third_length4;
};

} // namespace rootmap

#endif // #ifndef RootLength_H
