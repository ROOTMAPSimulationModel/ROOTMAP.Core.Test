

/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessTemplate.cpp
// Purpose:     Implementation of the ProcessTemplate class
// Created:     22-03-2009
// Author:      RvH
// $Date: 2009-02-25 04:09:59 +0900 (Wed, 25 Feb 2009) $
// $Revision: 35 $
// Copyright:   ©2002-2009 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

// Typically, your Process derived class will be in the following location
#include "simulation/process/common/ProcessTemplate.h"

// Common requirements
#include "core/common/DoubleCoordinates.h"
#include "core/common/RmAssert.h"
#include "core/common/ExtraStuff.h"
#include "core/utility/Utility.h"


// Useful enough macro
#define LOG_ERROR_IF(s) if ( (s) ) { RootMapLogError(LOG_LINE "Encountered error in logic : " #s); }


namespace rootmap
{


// Put constants here as const variables, not preprocessor macros
const double SomeConstant = 1.00;
const int SomeOtherInteger = 1;

// Causes the logger static var to be initialised
RootMapLoggerDefinition(ProcessTemplate);

// wxWidgets stuff, so the process can be dynamically instantiated by name
IMPLEMENT_DYNAMIC_CLASS(ProcessTemplate, Process)

// Oh hai, i'm in ur constructor
ProcessTemplate::ProcessTemplate()
: mySharedAttributeOwner(NULL)
{
    RootMapLoggerInitialisation("rootmap.ProcessTemplate");
}

ProcessTemplate::~ProcessTemplate()
{
}


long int ProcessTemplate::Initialise(ProcessActionDescriptor *action)
{
    Use_ReturnValue;

#if defined PROCESSTEMPLATE_SAMPLE_INITIALISE
    Use_PostOffice;
    Use_ProcessCoordinator;
    Use_ScoreboardCoordinator;

    // Determine the characteristic indices of the water process module's characteristics
    Water_Content_Index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("My Characteristic");

    SetPeriodicDelay(kNormalMessage, myTimeBetweenWaking);
    Send_GeneralPurpose(action->GetTime()+myTimeBetweenWaking, this, NULL, kNormalMessage);

    return_value = InitialisePackets(action);

    PrepareSharedAttributes(action);

    Process::Initialise(action);
#endif defined PROCESSTEMPLATE_SAMPLE_INITIALISE

    return (return_value);

} // long int ProcessTemplate::Initialise(ProcessActionDescriptor *action)

void ProcessTemplate::PrepareSharedAttributes(ProcessActionDescriptor * action)
{
    SharedAttributeSearchHelper helper(action->GetSharedAttributeManager(), mySharedAttributeOwner);

    // "Xtic" is our example characteristic. "Plant Xtic" is a scoreboard
    // shared attribute that is our Xtic varied by Plant
    SharedAttribute * myXticPlant = helper.SearchForAttribute("Plant Xtic1", "Plant");
    // "RootOrder Xtic" is a scoreboard shared attribute that is our Xtic varied by RootOrder
    SharedAttribute * myXticRO    = helper.SearchForAttribute("RootOrder Xtic1", "RootOrder");
    // "Total Xtic" is a global shared attribute with no variations
    SharedAttribute * myXticTotal = helper.SearchForAttribute("Total Xtic1");

    //
    // Load expected SharedAttributes
    saXticPerPlantIndex = helper.SearchForClusterIndex("Plant Xtic1","Plant",NULL);

    std::vector<RmString> variation_names;
    variation_names.push_back("Plant");
    variation_names.push_back("RootOrder");
    saComplexPerPlantPerRootOrderIndex = helper.SearchForClusterIndex("Xtic2","Plant",variation_names);

    // search for root length, per plant.
    saRootLengthPerPlantIndex = helper.SearchForClusterIndex("Root Length Wrap None","Plant",NULL);

    // search for root length, per Plant+RootOrder
    saRootLengthPerPlantPerRootOrderIndex = helper.SearchForClusterIndex("Root Length Wrap None","Plant",variation_names);

    saRootRadiusPerPlantPerRootOrderIndex = helper.SearchForClusterIndex("Root Radius","Plant",variation_names);

    // if any SharedAttributes were not found, log an error
    
}




/* ************************************************************************** */

long int ProcessTemplate::StartUp(ProcessActionDescriptor *action)
{
    Use_Time;

    // If this process has never been sent a message to perform before, make this
    // the initial time - to avoid  having a humungous dT in any functions that
    // subsequently perform.
    if (time_of_previous_diffusion<0)
    {
        time_of_previous_diffusion = time;
    }

    return (Process::StartUp(action));
}

long int ProcessTemplate::DoGeneralPurpose(ProcessActionDescriptor *action)
{
    Use_ReturnValue;
    // we want to know who sent the wake up, so we can decide how to act.
    Use_Source;
    Use_Code;

    // now check if it is the water module
    if (source_module_id == itsWaterModuleID)
    {
        switch (code)
        {
        case kWaterDrained:
            return_value = LeachN(action);
            break;
        case kWaterRedistributed:
            return_value = MassFlowN(action);
            break;
        default:
            // log error or whatever
            break;
        } // switch (code)
    } // if (source_module_id == itsWaterModuleID)

    return (return_value);
}

/* DoExternalWakeUp
Called when a 'XWak' message is received. Sent by another process module,
to wake "this" one up to do some processing based on the other process'
action[s]. This default method does nothing, you'll need to override it
for your process to accept wake up calls from other process modules.

98.10.14 RvH  Fixed the whole lot up. It nearly looks the same, but isn't at all*/
long int ProcessTemplate::DoExternalWakeUp(ProcessActionDescriptor *action)
{
    Use_ReturnValue;

    // we want to know who sent the wake up, so we can decide how to act.
    Use_Source;
    Use_Data;
    Use_Time;
    Use_Code;

    long source_module_id = -1;
    if (source != NULL)
    {
        source_module_id = source->GetProcessID();
    }

    // now check if it is the water module
    if (source_module_id == itsWaterModuleID)
    {
        // new code for this is in DoGeneralPurpose(...)
#if defined NITRATEWATERPHOSPHORUS_SPECIALPROCESSDATA_EXAMPLE_CODE
        WaterNotification * water_notification = dynamic_cast<WaterNotification *>(data);
        if (NULL!=data)
        {
            switch (water_notification->notification_type)
            {
            case water_drained:
                return_value = LeachN(action);
                break;
            case water_redistributed:
                return_value = MassFlowN(action);
                break;
            default:
                // log error or whatever
                break;
            }
        }
#endif // #if defined NITRATEWATERPHOSPHORUS_SPECIALPROCESSDATA_EXAMPLE_CODE

#if defined NITRATEWATERPHOSPHORUS_OLD_NOTIFICATION_CODE
        if (data == NULL)
        {
            return_value = LeachN(action);  //A message has been send from PWater::Drain
        }
        if (data != NULL)
        {
            return_value = MassFlowN(action);  //A message has been sent from PWater::RedistributeWater
        }
#endif // #if defined NITRATEWATERPHOSPHORUS_OLD_NOTIFICATION_CODE
    }

    return (return_value);
}

/* ************************************************************************** */


long int ProcessTemplate::DoNormalWakeUp(ProcessActionDescriptor *action)
{
    Use_ReturnValue;
    Use_Time;

/*
    return_value = DiffuseN(action);
    return_value = UptakeN(action);
    return_value = RedistributeDots(action);
*/

    return_value |= Process::DoNormalWakeUp(action);
    return (return_value);
}

/* ************************************************************************** */

long int ProcessTemplate::DoScoreboardPlantSummaryRegistered(ProcessActionDescriptor *action)
{
    Use_PlantSummaryRegistration;

    nitrate_first_plant_summary_index = psr->GetStartIndex();
    nitrate_number_of_plants = psr->GetNumberOfPlants();
    nitrate_number_of_branch_orders = psr->GetNumberOfBranchOrders();

    return (Process::DoScoreboardPlantSummaryRegistered(action));
}


/* ************************************************************************** */
/* VMD 12/11/98 This routine redraws all the ProcessTemplate dots on the screen everytime the window is resized etc. */
void ProcessTemplate::DrawScoreboard(const DoubleRect & area, Scoreboard * scoreboard)
{
    long int counter = 0;

    while (counter < Total_Nitrates)
    {
        pNitrateArray->GetCoordinate(counter, pNitrate);

        if (!((pNitrate->x == 0) && (pNitrate->y == 0) && (pNitrate->z == 0)))
        {
            BEGIN_DRAWING
            if (pNitrate->s < m_NITRATE_PACKET_SIZE)
            {
                drawing.RemoveDot(*pNitrate); //remove a dot
            }
            else
            {
                // black is the prevailing colour
                drawing.DrawDot(*pNitrate);
            }
            END_DRAWING
        }

        counter +=1;
    }

}


/* **************************************************************************
*/

/* VMD 22/9/98 This routine is used to model the ongoing redistribution of ProcessTemplate by diffusion. Using Fick's law of
      diffusion, the amount of transfer between a box and all its surrounding boxes is calculated.  This
      routine is to be called often (every hour) so that the transfer in any particular time step is small, reducing the
      error of not simultaneously redistributing to all surrounding boxes.  It also reduces the error of not
      using a pure random motion routine. Transfer of ProcessTemplate between boxes by diffusion is so small it could be omitted.

   VMD 09/07/02 Tidied up the code a little */

long int ProcessTemplate::DiffuseN(ProcessActionDescriptor *action)
{

    Use_ReturnValue;
    Use_Time;
    Use_ScoreboardCoordinator;
    Use_ProcessCoordinator;
    Use_Scoreboard;
    Use_Boundaries;

    long num_x, num_y, num_z;                    // number of layers in each dimension
    Get_Number_Of_Layers(num_x,num_y,num_z);    // How many boxes do we have?


    // Variables used for calculating the displacement of each ProcessTemplate.
    double box_height,  box_depth, box_width;
    double box_height_next,  box_depth_next, box_width_next;
    double area_per_unit_dist;

    long int dT, x, y, z;

    dT = time - time_of_previous_diffusion;
    time_of_previous_diffusion = time;

    if (dT==0)
    {
        return kNoError;
    }
    RmAssert( dT>0, "Time delta since previous diffusion is negative");

    //Look at each box in the scoreboard and "equilibrate" it with its surrounding boxes.
    for (y=1; y<=num_y; ++y)
    {
        for (x=1; x<=num_x; ++x)
        {
            for (z=1; z<=num_z; ++z)
            {
                box_height = Get_Layer_Height(z);
                box_depth =  Get_Layer_Depth(y);
                box_width = Get_Layer_Width(x);
                //Calculate the transfer that would occur to the LHS box

                if (x != 1) // For now, do not transfer ProcessTemplate outside the scoreboard.
                {
                    box_width_next = Get_Layer_Width(x-1);
                    area_per_unit_dist = (box_height * box_depth * 2) / (box_width+box_width_next);
                    CalculateNitrateDiffusion(action, (x-1), y, z, area_per_unit_dist, x, y, z, dT);
                } //if (x != 1)

                //Calculate the transfer that would occur to the RHS box
                if (x != num_x) // For now, do not transfer ProcessTemplate outside the scoreboard.
                {
                    box_width_next = Get_Layer_Width(x+1);
                    area_per_unit_dist = (box_height * box_depth * 2) / (box_width+box_width_next);
                    CalculateNitrateDiffusion(action, (x+1), y, z, area_per_unit_dist, x, y, z, dT);
                } //if (x != num_x)

                //Calculate the transfer that would occur to the box in front
                if (y != 1) // For now, do not transfer ProcessTemplate outside the scoreboard.
                {
                    box_depth_next = Get_Layer_Depth(y-1);
                    area_per_unit_dist = (box_height * box_width * 2) / (box_depth+box_depth_next);
                    CalculateNitrateDiffusion(action, x, y-1, z, area_per_unit_dist, x, y, z, dT);
                } //if (y != 1)

                //Calculate the transfer that would occur to the box behind
                if (y != num_y) // For now, do not transfer ProcessTemplate outside the scoreboard.
                {
                    box_depth_next = Get_Layer_Depth(y+1);
                    area_per_unit_dist = (box_height * box_width * 2) / (box_depth+box_depth_next);
                    CalculateNitrateDiffusion(action, x, y+1, z, area_per_unit_dist, x, y, z, dT);
                } //if (y != num_y)

                //Calculate the transfer that would occur to the box on top
                if (z != 1) // For now, do not transfer ProcessTemplate outside the scoreboard.
                {
                    box_height_next = Get_Layer_Height(z-1);
                    area_per_unit_dist = (box_depth * box_width * 2) / (box_height+box_height_next);
                    CalculateNitrateDiffusion(action, x, y, z-1, area_per_unit_dist, x, y, z, dT);
                } //if (z != 1)

                //Calculate the transfer that would occur to the box beneath
                if (z != num_z)// For now, do not transfer ProcessTemplate outside the scoreboard.
                {
                    box_height_next = Get_Layer_Height(z+1);
                    area_per_unit_dist = (box_depth * box_width * 2) / (box_height+box_height_next);
                    CalculateNitrateDiffusion(action, x, y, z+1, area_per_unit_dist, x, y, z, dT);
                } //if (z != num_z)

            } //for (z=1; z<=num_z; ++z)
        } //for (x=1; x<=num_x; ++x)
    } //for (y=1; y<=num_y; ++y)

    return(return_value);

} //ProcessTemplate::DiffuseN(ProcessActionDescriptor *action)


/* **************************************************************************
*/
double ProcessTemplate::CalculateNitrateDiffusion(ProcessActionDescriptor *action, long int next_x, long int next_y, long int next_z, double area_per_unit_distance, long int this_x, long int this_y, long int this_z, long int dT)
{
    // MSA 10.11.10 Is this function ever used?

    Use_ReturnValue;
    Use_ScoreboardCoordinator;
    Use_ProcessCoordinator;
    Use_Scoreboard;
    Use_Boundaries;


    double box_volume, box_volume_next;
    double water_content, lowest_water_content, water_content_next,impedance_factor;
    double nitrate_concentration, nitrate_conc_next, nitrate_amount, nitrate_amount_next, delta_N;

    double DIFFUSION_COEFF_SOLN = 1.9e-5; //cm2/sec

    nitrate_amount = Get_Concentration_Of_Box(Nitrate_Amount_Index, this_x, this_y, this_z);
    water_content = Get_Concentration_Of_Box(Water_Content_Index, this_x, this_y, this_z);
    box_volume = Get_Box_Volume(this_x, this_y, this_z);
    nitrate_concentration = nitrate_amount / (water_content * box_volume);

    nitrate_amount_next = Get_Concentration_Of_Box(Nitrate_Amount_Index, next_x, next_y, next_z);
    water_content_next = Get_Concentration_Of_Box(Water_Content_Index, next_x, next_y, next_z);
    box_volume_next = Get_Box_Volume(next_x, next_y, next_z);
    nitrate_conc_next = nitrate_amount_next / (water_content_next * box_volume_next);

    // Only transfer ProcessTemplate to the next box if there is a concentration gradient in that direction.
    if (nitrate_conc_next < nitrate_concentration)
    {
        // To be conservative, will chose the water content of the drier box to calculate the impedance factor.
        // This is not a bad approximation as the transfer into a box will be limited by the driest portion that it
        // will have to traverse.
        lowest_water_content = water_content;
        if (water_content_next < water_content)
        {
            lowest_water_content = water_content_next;
        }

        //impedance factor taken from Nye and Tinker, 1977, pg 78 for chloride/ProcessTemplate in sandy loam soil
        impedance_factor = (3.129 * pow(lowest_water_content, 2)) + (0.0651*lowest_water_content);
        LOG_ERROR_IF(impedance_factor<0); // a check against negative impedance factor
        RmAssert( impedance_factor>=0, "Negative impedance factor");

        delta_N = DIFFUSION_COEFF_SOLN * lowest_water_content*impedance_factor * (nitrate_concentration - nitrate_conc_next)* area_per_unit_distance * dT;

        nitrate_amount -= delta_N;
        LOG_ERROR_IF(nitrate_amount<0); // a check against negative N amount
        RmAssert( nitrate_amount>=0, "Negative N amount");

        nitrate_amount_next += delta_N;
        Set_Concentration_Of_Box(Nitrate_Amount_Index, nitrate_amount, this_x, this_y, this_z);
        Set_Concentration_Of_Box(Nitrate_Amount_Index, nitrate_amount_next, next_x, next_y, next_z);
        Set_Concentration_Of_Box(Nitrate_Concentration_Index, (nitrate_amount/(box_volume*water_content)), this_x, this_y, this_z);
        Set_Concentration_Of_Box(Nitrate_Concentration_Index, (nitrate_amount_next/(box_volume_next*water_content_next)), next_x, next_y, next_z);
    } //if (nitrate_conc_next < nitrate_concentration)


    return (return_value);
} //ProcessTemplate::CalculateNitrateDiffusion



//************************************************************************************
// VMD 23/9/98 This routine is based on the ProcessTemplate leaching routine but is to be called by the water redistribution routine rather
// than the drain routine.  As plant roots dry out a cell, water moves into it from neighboring cells.  That water will also bring with it ProcessTemplate,
// so that there is constantly ProcessTemplate being drawn to the rhizosphere with the water as the plant transpires.  This routine differes from the leach
// routine in that it allows ProcessTemplate movement in 3 dimensions, and that it calculates displacement on a water in basis only - not on water in/water out.
//VMD 3/11/99 Made changes to the calculation of the truncated area
long int ProcessTemplate::MassFlowN(ProcessActionDescriptor *action)
{
    RmAssert(false,"This code is still in use?!?");
    Use_ReturnValue;
    Use_Time;
    Use_ScoreboardCoordinator;
    Use_ProcessCoordinator;
    Use_Scoreboard;
    Use_Boundaries;

    long num_x, num_y, num_z;                    // number of layers in each dimension
    Get_Number_Of_Layers(num_x,num_y,num_z);    // How many boxes do we have?

    BoxCoordinate box_coord;
    BoxCoordinate * box_coords = &box_coord;

    // Variables used for calculating the displacement of each ProcessTemplate.
    double box_top, box_height, box_width, box_depth, water_moved, box_volume;
    double mean_displacement, variance, position, frequency, wetting_front, water_content;
    double random_number, area, frequency_at_mean, nitrate_amount, dot_coordinate;
    double truncated_area, non_truncated_area, adjustment_factor, field_capacity, truncate, no_displacement;
    double dispersivity = 3.0;  //Displacement dependent dispersivity (cm), the figure of 3cm is from Art Diggle's thesis

    long int number_of_points, enough_points, area_finished, points_finished, x, y, z;
    long int box_index, i;
    long int counter, index, do_nothing;
    int direction_moved, redistribute;
    double total_old_nitrate = 0, total_new_nitrate = 0;
    long int n_boxes = 0;

    double freq;
    double * pFrequency = &freq;
    double * frequency_array = NULL;

    // Allocate memory for the array that store the numbers of ProcessTemplate packets per box
    // Arrange to fail if it can't be done, cause if we can't even set aside space for THAT,
    // we certainly won't have enough memory for the allocations to come.
    long * number_of_packets_array = new long[num_x*num_y*num_z];
    RmAssert(0!=number_of_packets_array, "Could not allocate space for number_of_packets_array");

    nitrate_before = 0;
    nitrate_amount_before = 0;
    nitrate_dots_before = 0;
    nitrate_after = 0;
    counter = 0, index = 0, direction_moved = 0;
    redistribute = 0;


    while (counter < Total_Nitrates)
    {
        pNitrateArray->GetCoordinate(counter, pNitrate);
        do_nothing = 0;
        mean_displacement = 0;
        nitrate_dots_before += pNitrate->s;

        if ((pNitrate->x == 0)&&(pNitrate->y == 0)&&(pNitrate->z == 0))
        {
            //Do nothing, this ProcessTemplate has been taken up so it doesn't exist anymore.
            if (pNitrate->s != 0)
            {
                LOG_ERROR_IF((pNitrate->s != 0));
            }
        }
        else
        {
            scoreboardcoordinator->FindBoxBySoilCoordinate(box_coords, scoreboard, pNitrate);

            box_top = Get_Layer_Top(box_coords->z);
            box_width = Get_Layer_Width(box_coords->x);
            box_depth = Get_Layer_Depth(box_coords->y);
            box_height = Get_Layer_Height(box_coords->z);

            water_moved = Get_Concentration_Of_Box(Water_Moved_Index, box_coords->x, box_coords->y, box_coords->z); //cm of water
            direction_moved = Get_Concentration_Of_Box(Direction_Moved_Index, box_coords->x, box_coords->y, box_coords->z);
            water_content = Get_Concentration_Of_Box(Water_Content_Index, box_coords->x, box_coords->y, box_coords->z);
            field_capacity = Get_Concentration_Of_Box(Drained_Upper_Limit_Index, box_coords->x, box_coords->y, box_coords->z);


            if ((water_moved >= 0.01)&&(do_nothing != 1))  //Move some ProcessTemplate.
            {
                redistribute = 1;
                mean_displacement = 0;
                i=0;

                //Calculate the mean displacement - based on how far the water would have moved if the soil had been perfectly dry.
                if (direction_moved == -1) //Water and hence the ProcessTemplate moved through the left hand face of the box (-x direction).
                {
                    while ((water_moved > (field_capacity*box_width)) && (i < 2))
                    {
                        field_capacity = Get_Concentration_Of_Box(Drained_Upper_Limit_Index, ((box_coords->x)-i), box_coords->y, box_coords->z);
                        water_moved -= (field_capacity*box_width);
                        ++i;
                        box_width = Get_Layer_Width((box_coords->x)-i);
                        mean_displacement += box_width;
                    }

                    box_width = Get_Layer_Width((box_coords->x)-i);
                    mean_displacement += (water_moved / field_capacity);
                    wetting_front = Get_Layer_Left((box_coords->x)-1);
                } //if (direction_moved == -1)

                if (direction_moved == 1) //Water and hence the ProcessTemplate moved through the right hand face of the box (+ x direction).
                {

                    while ((water_moved > (field_capacity*box_width)) && (i < 2))
                    {
                        field_capacity = Get_Concentration_Of_Box(Drained_Upper_Limit_Index, ((box_coords->x)+i), box_coords->y, box_coords->z);
                        water_moved -= (field_capacity*box_width);
                        ++i;
                        box_width = Get_Layer_Width((box_coords->x)+i);
                        mean_displacement += box_width;
                        LOG_ERROR_IF(((water_moved > (field_capacity*box_width)) && (i == 2)));
                    }

                    box_width = Get_Layer_Width((box_coords->x)+i);
                    mean_displacement += (water_moved / field_capacity);
                    wetting_front = Get_Layer_Right((box_coords->x)+1);
                } //if (direction_moved == 1)

                if (direction_moved == -2) //Water and hence the ProcessTemplate moved through the front face of the box (- y direction).
                {
                    while ((water_moved > (field_capacity*box_depth)) && (i < 2))
                    {
                        field_capacity = Get_Concentration_Of_Box(Drained_Upper_Limit_Index, (box_coords->x), (box_coords->y)-i, box_coords->z);
                        water_moved -= (field_capacity*box_depth);
                        ++i;
                        box_depth = Get_Layer_Depth((box_coords->y)-i);
                        mean_displacement += box_depth;
                        LOG_ERROR_IF((water_moved > (field_capacity*box_width)) && (i == 2));
                    }

                    box_depth = Get_Layer_Depth((box_coords->y)-i);
                    mean_displacement += (water_moved / field_capacity);
                    wetting_front = Get_Layer_Front((box_coords->y)-1);
                } //if (direction_moved == -2)

                if (direction_moved == 2) //Water and hence the ProcessTemplate moved through the back face of the box (+ y direction).
                {
                    while ((water_moved > (field_capacity*box_depth)) && (i < 2))
                    {
                        field_capacity = Get_Concentration_Of_Box(Drained_Upper_Limit_Index, (box_coords->x), ((box_coords->y) +i), box_coords->z);
                        water_moved -= (field_capacity*box_depth);
                        ++i;
                        box_depth = Get_Layer_Depth((box_coords->y)+i);
                        mean_displacement += box_depth;
                        LOG_ERROR_IF((water_moved > (field_capacity*box_width)) && (i == 2));
                    }

                    box_depth = Get_Layer_Depth((box_coords->y)+i);
                    mean_displacement += (water_moved / field_capacity);
                    wetting_front = Get_Layer_Start(Y,((box_coords->y)+1));
                } //if (direction_moved == 2)

                if (direction_moved == -3) //Water and hence the ProcessTemplate moved through the top face of the box (- z direction).
                {
                    while ((water_moved > (field_capacity*box_height)) && (i < 2))
                    {
                        field_capacity = Get_Concentration_Of_Box(Drained_Upper_Limit_Index, (box_coords->x), box_coords->y, (box_coords->z)-i);
                        water_moved -= (field_capacity*box_width);
                        ++i;
                        box_height = Get_Layer_Height((box_coords->z)-i);
                        mean_displacement += box_height;
                        LOG_ERROR_IF((water_moved > (field_capacity*box_width)) && (i == 2));
                    }

                    box_height = Get_Layer_Height((box_coords->z)-i);
                    mean_displacement += (water_moved / field_capacity);
                    wetting_front = Get_Layer_Top((box_coords->z)-1);
                } //if (direction_moved == -3)

                if (direction_moved == 3) //Water and hence the ProcessTemplate moved through the bottom face of the box (+ z direction).
                {
                    while ((water_moved > (field_capacity*box_height)) && (i < 2))
                    {
                        field_capacity = Get_Concentration_Of_Box(Drained_Upper_Limit_Index, box_coords->x, box_coords->y, (box_coords->z)+i);
                        water_moved -= (field_capacity*box_height);
                        ++i;
                        box_height = Get_Layer_Height((box_coords->z)+i);
                        mean_displacement += box_height;
                        LOG_ERROR_IF((water_moved > (field_capacity*box_width)) && (i == 2));
                    }

                    box_height = Get_Layer_Height((box_coords->z)+i);
                    mean_displacement += (water_moved / field_capacity);
                    wetting_front = Get_Layer_Bottom((box_coords->z)+1);
                } //if (direction_moved == 3)


                LOG_ERROR_IF((direction_moved !=3)&(direction_moved !=-3)&(direction_moved !=2)&(direction_moved !=-2)&(direction_moved !=1)&(direction_moved !=-1));
                LOG_ERROR_IF(mean_displacement<0);

                double STEP_SIZE_SMALL = mean_displacement / 5; //this makes the division of the frequency distribution relative to the size of the mean displacement

                //Now that we know what the mean displacement is we can move some ProcessTemplate.

                variance = 2 * dispersivity * (mean_displacement);
                position = mean_displacement;

                frequency_at_mean = (1/ (sqrt(2*PI*variance))) * (exp( -((pow((position-(mean_displacement)),2)) / (2 * variance))));
                frequency = frequency_at_mean;

                while (frequency > (FREQ_CUTOFF*frequency_at_mean))
                {
                    position -= STEP_SIZE_SMALL;
                    frequency = (1/ (sqrt(2*PI*variance))) * (exp( -((pow((position-(mean_displacement)),2)) / (2 * variance))));
                }

                // We are going to store the frequency distibution into an array so we need to know how many points there are going to be.
                position += STEP_SIZE_SMALL;
                number_of_points = (((mean_displacement) - position)/STEP_SIZE_SMALL) + 1;

                frequency_array = new double[number_of_points];
                RmAssert(0!=frequency_array, "Could not allocate memory for frequency_array");

                enough_points = 1;

                // Firstly just calculate the entire distribution regardless of any wetting front positions or scoreboard boundaries.
                // Will start at the mean and calulate the distribution for one - ie keep calculating in increments or decrements of z
                // until the frequency value is less than the cut off frequency.  There will be too many points if we go any further along each tail.
                // since the distribution is symmetrical, ther is no need to waste time calculating both sides - they are exactly the same!

                // Calculate one of the distribution starting at the mean.  The other side is a mirror image of the first.

                position = mean_displacement;
                index = 0;
                truncated_area = 0;
                non_truncated_area = 0;
                truncate = 0;

                if (direction_moved < 0)
                {
                    while (index < number_of_points)
                    {
                        frequency = (1/ (sqrt(2*PI*variance))) * (exp( -((pow((position-(mean_displacement)),2)) / (2 * variance))));

                        *pFrequency = frequency;
                        frequency_array[index] = *pFrequency;

                        // A suitable increment in z value is determined by calculating the area under the frequency distribution with smaller and smaller
                        // delta z values until the value for the area does not change significantly.  We want to calculate the truncated area as we go so
                        // the position of the truncation point will determine if we do the "RHS" or "LHS" of the distribution.



                        switch (direction_moved)
                        {
                        case -1:
                            dot_coordinate = pNitrate->x;
                            break;
                        case -2:
                            dot_coordinate = pNitrate->y;
                            break;
                        case -3:
                            dot_coordinate = pNitrate->z;
                            break;
                        }

                        if ((dot_coordinate - mean_displacement) < wetting_front)
                        {
                            if ((dot_coordinate - position) < wetting_front)
                            {
                                truncated_area += frequency*STEP_SIZE_SMALL;
                                truncate = 1;
                            }

                            position -= STEP_SIZE_SMALL;
                        }
                        else
                        {
                            if ((dot_coordinate - position) >= wetting_front)
                            {
                                non_truncated_area += frequency*STEP_SIZE_SMALL;
                                truncate = 1;
                            }

                            position += STEP_SIZE_SMALL;
                        }

                        index += 1;

                    } // while (index < number_of_points)

                    if (dot_coordinate - mean_displacement < wetting_front)
                    {
                        truncated_area += 0.5;  //All of the RHS of the distribution is truncated aswell.
                    }
                    if (dot_coordinate - mean_displacement >= wetting_front)
                    {
                        truncated_area = 0.5 - non_truncated_area;
                    }
                } //if (direction_moved < 0)

                else
                {
                    while (index < number_of_points)
                    {
                        frequency = (1/ (sqrt(2*PI*variance))) * (exp( -((pow((position-(mean_displacement)),2)) / (2 * variance))));

                        *pFrequency = frequency;
                        frequency_array[index] = *pFrequency;

                        // A suitable increment in z value is determined by calculating the area under the frequency distribution with smaller and smaller
                        // delta z values until the value for the area does not change significantly.  We want to calculate the truncated area as we go so
                        // the position of the truncation point will determine if we do the "RHS" or "LHS" of the distribution.

                        switch (direction_moved)
                        {
                        case 1:
                            dot_coordinate = pNitrate->x;
                            break;
                        case 2:
                            dot_coordinate = pNitrate->y;
                            break;
                        case 3:
                            dot_coordinate = pNitrate->z;
                            break;
                        }

                        //Wetting front is in the LHS of the distribution.
                        if ((dot_coordinate + mean_displacement) > wetting_front)
                        {
                            if ((dot_coordinate + position) > wetting_front)
                            {
                                truncated_area += frequency*STEP_SIZE_SMALL;
                                truncate = 1;
                            }

                            position -= STEP_SIZE_SMALL;
                        }


                        //Wetting front is in the RHS of the distribution.
                        else
                        {
                            if ((dot_coordinate + position) <= wetting_front)
                            {
                                non_truncated_area += frequency*STEP_SIZE_SMALL;
                                truncate = 1;
                            }

                            position += STEP_SIZE_SMALL;
                        }

                        index += 1;

                    } // while (index < number_of_points)

                    if (dot_coordinate + mean_displacement > wetting_front)
                    {
                        truncated_area += 0.5;  //All of the RHS of the distribution is truncated aswell.
                    }
                    if (dot_coordinate + mean_displacement < wetting_front)
                    {
                        truncated_area = 0.5 - non_truncated_area;
                    }
                } //else (direction_moved > 0)



                //A ProcessTemplate cannot move passed the wetting front, so the area under the frequency distribution that is truncated must
                // be redistributed over the remaining area.  This is done using the adjustment factor.

                no_displacement = 1;
                if (truncated_area >=1)
                {
                    truncate = 0;
                    no_displacement = 0;
                }

                if (truncate == 1)
                {
                    adjustment_factor = truncated_area / (1 - truncated_area);


                    for (index = 0; index < number_of_points; ++index)
                    {
                        frequency = frequency_array[index];
                        frequency_array[index] = frequency + (frequency*adjustment_factor);
                    }
                }

                random_number = Random1();
                while (random_number > 0.98)
                {
                    random_number = Random1();
                }

                //Keep adding the area under the frequency distribution until the area is greater than or equal to the random number.

                if (no_displacement == 0)
                {
                    mean_displacement = 0;
                }


                if (no_displacement != 0)
                {
                    area = 0.0;
                    points_finished = 1;
                    area_finished = 1;
                    //Start at the very LHS of the distribution and step right.
                    position = mean_displacement - ((number_of_points)*STEP_SIZE_SMALL);
                    index = (number_of_points - 1);

                    while ((points_finished != 0) && (area_finished != 0))
                    {
                        *pFrequency = frequency_array[index];
                        area += (STEP_SIZE_SMALL * (*pFrequency));


                        if (area >= random_number)
                        {
                            mean_displacement = position + (0.5*STEP_SIZE_SMALL);
                            area_finished = 0;
                        }

                        if (index == 0)
                        {
                            points_finished = 0;
                        }
                        index-=1;
                        position +=STEP_SIZE_SMALL;

                    } //while ((points_finished != 0) && (area_finished != 0))

                    index = 0;

                    while ((points_finished == 0) && (area_finished != 0))
                        // Will leach passed the mean
                    {
                        *pFrequency = frequency_array[index];
                        area += (STEP_SIZE_SMALL * (*pFrequency));

                        if (area >= random_number)
                        {
                            mean_displacement = position + (0.5*STEP_SIZE_SMALL);
                            area_finished = 0;
                        }


                        LOG_ERROR_IF((index == number_of_points) && (area_finished == 1));
                        if ((index == number_of_points) && (area_finished == 1))
                        {
                            points_finished = 1;
                        }

                        index+=1;
                        position +=STEP_SIZE_SMALL;

                    } //while ((points_finished == 0) && (area_finished != 0))
                } //if (no_displacement != 0)

                position -= STEP_SIZE_SMALL;
                if (mean_displacement > 0)
                {
                    BEGIN_DRAWING
                    drawing.RemoveDot(*pNitrate); //remove a dot
                    END_DRAWING
                }

                LOG_ERROR_IF((pNitrate ->z == 0)&&(pNitrate->x == 0));

                if ((direction_moved < 0) &&(mean_displacement > 0))
                {

                    switch (direction_moved)
                    {
                    case -1:
                        pNitrate ->x = (pNitrate ->x) - (mean_displacement/2); //ProcessTemplate is leached to the new x position.
                        if (pNitrate->x < wetting_front)
                        {
                            pNitrate->x = wetting_front;
                        }
                        break;
                    case -2:
                        pNitrate ->y = (pNitrate ->y) - (mean_displacement/2);  //ProcessTemplate is leached to the new y position.
                        if (pNitrate->y < wetting_front)
                        {
                            pNitrate->y = wetting_front;
                        }
                        break;
                    case -3:
                        pNitrate ->z = (pNitrate ->z) - (mean_displacement/2);  //ProcessTemplate is leached to the new z position.
                        if (pNitrate->z < wetting_front)
                        {
                            pNitrate->z = wetting_front;
                        }
                        break;
                    }

                    pNitrateArray->SetCoordinate(counter, pNitrate);

                    BEGIN_DRAWING
                    if (pNitrate->s == m_NITRATE_PACKET_SIZE)
                    {
                        drawing.DrawDot(*pNitrate);
                    }
                    else
                    {
                        drawing.RemoveDot(*pNitrate); //remove a dot
                    }
                    END_DRAWING
                } // if((direction_moved < 0) &&(mean_displacement > 0))


                if ((direction_moved > 0) &&(mean_displacement > 0))
                {

                    switch (direction_moved)
                    {
                    case 1:
                        pNitrate ->x = (pNitrate ->x) + (mean_displacement/2);  //ProcessTemplate is leached to the new x position.
                        if (pNitrate->x > wetting_front)
                        {
                            pNitrate->x = wetting_front;
                        }
//                        LOG_ERROR_IF((pNitrate->x > (Get_Left_To_Right_Layer_List->GetLayerBottom(num_x))) || (pNitrate->x < (Get_Left_To_Right_Layer_List->GetLayerTop(1))));
                        LOG_ERROR_IF((pNitrate->x > (Get_Layer_Right(num_x))) || (pNitrate->x < (Get_Layer_Left(1))));

                        break;
                    case 2:
                        pNitrate ->y = (pNitrate ->y) + (mean_displacement/2);  //ProcessTemplate is leached to the new y position.
                        if (pNitrate->y > wetting_front)
                        {
                            pNitrate->y = wetting_front;
                        }

                        LOG_ERROR_IF((pNitrate->y > (Get_Layer_Back(num_y))) || (pNitrate->y < (Get_Layer_Front(1))));
                        break;
                    case 3:
                        pNitrate ->z = (pNitrate ->z) + (mean_displacement/2);  //ProcessTemplate is leached to the new z position.
                        if (pNitrate->z > wetting_front)
                        {
                            pNitrate->z = wetting_front;
                        }
                        if (pNitrate->z < 0)
                        {
                            pNitrate->z = 0.01;
                        }

                        LOG_ERROR_IF((pNitrate->z > (Get_Layer_Bottom(num_z))) || (pNitrate->z < (Get_Layer_Top(1))));
                        break;
                    }

                    pNitrateArray->SetCoordinate(counter, pNitrate);

                    BEGIN_DRAWING
                    if (pNitrate->s == m_NITRATE_PACKET_SIZE)
                    {
                        drawing.DrawDot(*pNitrate);
                    }
                    else
                    {
                        drawing.RemoveDot(*pNitrate); //remove a dot
                    }
                    END_DRAWING
                } // if((direction_moved > 0) &&(mean_displacement > 0))

            } // if(water_moved != 0)
        } //else

        delete[] frequency_array;
        frequency_array = NULL;
        counter += 1;

    } //while (counter < Total_Nitrates)


    // Go through and work out how many nitrates are now in each scoreboard box and hence recalculate
    // the total amount in each box.


    box_index = 0;
    counter = 0;

    if (redistribute == 1)
    {

        //massflow has caused ProcessTemplate dots to move, so now need to recalculate the ProcessTemplate amount in each box

        for (y=1; y<=num_y; ++y)
        {
            for (x=1; x<=num_x; ++x)
            {
                for (z=1; z<=num_z; ++z)
                {
                    nitrate_amount = Get_Concentration_Of_Box(Nitrate_Amount_Index,x, y, z);
                    nitrate_amount_before += nitrate_amount;
                    Set_Concentration_Of_Box(Nitrate_Amount_Index,0, x,y,z);
                }
            }
        }

        LOG_ERROR_IF(nitrate_dots_before!=nitrate_amount_before);

        counter = 0;
        while (counter < Total_Nitrates)
        {
            pNitrateArray->GetCoordinate(counter, pNitrate);
            nitrate_amount = 0;
            if ((pNitrate->x != 0)&&(pNitrate->y != 0)&&(pNitrate->z != 0))
            {
                scoreboardcoordinator->FindBoxBySoilCoordinate(box_coords, scoreboard, pNitrate);
                nitrate_amount = Get_Concentration_Of_Box(Nitrate_Amount_Index,box_coords ->x, box_coords ->y, box_coords ->z);
                nitrate_amount += (pNitrate->s);
                nitrate_after += pNitrate->s;
                Set_Concentration_Of_Box(Nitrate_Amount_Index,nitrate_amount, box_coords ->x, box_coords ->y, box_coords ->z);
                water_content = Get_Concentration_Of_Box(Water_Content_Index,box_coords ->x, box_coords ->y, box_coords ->z);
                box_volume = Get_Box_Volume(box_coords ->x, box_coords ->y, box_coords ->z);
                Set_Concentration_Of_Box(Nitrate_Concentration_Index, (nitrate_amount/(box_volume*water_content)), box_coords ->x, box_coords ->y, box_coords ->z);
            }
            counter += 1;
        } //while ((counter < Total_Nitrates)&&(number_nitrate_packets>0))

        LOG_ERROR_IF(nitrate_before!=nitrate_after);

    }//if (redistribute == 1)

    delete[] number_of_packets_array;

    return(return_value);
} //long int ProcessTemplate::MassFlowN(ProcessActionDescriptor *action)


/* ****************************************************************************************************** */
//VMD 3/11/99 made changes the calculation of the mean displacement and the calculation of the truncated area.

long int ProcessTemplate::LeachN(ProcessActionDescriptor *action)
{

    Use_ReturnValue;
    Use_Time;
    Use_ScoreboardCoordinator;
    Use_ProcessCoordinator;
    Use_Scoreboard;
    Use_Boundaries;

    long num_x, num_y, num_z;                    // number of layers in each dimension
    Get_Number_Of_Layers(num_x,num_y,num_z);    // How many boxes do we have?

    BoxCoordinate box_coord;
    BoxCoordinate * box_coords = &box_coord;

    // Variables used for calculating the displacement of each ProcessTemplate.
    double box_top, box_height, box_width, box_depth, dist_fraction, box_volume;
    double mean_displacement, mean_displacement2, variance, position, frequency, wetting_front;
    double random_number, area, frequency_at_mean, water_in, water_out, delta_water, water_content;
    double nitrate_amount, box_bottom, nitrate_leached;
    double dispersivity = 3.0; //displacement dependent dispersivity (cm) value of 3cm is from A. Diggle's thesis
    double truncated_area, non_truncated_area, adjustment_factor, field_capacity, truncate, no_displacement;

    long int number_of_points, enough_points, area_finished, points_finished, x, y, z;
    long int counter, index, endsboard, i;
    long int leached_this_time = 0, n_boxes = 0;
    double total_old_nitrate = 0, total_new_nitrate = 0;

    double freq;
    double * pFrequency = &freq;
    double * frequency_array = NULL;


    // Allocate memory for the array that stores the number of ProcessTemplate packets per box
    // Arrange to fail if it can't be done, cause if we can't even set aside space for THAT,
    // we certainly won't have enough memory for the allocations to come.
    long * number_of_packets_array = new long[num_x*num_y*num_z];
    RmAssert(0!=number_of_packets_array, "Could not allocate space for number_of_packets_array");

    nitrate_amount = 0;

    counter = 0, index = 0;


    while (counter < Total_Nitrates)
    {
        pNitrateArray->GetCoordinate(counter, pNitrate);
        mean_displacement = 0;
        nitrate_leached = 0;

        if ((pNitrate->x == 0)&&(pNitrate->y == 0)&&(pNitrate->z == 0))
        {
            //Do nothing, this ProcessTemplate has been taken up so it doesn't exist anymore.
        }

        else
        {
            scoreboardcoordinator->FindBoxBySoilCoordinate(box_coords, scoreboard, pNitrate);
            box_top = Get_Layer_Top(box_coords->z);
            box_height = Get_Layer_Height(box_coords->z);
            box_depth =  Get_Layer_Depth(box_coords->y);
            box_width = Get_Layer_Width(box_coords->x);
            dist_fraction = ((pNitrate ->z) - box_top) / box_height;
            water_in = Get_Concentration_Of_Box(Rained_Amount_Index, box_coords->x, box_coords->y, box_coords->z); //cm of water
            endsboard = 0;

            if (water_in != 0) //then some water has swept passed and ProcessTemplate needs to leach
            {
                if ((box_coords->z) <= (num_z -1))
                {
                    water_out = Get_Concentration_Of_Box(Rained_Amount_Index, box_coords->x, box_coords->y, ((box_coords->z)+1));
                    field_capacity = Get_Concentration_Of_Box(Drained_Upper_Limit_Index, box_coords->x, box_coords->y, box_coords->z);
                }

                else
                {
                    field_capacity = Get_Concentration_Of_Box(Drained_Upper_Limit_Index, box_coords->x, box_coords->y, box_coords->z);
                    water_out = Get_Concentration_Of_Box(Rain_Out_Index, box_coords->x, box_coords->y, box_coords->z);
                }

                if ((water_out == water_in)) //This box was already filled to field capacity so all the water has moved to the next box
                {
                    i=0;
                    while ((water_out == water_in)&&((box_coords->z)+i+1) <= num_z)
                    {
                        ++i;
                        water_out = Get_Concentration_Of_Box(Rained_Amount_Index, box_coords->x, box_coords->y, (box_coords->z)+i+1);
                        water_in = Get_Concentration_Of_Box(Rained_Amount_Index, box_coords->x, box_coords->y, (box_coords->z)+i);
                    }

                    if (((box_coords->z)+i+1) > num_z)
                    {
                        endsboard = 1;  // There has been so much water that the little ProcessTemplate has been swept out of the bottom of the scoreboard.
                        water_out = Get_Concentration_Of_Box(Rain_Out_Index, box_coords->x, box_coords->y, (box_coords->z)+i);
                    }

                    field_capacity = Get_Concentration_Of_Box(Drained_Upper_Limit_Index, box_coords->x, box_coords->y, (box_coords->z)+i);
                    box_height = Get_Layer_Height((box_coords->z)+i);
                    while ((water_in > (field_capacity*box_height)) && (((box_coords->z) + i) <= num_z))
                    {
                        water_in -= (field_capacity*box_height); //maximum potential displacement in dry soil
                        ++i;
                        field_capacity = Get_Concentration_Of_Box(Drained_Upper_Limit_Index, box_coords->x, box_coords->y, (box_coords->z) + i);
                        box_height = Get_Layer_Height((box_coords->z)+i);
                    } //while

                    if (((box_coords->z) + i) > num_z)
                    {
                        endsboard = 1;
                        box_top = Get_Layer_Bottom((box_coords->z));
                        mean_displacement = (water_in / field_capacity) + box_top - (pNitrate->z);
                    }
                    else
                    {
                        box_top = Get_Layer_Top((box_coords->z)+i);
                        mean_displacement = (water_in / field_capacity) + box_top - (pNitrate->z);
                    }
                } //if ((water_out == water_in))

                else //The ProcessTemplate ions don't get swept along for a few boxes.
                {
                    if (water_out != 0)
                    {
                        //How much water flows passed the ProcessTemplate?
                        delta_water = water_in - water_out;
                        water_in -= (delta_water * dist_fraction);  //this is how much water would get to that ProcessTemplate ion

                        if (water_in <= field_capacity*(box_height*(1-dist_fraction)))
                        {
                            mean_displacement = (water_in / field_capacity);
                        }

                        if (water_in > field_capacity*(box_height*(1-dist_fraction)))
                        {
                            i=1;
                            water_in -= field_capacity*(box_height*(1-dist_fraction)); // How much water goes into the next box
                            if (((box_coords->z) + i) > num_z)
                            {
                                field_capacity = Get_Concentration_Of_Box(Drained_Upper_Limit_Index, box_coords->x, box_coords->y, box_coords->z);
                            }
                            else
                            {
                                field_capacity = Get_Concentration_Of_Box(Drained_Upper_Limit_Index, box_coords->x, box_coords->y, (box_coords->z) + i);
                                box_height = Get_Layer_Height((box_coords->z)+i);
                            }
                            while ((water_in > field_capacity*box_height) && (((box_coords->z) + i) <= num_z))
                            {
                                water_in -= (field_capacity*box_height);
                                ++i;
                                field_capacity = Get_Concentration_Of_Box(Drained_Upper_Limit_Index, box_coords->x, box_coords->y, (box_coords->z) + i);
                                box_height = Get_Layer_Height((box_coords->z)+i);
                            } //while
                            if (((box_coords->z) + i) > num_z)
                            {
                                box_top =  Get_Layer_End(Z,(box_coords->z));
                                mean_displacement = (water_in / field_capacity) + box_top - (pNitrate->z);
                                endsboard = 1;
                            }
                            else
                            {
                                box_top = Get_Layer_Start(Z,(box_coords->z)+i);
                                mean_displacement = (water_in / field_capacity) + box_top - (pNitrate->z);
                            }
                        } //if (water_in > field_capacity)
                    }  //if ((water_out != 0)

                    else // water out = 0
                    {
                        delta_water = water_in;
                        water_in -= (delta_water * dist_fraction);
                        mean_displacement = (water_in / field_capacity);
                    } //else, water out = 0

                } //else The ProcessTemplate ions don't get swept along for a few boxes.

                double STEP_SIZE = mean_displacement / 20; //this makes the frequency distribution division relative to the size of the mean displacement

                if (endsboard != 1) //the mean displacement of the ProcessTemplate ion is not out the bottom of the board so need to calculated its actual displacement
                {
                    water_out = Get_Concentration_Of_Box(Rained_Amount_Index, box_coords->x, box_coords->y, (box_coords->z)+1);

                    if (water_out !=0)  // Mean ProcessTemplate displacement will be below this box.
                    {
                        wetting_front = Get_Concentration_Of_Box(Wetting_Front_Index, box_coords->x, box_coords->y, ((box_coords->z)+1));
                        i=1;

                        while ((wetting_front == 0) && ((i + (box_coords->z)) <= num_z))
                        {
                            ++i;
                            wetting_front = Get_Concentration_Of_Box(Wetting_Front_Index, box_coords->x, box_coords->y, (box_coords->z)+ i);
                        } //while (wetting_front == 0)


                        if ((i + (box_coords->z)) > num_z)
                        {
                            wetting_front = 0;
                        }
                    } //if (water_out !=0)

                    if (water_out == 0) // Water has not moved out of the box, and may or may not have gone passed the ProcessTemplate.
                    {
                        wetting_front = Get_Concentration_Of_Box(Wetting_Front_Index, box_coords->x, box_coords->y, box_coords->z);
                        if ((pNitrate ->z) > wetting_front)
                        {
                            mean_displacement = 0;  // the wetting front is above the ProcessTemplate so it wont have moved any where.
                        }
                    } //if (water_out == 0)

                    if (mean_displacement <= 0.5)
                    {
                        mean_displacement = 0;  // do nothing as no water has gone passed thus the ProcessTemplate is going no where.
                    }
                    mean_displacement2 = mean_displacement;

                    //Now that we know what the mean displacement is we can do some leaching!!

                    if (mean_displacement != 0) //Water has gone passed the ProcessTemplate and it will leach.
                    {
                        variance = 2 * dispersivity * mean_displacement;
                        position = mean_displacement;

                        frequency_at_mean = (1/ (sqrt(2*PI*variance))) * (exp( -((pow((position-mean_displacement),2)) / (2 * variance))));
                        frequency = frequency_at_mean;

                        while (frequency > (FREQ_CUTOFF*frequency_at_mean))
                        {
                            position -= STEP_SIZE;
                            frequency = (1/ (sqrt(2*PI*variance))) * (exp( -((pow((position-mean_displacement),2)) / (2 * variance))));
                        }

                        // We are going to store the frequency distibution into an array so we need to know howmany points there are going to be.
                        position += STEP_SIZE;
                        number_of_points = ((mean_displacement - position)/STEP_SIZE) + 1;

                        frequency_array = new double[number_of_points];
                        RmAssert(0!=frequency_array, "Could not allocate memory for frequency_array");

                        enough_points = 1;

                        // Firstly just calculate the entire distribution regardless of any wetting front positions or scoreboard boundaries.
                        // Will start at the mean and calulate the distribution for one - ie keep calculating in increments or decrements of z
                        // until the frequency value is less than a the cut off frequency.  There will be too many points if we go any further along each tail.
                        // since the distribution is symmetrical, ther is no need to waste time calculating both sides - they are exactly the same!

                        // Calculate one of the distribution starting at the mean.  The other side is a mirror image of the first.

                        position = mean_displacement;
                        index = 0;
                        truncated_area = 0;
                        non_truncated_area = 0;
                        truncate = 0;

                        while (index < number_of_points)
                        {
                            frequency = (1/ (sqrt(2*PI*variance))) * (exp( -((pow((position-mean_displacement),2)) / (2 * variance))));


                            *pFrequency = frequency;
                            frequency_array[index] = *pFrequency;


                            // A suitable increment in z value is determined by calculating the area under the frequency distribution with smaller and smaller
                            // delta z values until the value for the area does not change significantly.  We want to calculate the truncated area as we go so
                            // the position of the truncation point will determine if we do the "RHS" or "LHS" of the distribution.

                            //The wetting front is in the LHS of the distribution, and any position greater than the wetting front will be truncated.
                            //Only need to calculate to the LHS and then add 0.5 to get the total truncated area.
                            if ((mean_displacement + pNitrate->z > wetting_front) && (wetting_front > 0))
                            {
                                if (((position + pNitrate->z) > wetting_front)&&(wetting_front > 0))
                                {
                                    truncated_area += frequency*STEP_SIZE;
                                    truncate = 1;
                                }
                                position -= STEP_SIZE;
                            }

                            //Else the wetting front is at the mean or in the RHS of the distribution.  Any point greater than the wetting front
                            //must be truncated.  Only need to calculate the RHS.  Calculate the area upto the wetting front and add 0.5 gives the total non-truncated
                            //area, then subtract this from 1 to get the truncated area.
                            else
                            {
                                if (((position + pNitrate->z) <= wetting_front)&&(wetting_front > 0))
                                {
                                    non_truncated_area += frequency*STEP_SIZE;
                                    truncate = 1;
                                }
                                position += STEP_SIZE;
                            }

                            ++index;
                        } // while (index < number_of_points)

                        //If the wetting front is in the LHS of the distribution, all of the RHS is truncated so need to add this on.
                        if ((mean_displacement + pNitrate->z > wetting_front) && (wetting_front > 0))
                        {
                            truncated_area += 0.5;  //All of the RHS of the distribution is truncated aswell.
                        }

                        //If the wetting front is in the RHS of the distribution, only the portion greater than the wetting front is truncated.
                        //This is calculated by subtracting the non-truncated area from the total area 1 (or 0.5 as only need to look at half the distribution).
                        if ((mean_displacement + pNitrate->z < wetting_front) && (wetting_front > 0))
                        {
                            truncated_area = 0.5 - non_truncated_area;
                        }

                        // A ProcessTemplate cannot move passed the wetting front, so the area under the frequency distribution that is truncated must
                        // be redistributed over the remaining area, so that the probability of being in the included area is 1.
                        // This is done using the adjustment factor - a multiplication factor that makes the area under the truncated curve the same
                        // as that under the original curve.

                        no_displacement = 1;
                        if (truncated_area >=1)
                        {
                            truncate = 0;
                            no_displacement = 0;
                        }

                        if (truncate == 1)
                        {
                            adjustment_factor = truncated_area / (1 - truncated_area);
                            for (index = 0; index < number_of_points; ++index)
                            {
                                frequency = frequency_array[index];
                                frequency_array[index] = frequency + (frequency*adjustment_factor);
                            }
                        }

                        random_number = Random1();
                        while (random_number > 0.95)
                        {
                            random_number = Random1();
                        }

                        //Keep adding the area under the frequency distribution until the area is greater than or equal to the random number.

                        if (no_displacement == 0)
                        {
                            mean_displacement = 0;
                        }

                        if (no_displacement != 0)
                        {
                            area = 0.0;
                            points_finished = 1;
                            area_finished = 1;
                            position = mean_displacement - ((number_of_points)*STEP_SIZE);
                            index = (number_of_points - 1);

                            while ((points_finished != 0) && (area_finished != 0))
                            {
                                *pFrequency = frequency_array[index];
                                area += (STEP_SIZE * (*pFrequency));

                                if (area >= random_number)
                                {
                                    mean_displacement = position + (0.5*STEP_SIZE);
                                    area_finished = 0;
                                }

                                if (index == 0)
                                {
                                    points_finished = 0;
                                }
                                --index;
                                position +=STEP_SIZE;

                            } //while ((points_finished != 0) && (area_finished != 0))

                            index = 0;

                            while ((points_finished == 0) && (area_finished != 0))
                                // Will leach passed the mean
                            {
                                *pFrequency = frequency_array[index];
                                area += (STEP_SIZE * (*pFrequency));

                                if (area >= random_number)
                                {
                                    mean_displacement = position + (0.5*STEP_SIZE);
                                    area_finished = 0;
                                }


                                if ((index == number_of_points) && (area_finished == 1))
                                {
                                    points_finished = 1;
                                    LOG_ERROR_IF((index == number_of_points) && (area_finished == 1));
                                }

                                ++index;
                                position +=STEP_SIZE;
                            } //while ((points_finished == 0) && (area_finished != 0))
                        } //if (no_displacement != 0)
                    }//if mean_displacement!=0
                } //if endsboard != 1

                position -= STEP_SIZE;


                BEGIN_DRAWING
                drawing.RemoveDot(*pNitrate);
                END_DRAWING

                LOG_ERROR_IF((pNitrate->z == 0)&&(pNitrate->x == 0));

                pNitrate ->z = (pNitrate ->z) + mean_displacement;  //ProcessTemplate is leached to the new z position.

                if ((pNitrate->z > wetting_front) && (wetting_front > 0) && (mean_displacement > 0))
                {
                    pNitrate->z = wetting_front;
                }

                if (pNitrate->z <= 0)
                {
                    pNitrate->z = 0.1;
                }

                box_bottom = Get_Layer_Bottom(num_z);

                if ((pNitrate->z > box_bottom)||(endsboard == 1))
                {
                    //The water and the ProcessTemplate has flowed out the bottom of the scoreboard hence make the ProcessTemplate
                    //coords 0,0,0 so that it dissapears off the screen.

                    nitrate_leached = Get_Concentration_Of_Box(Nitrate_Leached_Index, box_coords->x, box_coords->y, 1);
                    nitrate_leached += m_NITRATE_PACKET_SIZE;
                    leached_this_time += m_NITRATE_PACKET_SIZE;
                    Set_Concentration_Of_Box(Nitrate_Leached_Index, nitrate_leached, box_coords->x, box_coords->y, 1);
                    pNitrate->x = 0;
                    pNitrate->y = 0;
                    pNitrate->z = 0;
                    pNitrate->s = 0;
                }


                pNitrateArray->SetCoordinate(counter, pNitrate);

                if ((pNitrate->x != 0) && (pNitrate->y != 0) && (pNitrate->z != 0))
                {
                    BEGIN_DRAWING
                    if (pNitrate->s < m_NITRATE_PACKET_SIZE)
                    {
                        drawing.RemoveDot(*pNitrate); //this method tracks all ProcessTemplate, don't want to show part packets
                    }
                    else
                    {
                        drawing.DrawDot(*pNitrate);
                    }
                    END_DRAWING
                }

            } // if (water_in !=0) Water has gone passed the ProcessTemplate and need to calculate where it moves to.

            delete[] frequency_array;
            frequency_array = NULL;

        } //else
        ++counter;

    } //while (counter < Total_Nitrates)


    //leaching has caused ProcessTemplate dots to move, so now need to recalculate the ProcessTemplate amount in each box

    for (y=1; y<=num_y; ++y)
    {
        for (x=1; x<=num_x; ++x)
        {
            for (z=1; z<=num_z; ++z)
            {
                Set_Concentration_Of_Box(Nitrate_Amount_Index,0, x,y,z);
                Set_Concentration_Of_Box(Rained_Amount_Index, 0, x,y,z);
                Set_Concentration_Of_Box(Wetting_Front_Index, 0, x,y,z);
            }
        }
    }

    counter = 0;
    while (counter < Total_Nitrates)
    {
        pNitrateArray->GetCoordinate(counter, pNitrate);
        nitrate_amount = 0;
        if ((pNitrate->x != 0)&&(pNitrate->y != 0)&&(pNitrate->z != 0))
        {
            scoreboardcoordinator->FindBoxBySoilCoordinate(box_coords, scoreboard, pNitrate);
            nitrate_amount = Get_Concentration_Of_Box(Nitrate_Amount_Index,box_coords ->x, box_coords ->y, box_coords ->z);
            nitrate_amount += (pNitrate->s);
            Set_Concentration_Of_Box(Nitrate_Amount_Index,nitrate_amount, box_coords ->x, box_coords ->y, box_coords ->z);
            water_content = Get_Concentration_Of_Box(Water_Content_Index,box_coords ->x, box_coords ->y, box_coords ->z);
            box_volume = Get_Box_Volume(box_coords ->x, box_coords ->y, box_coords ->z);
            Set_Concentration_Of_Box(Nitrate_Concentration_Index, (nitrate_amount/(box_volume*water_content)), box_coords ->x, box_coords ->y, box_coords ->z);
        }
        counter += 1;
    } //while ((counter < Total_Nitrates)&&(number_nitrate_packets>0))



    delete[] number_of_packets_array;


    return(return_value);
}  // long int ProcessTemplate::LeachN(ProcessActionDescriptor *action)

/* ********************************************************************************* */

long int ProcessTemplate::UptakeN(ProcessActionDescriptor *action)
{
    // MSA 11.01.24 Is this method used? If not, I don't want to waste time updating it

    Use_ReturnValue;
    Use_Time;
    //UseObjects;
// WAS: MSA 11.05.02 IS!                                                                             RvH
    Use_ScoreboardCoordinator;
    Use_ProcessCoordinator;
    Use_PostOffice;
    Use_Scoreboard;
    Use_Boundaries;

    long int x,y,z;
    double root_length, tap_root_length, first_root_length, second_root_length, third_root_length;
    double water_content, water_flux0, water_flux1, water_flux2, water_flux3;
    double new_amount, new_uptake, absorption_power, nitrate_concentration, nitrate_uptake, nitrate_amount;
    double delta_amount0, delta_amount1, delta_amount2, delta_amount3 = 0;
    double box_volume;

    double factor10, factor11, factor12, factor13;
    double factor20, factor21, factor22, factor23;
    double factor30, factor31, factor32, factor33;
    double factor40, factor41, factor42, factor43;


    long num_x, num_y, num_z;                    // number of layers in each dimension
    Get_Number_Of_Layers(num_x,num_y,num_z);    // How many boxes do we have?

    double root_radius0, root_radius1, root_radius2, root_radius3; //average root radius for L. angustifolius is 0.045, for L. pilosus it is 0.03 cm.
    double buffer_power = 1; //for non-adsorbed ProcessTemplate
    double diffusion_coeff = 0.000019; //diffusion coefficient in free solution = 1.9e-5 cm2/sec for chloride/ProcessTemplate in free solution
    double impedance_factor;

    /*long int root_length_density_summary_index, branch_order;*/
    nitrate_uptake = 0;
    long int dT1;
    long int plant_counter = 1;

    dT1 = time - time_prev_waking;
    time_prev_waking = time;

    if (dT1==0)
    {
        return kNoError;
    }
    RmAssert( dT1>0, "Time delta since previous waking is negative");



    //This routine reproduces the ProcessTemplate mineralisation rates measured by Anderson et al in the field
    //CalculateNitrateMineralisation(action, dT1);
    nitrate_before = 0;
    nitrate_after = 0;


    //
    // This attempts (usually successfully :-) to get a Cluster of
    // SharedAttributeLists, clustered according to their variation by Plant
    const SharedAttributeCluster & cluster = Get_Cluster("Plant");

    //
    // Believe it or not, this is effectively a "for each Plant do..." loop
    for ( SharedAttributeCluster::const_iterator cluster_iter = cluster.begin() ;
            cluster_iter != cluster.end() ;
            ++cluster_iter
        )
    {
        //                                                                          RvH
        // This is "use the next Plant X attribute list".
        // Note that the name of the Plant/Cluster is in (*cluster_iter).first
        const SharedAttributeList & sa_list = (*cluster_iter).second;

        //
        // the variable saNitrateUptakePerPlantIndex is a data member that was
        // found during ::PrepareSharedAttributes
        SharedAttribute * root_length_sa = sa_list[saRootLengthPerPlantIndex];
        SharedAttribute * root_length_ro_sa = sa_list[saRootLengthPerPlantPerRootOrderIndex];
        SharedAttribute * root_radius_ro_sa = sa_list[saRootRadiusPerPlantPerRootOrderIndex];
        SharedAttribute * water_flux_sa = sa_list[saWaterFluxPerPlantPerRootOrderIndex];
        SharedAttribute * nitrate_uptake_sa = sa_list[saNitrateUptakePerPlantIndex];

        // WAS:                                                                     RvH
//    while(plant_counter<=nitrate_number_of_plants)
//    {

        for (y=1; y<=num_y; ++y)
        {
            for (z=1; z<=num_z; ++z)
            {
                for (x=1; x<=num_x; ++x)
                {
                    BoxIndex box_index = scoreboard->GetBoxIndex(x,y,z);

                    root_length = 0, tap_root_length = 0, first_root_length = 0, second_root_length = 0, third_root_length = 0;
                    factor40 = 0, factor41 = 0, factor42 = 0, factor43 = 0;

                    root_length = root_length_sa->GetValue(box_index);
                    tap_root_length = root_length_ro_sa->GetValue(box_index,0);
                    first_root_length = root_length_ro_sa->GetValue(box_index,1);
                    second_root_length = root_length_ro_sa->GetValue(box_index,2);
                    third_root_length = root_length_ro_sa->GetValue(box_index,3);
                    nitrate_uptake = nitrate_uptake_sa->GetValue(box_index);
                    water_flux0 = water_flux_sa->GetValue(box_index,0);
                    water_flux1 = water_flux_sa->GetValue(box_index,1);
                    water_flux2 = water_flux_sa->GetValue(box_index,2);
                    water_flux3 = water_flux_sa->GetValue(box_index,3);
                    // WAS:                                                         RvH
//                    switch (plant_counter)
//                    {
//                        case 1:
//                        root_length = Get_Concentration_Of_Box(Combined_Length_Index1,x,y,z);
//                        tap_root_length = Get_Concentration_Of_Box(Tap_Length_Index1,x,y,z);
//                        first_root_length = Get_Concentration_Of_Box(First_Length_Index1,x,y,z);
//                        second_root_length = Get_Concentration_Of_Box(Second_Length_Index1,x,y,z);
//                        third_root_length = Get_Concentration_Of_Box(Third_Length_Index1,x,y,z);
//                        // nitrate_uptake = Get_Concentration_Of_Box(Nitrate_Uptake_Index1,x,y,z); //µmol
//                        water_flux0 = Get_Concentration_Of_Box(Water_Flux0_Index1,x,y,z);
//                        water_flux1 = Get_Concentration_Of_Box(Water_Flux1_Index1,x,y,z);
//                        water_flux2 = Get_Concentration_Of_Box(Water_Flux2_Index1,x,y,z);
//                        water_flux3 = Get_Concentration_Of_Box(Water_Flux3_Index1,x,y,z);
//                        break;
//                        case 2:
//                        root_length = Get_Concentration_Of_Box(Combined_Length_Index2,x,y,z);
//                        tap_root_length = Get_Concentration_Of_Box(Tap_Length_Index2,x,y,z);
//                        first_root_length = Get_Concentration_Of_Box(First_Length_Index2,x,y,z);
//                        second_root_length = Get_Concentration_Of_Box(Second_Length_Index2,x,y,z);
//                        third_root_length = Get_Concentration_Of_Box(Third_Length_Index2,x,y,z);
//                        nitrate_uptake = Get_Concentration_Of_Box(Nitrate_Uptake_Index2,x,y,z); //µmol
//                        water_flux0 = Get_Concentration_Of_Box(Water_Flux0_Index2,x,y,z);
//                        water_flux1 = Get_Concentration_Of_Box(Water_Flux1_Index2,x,y,z);
//                        water_flux2 = Get_Concentration_Of_Box(Water_Flux2_Index2,x,y,z);
//                        water_flux3 = Get_Concentration_Of_Box(Water_Flux3_Index2,x,y,z);
//                        break;
//                        case 3:
//                        root_length = Get_Concentration_Of_Box(Combined_Length_Index3,x,y,z);
//                        tap_root_length = Get_Concentration_Of_Box(Tap_Length_Index3,x,y,z);
//                        first_root_length = Get_Concentration_Of_Box(First_Length_Index3,x,y,z);
//                        second_root_length = Get_Concentration_Of_Box(Second_Length_Index3,x,y,z);
//                        third_root_length = Get_Concentration_Of_Box(Third_Length_Index3,x,y,z);
//                        nitrate_uptake = Get_Concentration_Of_Box(Nitrate_Uptake_Index3,x,y,z); //µmol
//                        water_flux0 = Get_Concentration_Of_Box(Water_Flux0_Index3,x,y,z);
//                        water_flux1 = Get_Concentration_Of_Box(Water_Flux1_Index3,x,y,z);
//                        water_flux2 = Get_Concentration_Of_Box(Water_Flux2_Index3,x,y,z);
//                        water_flux3 = Get_Concentration_Of_Box(Water_Flux3_Index3,x,y,z);
//                        break;
//                        case 4:
//                        root_length = Get_Concentration_Of_Box(Combined_Length_Index4,x,y,z);
//                        tap_root_length = Get_Concentration_Of_Box(Tap_Length_Index4,x,y,z);
//                        first_root_length = Get_Concentration_Of_Box(First_Length_Index4,x,y,z);
//                        second_root_length = Get_Concentration_Of_Box(Second_Length_Index4,x,y,z);
//                        third_root_length = Get_Concentration_Of_Box(Third_Length_Index4,x,y,z);
//                        nitrate_uptake = Get_Concentration_Of_Box(Nitrate_Uptake_Index4,x,y,z); //µmol
//                        water_flux0 = Get_Concentration_Of_Box(Water_Flux0_Index4,x,y,z);
//                        water_flux1 = Get_Concentration_Of_Box(Water_Flux1_Index4,x,y,z);
//                        water_flux2 = Get_Concentration_Of_Box(Water_Flux2_Index4,x,y,z);
//                        water_flux3 = Get_Concentration_Of_Box(Water_Flux3_Index4,x,y,z);
//                        break;
//                    } //switch (plant_counter)

                    root_radius0 = root_radius_ro_sa->GetValue(box_index,0);
                    root_radius1 = root_radius_ro_sa->GetValue(box_index,1);
                    root_radius2 = root_radius_ro_sa->GetValue(box_index,2);
                    root_radius3 = root_radius_ro_sa->GetValue(box_index,3);
                    // WAS:                                                         RvH
//                    root_radius0 = Get_Concentration_Of_Box(Root_Radius0_Index,x,y,z);
//                    root_radius1 = Get_Concentration_Of_Box(Root_Radius1_Index,x,y,z);
//                    root_radius2 = Get_Concentration_Of_Box(Root_Radius2_Index,x,y,z);
//                    root_radius3 = Get_Concentration_Of_Box(Root_Radius3_Index,x,y,z);


                    LOG_ERROR_IF((root_length < 0)||(tap_root_length<0)||(first_root_length<0)||(second_root_length<0)||(third_root_length<0));
                    nitrate_amount = Get_Concentration_Of_Box(Nitrate_Amount_Index,x,y,z);
                    nitrate_before += nitrate_amount;
                    water_content = Get_Concentration_Of_Box(Water_Content_Index,x,y,z);
                    box_volume = Get_Box_Volume(x,y,z);
                    nitrate_concentration = nitrate_amount / (water_content * box_volume); //µgN/cm^3 water
                    absorption_power = (Get_Concentration_Of_Box(NAbsorption_Power_Index,x,y,z));
                    LOG_ERROR_IF(root_length < 0);
                    //impedance factor taken from Nye and Tinker, 1977, pg 78 for chloride/ProcessTemplate in sandy loam soil
                    impedance_factor = (3.129*water_content*water_content) + (0.0651*water_content);
                    LOG_ERROR_IF(impedance_factor < 0);


                    if ((root_length > 0.00000001)&&(nitrate_concentration > 0)&&(water_flux0 > 0))
                    {
                        factor10 = (-2*PI*root_radius0*absorption_power*(tap_root_length/box_volume)) / buffer_power;
                        factor11 = (-2*PI*root_radius1*absorption_power*(first_root_length/box_volume)) / buffer_power;
                        factor12 = (-2*PI*root_radius2*absorption_power*(second_root_length/box_volume)) / buffer_power;
                        factor13 = (-2*PI*root_radius3*absorption_power*(third_root_length/box_volume)) / buffer_power;
                        factor20 = (absorption_power / water_flux0);
                        factor21 = (absorption_power / water_flux1);
                        factor22 = (absorption_power / water_flux2);
                        factor23 = (absorption_power / water_flux3);
                        factor30 = (root_radius0 * water_flux0) / (diffusion_coeff * water_content * impedance_factor * buffer_power);
                        factor31 = (root_radius1 * water_flux1) / (diffusion_coeff * water_content * impedance_factor * buffer_power);
                        factor32 = (root_radius2 * water_flux2) / (diffusion_coeff * water_content * impedance_factor * buffer_power);
                        factor33 = (root_radius3 * water_flux3) / (diffusion_coeff * water_content * impedance_factor * buffer_power);
                        if (tap_root_length > 0)
                        {
                            factor40 = (1/(pow((PI*(tap_root_length/box_volume)),0.5)))/root_radius0;
                        }
                        if (first_root_length > 0)
                        {
                            factor41 = (1/(pow((PI*(first_root_length/box_volume)),0.5)))/root_radius1;
                        }
                        if (second_root_length > 0)
                        {
                            factor42 = (1/(pow((PI*(second_root_length/box_volume)),0.5)))/root_radius2;
                        }
                        if (third_root_length > 0)
                        {
                            factor43 = (1/(pow((PI*(third_root_length/box_volume)),0.5)))/root_radius3;
                        }

                        delta_amount0 = (factor10 * ((nitrate_amount) / (factor20 + (1 - factor20)*(2/(2-factor30))*(((pow(factor40, (2-factor30)))-1) / (pow(factor40,2) -1)))));
                        delta_amount1 = (factor11 * ((nitrate_amount) / (factor21 + (1 - factor21)*(2/(2-factor31))*(((pow(factor41, (2-factor31)))-1) / (pow(factor41,2) -1)))));
                        delta_amount2 = (factor12 * ((nitrate_amount) / (factor22 + (1 - factor22)*(2/(2-factor32))*(((pow(factor42, (2-factor32)))-1) / (pow(factor42,2) -1)))));
                        delta_amount3 = (factor13 * ((nitrate_amount) / (factor23 + (1 - factor23)*(2/(2-factor33))*(((pow(factor43, (2-factor33)))-1) / (pow(factor43,2) -1)))));
                        new_amount = ((delta_amount0 + delta_amount1 + delta_amount2 + delta_amount3) * dT1) + nitrate_amount; //µgN

                        LOG_ERROR_IF((delta_amount0 >0) || (delta_amount1 >0) || (delta_amount2 >0) || (delta_amount3 >0));

                        if (new_amount < 0)
                        {
                            new_amount = 0;
                        }

                        LOG_ERROR_IF(new_amount > 5000);
                        LOG_ERROR_IF(!((new_amount >= 0) && (new_amount < 5000)));

                        if ((delta_amount0 <= 0) && (delta_amount1 <= 0) && (delta_amount2 <= 0) && (delta_amount3 <= 0))
                        {
                            new_uptake = -(((delta_amount0 + delta_amount1 + delta_amount2 + delta_amount3)*dT1)/14.0067);  //µmolN
                            nitrate_uptake += new_uptake; // This is used in PlantCoordinator.cp for calculating the resource ratio.
                            LOG_ERROR_IF(nitrate_uptake == 0);
                            nitrate_after += new_amount + (new_uptake * 14.0067);

                            Set_Concentration_Of_Box(Nitrate_Amount_Index, new_amount, x,y,z);
                            Set_Concentration_Of_Box(Nitrate_Concentration_Index, (new_amount/(box_volume*water_content)), x, y, z);
                            nitrate_uptake_sa->SetValue(nitrate_uptake,box_index);

#if defined NITRATE_USE_CUMULATIVE_NITRATE_UPTAKE
                            Cumulative_Nitrate_Uptake += new_uptake;
                            //
                            // WAS:                                                 RvH
                            // Haven't transfered the Cumulative_Nitrate_UptakeX variables because
                            // didn't seem to be used.
                            switch (plant_counter)
                            {
                            case 1:
                                Set_Concentration_Of_Box(Nitrate_Uptake_Index1, (nitrate_uptake), x,y,z); //convert µg to µmol
                                Cumulative_Nitrate_Uptake1 += new_uptake;
                                break;
                            case 2:
                                Set_Concentration_Of_Box(Nitrate_Uptake_Index2, (nitrate_uptake), x,y,z);
                                Cumulative_Nitrate_Uptake2 += new_uptake;
                                break;
                            case 3:
                                Set_Concentration_Of_Box(Nitrate_Uptake_Index3, (nitrate_uptake), x,y,z);
                                Cumulative_Nitrate_Uptake3 += new_uptake;
                                break;
                            case 4:
                                Set_Concentration_Of_Box(Nitrate_Uptake_Index4, (nitrate_uptake), x,y,z);
                                Cumulative_Nitrate_Uptake4 += new_uptake;
                                break;
                            } //switch (plant_counter)
#endif // #if defined NITRATE_USE_CUMULATIVE_NITRATE_UPTAKE
                        } //if ((delta_conc0 <= 0) && (delta_conc1 <= 0) && (delta_conc2 <= 0) && (delta_conc3 <= 0))
                        else
                        {
                            LOG_ERROR << HERE << "Not all delta_amountX were <=0. 0:" << delta_amount0 << ", 1:" << delta_amount1 << ", 2:" << delta_amount2 << ", 3:" << delta_amount3;
                        }

                    } //if ((root_length) > (0.0000000001)&&(nitrate_concentration > 0))
                    else
                    {
                        nitrate_after += nitrate_amount;
                    }

                    LOG_ERROR_IF(nitrate_after != nitrate_before);
                }
            }
        }
        nitrate_uptake = 0;

//        ++plant_counter;
    }//while(plant_counter<=rootlength_number_of_plants)


    return(return_value);

}  //ProcessTemplate::UptakeN(ProcessActionDescriptor *action)

/*****************************************************************************************************/

double ProcessTemplate::CalculateNitrateMineralisation(ProcessActionDescriptor *action, long int dT)
{
    Use_ReturnValue;
    Use_ScoreboardCoordinator;
    Use_ProcessCoordinator;
    Use_Time;
    Use_Scoreboard;
    Use_Boundaries;

    double mineralisation_rate11 = 3.10e-5;
    double mineralisation_rate21 = 0.90e-5;
    double mineralisation_rate31 = 1.25e-5;
    double mineralisation_rate41 = 5.74e-5;
    double mineralisation_rate51 = 3.77e-5;
    double mineralisation_rate61 = 2.79e-5;
    double mineralisation_rate71 = 7.95e-6;

    double box_width, box_depth, mineralised_amount,nitrate_amount;

    long num_x, num_y, num_z;                    // number of layers in each dimension
    Get_Number_Of_Layers(num_x,num_y,num_z);    // How many boxes do we have?

    long int x,y,z;

    for (y=1; y<=num_y; ++y)
    {
        for (z=1; z<=num_z; ++z)
        {
            for (x=1; x<=num_x; ++x)
            {
                box_width = Get_Layer_Width(x);
                box_depth = Get_Layer_Depth(y);
                mineralised_amount = 0;
                //14/08/00 VMD this is a small basic routine that adds in mineralised ProcessTemplate for modelling the Anderson et al data. The mineralisation
                //rates are taken straight from the Anderson et al papers.
                //Once the N mineralisation routine is in this can be removed.
                nitrate_amount = Get_Concentration_Of_Box(Nitrate_Amount_Index,x,y,z);
                if ((time > 0) && (time <= 4060800))
                {
                    if (z == 1)
                    {
                        mineralised_amount = (mineralisation_rate11 * dT);
                        if (nitrate_amount > 1500)
                        {
                            mineralised_amount = 0;
                        }
                    }
                }
                if ((time > 4060800) && (time <= 7948800))
                {
                    if (z <= 2)
                    {
                        mineralised_amount = mineralisation_rate21 * dT;
                        if (nitrate_amount > 1500)
                        {
                            mineralised_amount = 0;
                        }
                    }
                }
                if ((time > 7948800) && (time <= 10368000))
                {
                    if (z <= 2)
                    {
                        mineralised_amount = mineralisation_rate31 * dT;
                        if (nitrate_amount > 1500)
                        {
                            mineralised_amount = 0;
                        }
                    }
                }
                if ((time > 10368000) && (time <= 12787200))
                {
                    if (z <= 1)
                    {
                        mineralised_amount = mineralisation_rate41 * dT;
                        if (nitrate_amount > 1500)
                        {
                            mineralised_amount = 0;
                        }
                    }
                }
                if ((time > 12787200) && (time <= 15120000))
                {
                    if (z <= 2)
                    {
                        mineralised_amount = mineralisation_rate51 * dT;
                        if (nitrate_amount > 1500)
                        {
                            mineralised_amount = 0;
                        }
                    }
                }
                if ((time > 15120000) && (time <= 16502400))
                {
                    if (z <= 2)
                    {
                        mineralised_amount = mineralisation_rate61 * dT;
                        if (nitrate_amount > 1500)
                        {
                            mineralised_amount = 0;
                        }
                    }
                }
                if ((time > 16502400) && (time <= 19785600))
                {
                    if (z <= 2)
                    {
                        mineralised_amount = mineralisation_rate71 * dT;
                        if (nitrate_amount > 1500)
                        {
                            mineralised_amount = 0;
                        }
                    }
                }
                nitrate_amount += mineralised_amount*box_width*box_depth;
#if defined NITRATE_USE_CUMULATIVE_MINERALISATION
                Cumulative_Mineralisation += mineralised_amount*box_width*box_depth;
                if (time <= 7862400)
                {
                    Cumulative1_Mineralisation = Cumulative_Mineralisation;
                }
                if (time <= 10281600)
                {
                    Cumulative2_Mineralisation = Cumulative_Mineralisation;
                }
                if (time <= 10972800)
                {
                    Cumulative3_Mineralisation = Cumulative_Mineralisation;
                }
                if (time <= 12700800)
                {
                    Cumulative4_Mineralisation = Cumulative_Mineralisation;
                }
                if (time <= 13132800)
                {
                    Cumulative5_Mineralisation = Cumulative_Mineralisation;
                }
                if (time <= 15120000)
                {
                    Cumulative6_Mineralisation = Cumulative_Mineralisation;
                }
                if (time <= 15465600)
                {
                    Cumulative7_Mineralisation = Cumulative_Mineralisation;
                }
                if (time <= 16416000)
                {
                    Cumulative8_Mineralisation = Cumulative_Mineralisation;
                }
#endif // #if defined NITRATE_USE_CUMULATIVE_MINERALISATION
                Set_Concentration_Of_Box(Nitrate_Amount_Index, nitrate_amount, x,y,z);


            }
        }
    }


    return (return_value);
} //ProcessTemplate::CalculateMineralisation



/* ******************************************************************************* */

//VMD 4/10/98 Separated this dot redrawing routine from ProcessTemplate uptake.  Both N diffusion and N uptake result in a change in the
// ProcessTemplate concentration in boxes hence requiring the dots to be redrawn on the screen.  Separating this routine out allows any
// routine to call a dot redraw.
//VMD 2/11/99 This routine as it stands can only be used for those routines that change the ProcessTemplate concentration of a box and hence
// the number of dots needs to be adjusted - ie N diffusion and N uptake.  This routine cannot be used for MassFlow and Leach which change the distribution
// of the dots and need a resultant recalculation of the ProcessTemplate concentration in each scoreboard box.  I have added some more comments to this
// routine.
//VMD 12/07/02, 19/07/02 Have done some recoding to account for the new pNitrate structure of (s,x,y,z).  Now track not only nitrates of whole packet size,
// but also nitrates of a lesser size that were the left over.

long int ProcessTemplate::RedistributeDots(ProcessActionDescriptor *action)
{
    Use_ReturnValue;
    Use_Time;
    Use_ScoreboardCoordinator;
    Use_ProcessCoordinator;
    Use_Scoreboard;
    Use_Boundaries;

    long int new_number_whole_nitrate_packets;
    long int old_number_whole_nitrate_packets, number_part_nitrate_packets, x, y, z;
    BoxCoordinate box_coord;
    BoxCoordinate * box_coords = &box_coord;

    double box_front, box_depth, box_height, box_left, box_width, box_top, nitrate_amount, left_over_nitrate;

    // Allocate memory for the array that stores the numbers of ProcessTemplate packets per box
    // Arrange to fail if it can't be done, cause if we can't even set aside space for THAT,
    // we certainly won't have enough memory for the allocations to come.
    long num_x, num_y, num_z;                    // number of layers in each dimension
    Get_Number_Of_Layers(num_x,num_y,num_z);    // How many boxes do we have?

    //all dots of m_NITRATE_PACKET_SIZE
    long int * number_of_whole_packets_array = new long int[num_x*num_y*num_z];
    RmAssert(0!=number_of_whole_packets_array, "Could not allocate memory for number_of_whole_packets_array");

    //all dots containing the leftover ProcessTemplate
    long int * number_of_part_packets_array = new long int[num_x*num_y*num_z];
    RmAssert(0!=number_of_part_packets_array, "Could not allocate memory for number_of_part_packets_array");

    const BoxIndex total = (num_x*num_y*num_z);

    for (BoxIndex box_index = 0; box_index<total; ++box_index)
    {
        number_of_whole_packets_array[box_index] = 0; // these arrays store the number of ProcessTemplate packets that are in each scoreboard box
    }
    for (BoxIndex box_index = 0; box_index<total; ++box_index)
    {
        number_of_part_packets_array[box_index] = 0; // these arrays store the number of part ProcessTemplate packets that are in each scoreboard box
    }

    counter = 0;

    //  This routine counts up how many ProcessTemplate packets are in each box.
    for(size_t counter=0; counter<Total_Nitrates; ++counter)
    {
        pNitrateArray->GetCoordinate(counter, pNitrate);

        if ((pNitrate->x != 0)&&(pNitrate->y != 0)&&(pNitrate->z != 0))
        {
            //Need to count how many ProcessTemplate packets are in each box.  To do this every box in the scoreboard is given a number
            //which corresponds to the index in an array. The number of the first box is zero and the order of numbering is ZXY.

            scoreboardcoordinator->FindBoxBySoilCoordinate(box_coords, scoreboard, pNitrate);
            BoxIndex box_index = 0;

            if ((box_coords->y) == 1)
            {

                if (box_coords->x == 1)
                {
                    box_index = (box_coords->z) -1;
                }

                if ((box_coords->x) > 1)
                {
                    if (box_coords->x == num_x)
                    {
                        box_index = (((box_coords->y) * (((box_coords->x)-1)*num_z)) + (box_coords->z)) -1;
                    }

                    else
                    {
                        box_index = (((box_coords->y) * (((box_coords ->x) - 1)*num_z)) + box_coords->z)-1;
                    }
                }

                // there shouldn't be a box index number higher than the number of boxes in the scoreboard
                LOG_ERROR_IF(box_index >= (num_x*num_y*num_z));
                RmAssert( box_index<(num_x*num_y*num_z), "BoxIndex is greater than #boxes in scoreboard");

                if (pNitrate->s == m_NITRATE_PACKET_SIZE)
                {
                    number_of_whole_packets_array[box_index] += 1;
                    new_number_whole_nitrate_packets = number_of_whole_packets_array[box_index];
                }
                if (pNitrate->s != m_NITRATE_PACKET_SIZE)
                {
                    number_of_part_packets_array[box_index] += 1;
                    new_number_whole_nitrate_packets = number_of_whole_packets_array[box_index];
                }

            } //if (y = 1)

            else
            {
                if (box_coords->x == 1)
                {
                    box_index = ((((box_coords->y)-1) * num_x * num_z) - 1) + (box_coords->z);
                }

                else
                {
                    box_index = ((((box_coords->y)-1) * num_x * num_z) - 1) +  ((((box_coords ->x) - 1) * num_z) + (box_coords ->z));
                }

                if (pNitrate->s == m_NITRATE_PACKET_SIZE)
                {
                    number_of_whole_packets_array[box_index] += 1;
                    new_number_whole_nitrate_packets = number_of_whole_packets_array[box_index];
                }
                if (pNitrate->s != m_NITRATE_PACKET_SIZE)
                {
                    number_of_part_packets_array[box_index] += 1;
                    new_number_whole_nitrate_packets = number_of_whole_packets_array[box_index];
                }
            }
        } //if((pNitrate->x != 0)&&(pNitrate->y != 0)&&(pNitrate->z != 0))
    }

    new_number_whole_nitrate_packets = 0;
    old_number_whole_nitrate_packets = 0;
    number_part_nitrate_packets = 0;
    left_over_nitrate = 0;

    nitrate_before = 0;
    nitrate_after = 0;

    //If the number of ProcessTemplate packets in any particular scoreboard box has changed then dots need to be removed from or added to the screen.
    for (y=1; y<=num_y; ++y)
    {
        for (x=1; x<=num_x; ++x)
        {
            for (z=1; z<=num_z; ++z)
            {
                nitrate_amount = Get_Concentration_Of_Box(Nitrate_Amount_Index,x,y,z);
                nitrate_before += nitrate_amount;
                new_number_whole_nitrate_packets = nitrate_amount / m_NITRATE_PACKET_SIZE;  //based on the current amount of ProcessTemplate
                left_over_nitrate = (nitrate_amount) - (new_number_whole_nitrate_packets * m_NITRATE_PACKET_SIZE);
                old_number_whole_nitrate_packets = number_of_whole_packets_array[box_index]; //ProcessTemplate dots that use to be in this box
                number_part_nitrate_packets = number_of_part_packets_array[box_index]; //part ProcessTemplate dots in this box

                box_front = Get_Layer_Front(y);
                box_depth = Get_Layer_Depth(y);
                box_left = Get_Layer_Left(x);
                box_width = Get_Layer_Width(x);
                box_height = Get_Layer_Height(z);
                box_top = Get_Layer_Top(z);

                if (((number_part_nitrate_packets == 0)&&(left_over_nitrate > 0)) || (old_number_whole_nitrate_packets < new_number_whole_nitrate_packets))
                {
                    for(size_t counter=0; counter<Total_Nitrates; ++counter)
                    {
                        pNitrateArray->GetCoordinate(counter, pNitrate);
                        if ((pNitrate->x == 0)&&(pNitrate->y == 0)&&(pNitrate->z == 0)) //is a blank ProcessTemplate, can use it to add dots if needed
                        {
                            //add part ProcessTemplate packets if needed
                            if ((number_part_nitrate_packets == 0)&&(left_over_nitrate > 0))
                                //there is a left over amount, but no part ProcessTemplate to put it in
                            {
                                //Will use this zeroed ProcessTemplate to add a part ProcessTemplate to the box.

                                /* Use random coordinates for allocating to ProcessTemplate packets.
                                To generate the random numbers, Random1() from Utilities.h is used */
                                pNitrate->x = box_left + (Random1() * box_width);
                                pNitrate->y = box_front + (Random1() * box_depth);
                                pNitrate->z = box_top + (Random1() * box_height);
                                pNitrate->s = left_over_nitrate; //we are making a new part packet
                                left_over_nitrate = 0;
                                number_of_part_packets_array[box_index] = 1;
                                number_part_nitrate_packets = 1;

                                //I was having problems with ProcessTemplate packets falling exactly on a box edge
                                //and the computer not knowing which box it was in.
                                if (pNitrate->z == box_top) {pNitrate->z += 0.01;}
                                if (pNitrate->z == (box_top + box_height)) {pNitrate->z -= 0.01;}
                                if (pNitrate->x == box_left) {pNitrate->x += 0.01;}
                                if (pNitrate->x == (box_left + box_width)) {pNitrate->x -= 0.01;}
                                if (pNitrate->y == box_front) {pNitrate->y += 0.01;}
                                if (pNitrate->y == (box_front+box_depth)) {pNitrate->y -= 0.01;}

                                pNitrateArray->SetCoordinate(counter, pNitrate);
                                BEGIN_DRAWING
                                drawing.RemoveDot(*pNitrate);
                                END_DRAWING
                                //this method tracks all non-zero amounts, but don't want to see dots of stupidly small size
                                //so don't show any part nitrates, only ones of m_NITRATE_PACKET_SIZE
                            } //if((number_part_nitrate_packets == 0)&&(left_over_nitrate > 0))

                            if ((pNitrate->x == 0)&&(pNitrate->y == 0)&&(pNitrate->z == 0))
                            {
                                if (old_number_whole_nitrate_packets < new_number_whole_nitrate_packets) //ProcessTemplate has been added
                                {
                                    //Will use this zeroed ProcessTemplate to add to the box.
                                    /* Make the linked list of random coordinates for allocating to ProcessTemplate packets.
                                    To generate the random numbers, Random1() from Utilities.h is used */
                                    pNitrate->x = box_left + (Random1() * box_width);
                                    pNitrate->y = box_front + (Random1() * box_depth);
                                    pNitrate->z = box_top + (Random1() * box_height);
                                    pNitrate->s = m_NITRATE_PACKET_SIZE; //we are making a new whole packet
                                    number_of_whole_packets_array[box_index] +=1;
                                    old_number_whole_nitrate_packets += 1;

                                    //I was having problems with ProcessTemplate packets falling exactly on a box edge
                                    //and the computer not knowing which box it was in.
                                    if (pNitrate->z == box_top) {pNitrate->z += 0.01;}
                                    if (pNitrate->z == (box_top + box_height)) {pNitrate->z -= 0.01;}
                                    if (pNitrate->x == box_left) {pNitrate->x += 0.01;}
                                    if (pNitrate->x == (box_left + box_width)) {pNitrate->x -= 0.01;}
                                    if (pNitrate->y == box_front) {pNitrate->y += 0.01;}
                                    if (pNitrate->y == (box_front+box_depth)) {pNitrate->y -= 0.01;}

                                    // set the array value
                                    pNitrateArray->SetCoordinate(counter, pNitrate);
                                    BEGIN_DRAWING
                                    drawing.DrawDot(*pNitrate);
                                    END_DRAWING
                                } //if((pNitrate->x == 0)&&(pNitrate->y == 0)&&(pNitrate->z == 0))
                            } //if((pNitrate->x == 0)&&(pNitrate->y == 0)&&(pNitrate->z == 0))
                        }
                    }
                } //if(((number_part_nitrate_packets == 0)&&(left_over_nitrate > 0)) || (old_number_whole_nitrate_packets < new_number_whole_nitrate_packets))

                ++box_index;

            } //for z
        } //for x
    } //for y


    for(size_t counter=0; counter<Total_Nitrates; ++counter)
    {
        pNitrateArray->GetCoordinate(counter, pNitrate);

        if ((pNitrate->x != 0)&&(pNitrate->y != 0)&&(pNitrate->z != 0))
        {
            nitrate_after += pNitrate->s;
            scoreboardcoordinator->FindBoxBySoilCoordinate(box_coords, scoreboard, pNitrate);
            nitrate_amount = Get_Concentration_Of_Box(Nitrate_Amount_Index,box_coords ->x,box_coords->y,box_coords ->z);
            LOG_ERROR_IF(nitrate_amount > 5000);

            new_number_whole_nitrate_packets = nitrate_amount / m_NITRATE_PACKET_SIZE;  //based on the current amount of ProcessTemplate
            LOG_ERROR_IF(new_number_whole_nitrate_packets > 100);

            left_over_nitrate = (nitrate_amount) - (new_number_whole_nitrate_packets * m_NITRATE_PACKET_SIZE);
            LOG_ERROR_IF(left_over_nitrate < 0);

            if ((box_coords->y) == 1) //calculate the box index associate with this ProcessTemplate packet
            {
                if (box_coords->x == 1)
                {
                    box_index = (box_coords->z) -1;
                }

                if ((box_coords->x) > 1)
                {
                    if (box_coords->x == num_x)
                    {
                        box_index = (((box_coords->y) * (((box_coords->x)-1)*num_z)) + (box_coords->z)) -1;
                    }

                    else
                    {
                        box_index = (((box_coords->y) * (((box_coords ->x) - 1)*num_z)) + box_coords->z)-1;
                    }
                }

                // there shouldn't be a box index number higher than the number of boxes in the scoreboard
                LOG_ERROR_IF(box_index >= (num_x*num_y*num_z));
                RmAssert( box_index<(num_x*num_y*num_z), "BoxIndex is greater than #boxes in scoreboard");

            } //if (y = 1)

            else
            {
                if (box_coords->x == 1)
                {
                    box_index = ((((box_coords->y)-1) * num_x * num_z) - 1) + (box_coords->z);
                }

                else
                {
                    box_index = ((((box_coords->y)-1) * num_x * num_z) - 1) +  ((((box_coords ->x) - 1) * num_z) + (box_coords ->z));
                }
            } //else


            old_number_whole_nitrate_packets = number_of_whole_packets_array[box_index]; //ProcessTemplate dots that use to be in this box
            number_part_nitrate_packets = number_of_part_packets_array[box_index]; //part ProcessTemplate dots in this box

            LOG_ERROR_IF((old_number_whole_nitrate_packets < 0) || (new_number_whole_nitrate_packets < 0) || (number_part_nitrate_packets < 0));

            if (pNitrate->s != m_NITRATE_PACKET_SIZE)
            {
                if (number_part_nitrate_packets == 1)
                {
                    if (left_over_nitrate > 0)
                    {
                        pNitrate->s = left_over_nitrate;
                        left_over_nitrate = 0;
                    }
                    else //there is not left over amount, get rid of this dot.
                    {
                        BEGIN_DRAWING
                        drawing.RemoveDot(*pNitrate); //remove a dot
                        END_DRAWING
                        pNitrate->x = 0;
                        pNitrate->y = 0;
                        pNitrate->z = 0;
                        pNitrate->s = 0;
                        pNitrateArray->SetCoordinate(counter, pNitrate);
                        number_of_part_packets_array[box_index] -=1;
                    } //else
                } //if (number_part_nitrate_packets == 1)
                else //there is more than 1 part packet in this box, delete the extra and make the remaining one equal to the left over amount
                {
                    BEGIN_DRAWING
                    drawing.RemoveDot(*pNitrate); //remove a dot
                    END_DRAWING
                    pNitrate->x = 0;
                    pNitrate->y = 0;
                    pNitrate->z = 0;
                    pNitrate->s = 0;
                    pNitrateArray->SetCoordinate(counter, pNitrate);
                    number_of_part_packets_array[box_index] -=1;
                } //else
            } //if (pNitrate->s != m_NITRATE_PACKET_SIZE)


            if (pNitrate->s == m_NITRATE_PACKET_SIZE)
            {
                if (old_number_whole_nitrate_packets > new_number_whole_nitrate_packets) // ProcessTemplate has been lost
                {
                    BEGIN_DRAWING
                    drawing.RemoveDot(*pNitrate); //remove a dot
                    END_DRAWING
                    pNitrate->x = 0;
                    pNitrate->y = 0;
                    pNitrate->z = 0;
                    pNitrate->s = 0;

                    pNitrateArray->SetCoordinate(counter, pNitrate);
                    number_of_whole_packets_array[box_index] -=1;
                }
            }  //if ((pNitrate->s == m_NITRATE_PACKET_SIZE))
        } //if((pNitrate->x != 0)&&(pNitrate->y != 0)&&(pNitrate->z != 0))

    } //while ((counter < Total_Nitrates) && ((old_number_whole_nitrate_packets < new_number_whole_nitrate_packets)))


    TCLForgetPtr(number_of_whole_packets_array);
    TCLForgetPtr(number_of_part_packets_array);


    return (return_value);
}


bool ProcessTemplate::DoesOverride() const
{
    return (true);
}
bool ProcessTemplate::DoesDrawing()
{
    return (true);
}

} /* namespace rootmap */
