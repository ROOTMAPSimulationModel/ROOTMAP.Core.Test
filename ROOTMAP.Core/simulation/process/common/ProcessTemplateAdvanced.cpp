

/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessTemplateAdvanced.cpp
// Purpose:     Implementation of the ProcessTemplateAdvanced class
// Created:     22-03-2009
// Author:      RvH
// $Date: 2009-02-25 04:09:59 +0900 (Wed, 25 Feb 2009) $
// $Revision: 35 $
// Copyright:   ©2002-2009 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

// Typically, your Process derived class will be in the following location
#include "simulation/process/modules/ProcessTemplateAdvanced.h"

// Required if drawing
#include "simulation/process/common/ProcessDrawing.h"

// If you register for plant summaries, you'll want these
#include "simulation/process/plant/PlantSummaryRegistration.h"
#include "simulation/process/plant/PlantSummaryHelper.h"

// Shared Attributes - read the doco
#include "simulation/process/shared_attributes/ProcessSharedAttributeOwner.h"
#include "simulation/process/shared_attributes/ProcessAttributeSupplier.h"
#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"
#include "simulation/process/shared_attributes/SharedAttributeManager.h"
#include "simulation/process/shared_attributes/SharedAttributeSearchHelper.h"
#include "simulation/process/shared_attributes/SharedAttribute.h"

// Common requirements
#include "core/common/DoubleCoordinates.h"
#include "core/common/RmAssert.h"
#include "core/common/ExtraStuff.h"
#include "core/scoreboard/BoundaryArray.h"
#include "core/utility/Utility.h"


// Useful macro, example:
// LOG_ERROR_IF(value<0);
#define LOG_ERROR_IF(s) if ( (s) ) { RootMapLogError(LOG_LINE "Encountered error in logic : " #s); }


namespace rootmap
{


// Put constants here as const variables, not preprocessor macros
const double SomeConstant = 1.00;
const int SomeOtherInteger = 1;

// Causes the logger static var to be initialised
RootMapLoggerDefinition(ProcessTemplateAdvanced);

// wxWidgets stuff, so the process can be dynamically instantiated by name
IMPLEMENT_DYNAMIC_CLASS(ProcessTemplateAdvanced, Process)

// Oh hai, i'm in ur constructor
ProcessTemplateAdvanced::ProcessTemplateAdvanced()
: mySharedAttributeOwner(NULL)
, myWaterModuleID(IdentifierUtility::InvalidIdentifier)
, myPlantCoordinatorModuleID(IdentifierUtility::InvalidIdentifier)
{
    RootMapLoggerInitialisation("rootmap.ProcessTemplateAdvanced");

    // Here is where your attribute owner is initialised
    mySharedAttributeOwner = NEW ProcessSharedAttributeOwner("ProcessTemplateAdvanced",this);
    SharedAttributeRegistrar::RegisterOwner(mySharedAttributeOwner);

    // And the supplier of any attributes you may have
    ProcessAttributeSupplier * supplier = NEW ProcessAttributeSupplier(this, "ProcessTemplateAdvanced");
    SharedAttributeRegistrar::RegisterSupplier(supplier);
}

ProcessTemplateAdvanced::~ProcessTemplateAdvanced()
{
    // use wxDELETE - sets the var to null after calling delete
    // Arrays declared as "int myArray[]" should use wxDELETEA
    wxDELETE(myPointerDataMember);

    // Owners are deleted as part of Engine destruction
    // wxDELETE(mySharedAttributeOwner);
}


//
// 
//
long int ProcessTemplateAdvanced::Register(ProcessActionDescriptor *action)
{
//#if defined SAMPLE_PROCESSTEMPLATE_REGISTER
    Use_ProcessCoordinator;

    // the second arg to the constructor is the PlantCoordinator. The helper
    // will find it if you don't provide it.
    PlantSummaryHelper helper(processcoordinator, 0);

    // Per-Plant registration
    PlantSummaryRegistration *psr = helper.MakePlantSummaryRegistration(action);
    // per-box means a scoreboard summary (not global/single-value)
    psr->RegisterByPerBox(true);
    psr->RegisterAllPlantCombos();
    psr->RegisterAllBranchOrders();
    psr->RegisterSummaryType(srRootLength);
    psr->RegisterModule((Process*)this);
    helper.SendPlantSummaryRegistration(psr, action, this);

    // Per-Plant Per-RootOrder Per-Box (ie. scoreboard)
    PlantSummaryRegistration *psr2 = helper.MakePlantSummaryRegistration(action);
    psr2->RegisterByPerBox(true);
    psr2->RegisterAllPlantCombos();
    psr2->RegisterAllBranchOrderCombos();
    psr2->RegisterSummaryType(srRootLength);
    psr2->RegisterModule((Process*)this);
    helper.SendPlantSummaryRegistration(psr2, action, this);

//#endif // #if defined SAMPLE_PROCESSTEMPLATE_REGISTER

    return kNoError;
}

/* ***************************************************************************
*/
long int ProcessTemplateAdvanced::Initialise(ProcessActionDescriptor *action)
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

    // allows us to track 
    SetTimeDeltaTracking(kNormalMessage);
    SetTimeDeltaTracking(kGeneralPurposeMessage0);

    PrepareSharedAttributes(action);

    Process::Initialise(action);
#endif defined PROCESSTEMPLATE_SAMPLE_INITIALISE

    return (return_value);

} // long int ProcessTemplateAdvanced::Initialise(ProcessActionDescriptor *action)

void ProcessTemplateAdvanced::PrepareSharedAttributes(ProcessActionDescriptor * action)
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

long int ProcessTemplateAdvanced::StartUp(ProcessActionDescriptor *action)
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

long int ProcessTemplateAdvanced::DoGeneralPurpose(ProcessActionDescriptor *action)
{
    Use_ReturnValue;
    // we want to know who sent the wake up, so we can decide how to act.
    Use_Source;
    Use_Code;

    ProcessIdentifier source_module_id = IdentifierUtility::InvalidIdentifier;
    if (source != NULL)
    {
        source_module_id = source->GetProcessID();
    }

    // now check if it is the water module
    if (source_module_id == myWaterModuleID)
    {
        // new code for this is in DoGeneralPurpose(...)
    }
    else if (source_module_id == myPlantModuleID)
    {
    }

    return (return_value);
}

/*
 * DoExternalWakeUp
 *  Called when a 'XWak' message is received. Sent by another process module,
 *  to wake "this" one up to do some processing based on the other process'
 *  action[s]. This default method does nothing, you'll need to override it
 *  for your process to accept wake up calls from other process modules.
 */
long int ProcessTemplateAdvanced::DoExternalWakeUp(ProcessActionDescriptor *action)
{
    Use_ReturnValue;
    return (return_value);
}

/* ************************************************************************** */


long int ProcessTemplateAdvanced::DoNormalWakeUp(ProcessActionDescriptor *action)
{
    Use_ReturnValue;

/*
    return_value = DoSomeNormalStuff(action);
*/

    return_value |= Process::DoNormalWakeUp(action);
    return (return_value);
}

/* ************************************************************************** */

long int ProcessTemplateAdvanced::DoScoreboardPlantSummaryRegistered(ProcessActionDescriptor *action)
{
    Use_PlantSummaryRegistration;

    my_first_plant_summary_index = psr->GetStartIndex();
    my_number_of_plants = psr->GetNumberOfPlants();
    my_number_of_branch_orders = psr->GetNumberOfBranchOrders();

    return (Process::DoScoreboardPlantSummaryRegistered(action));
}



void ProcessTemplateAdvanced::DrawScoreboard(const DoubleRect & area, Scoreboard * scoreboard)
{
    BEGIN_DRAWING
    for (MyArray::iterator iter(myArray.begin());
            iter!=myArray.end() ; ++iter)
    {
        DoubleCoordinate coord = *iter;

        // if any of x,y,z are negative, we are erasing the feature
        if ( (coord.x<0.0) || (coord.y<0.0) || (coord.z<0.0) )
        {
            drawing.DrawDot(coord, *wxWHITE);
        }
        else
        {
            drawing.DrawDot(coord);
        }
    }
    END_DRAWING
}


bool ProcessTemplateAdvanced::DoesOverride() const
{
    return (true);
}
bool ProcessTemplateAdvanced::DoesDrawing() const
{
    return (true);
}

} /* namespace rootmap */
