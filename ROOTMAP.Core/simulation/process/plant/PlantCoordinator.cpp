/*
PlantCoordinator
The class which handles Plant Summaries, Plant Types and manages the “Plant” Menu.

At the moment, this Process is in amongst the Simulation's d[A|B]Processs lists.

REASONS for tracking plant characteristics here :
* A plant characteristic is not a summary of just one plant.
* When a plant dies/is removed we need some robust way of making sure all the characteristics are valid.
* Each plant still registers its own TimeStep/WatchEvent requirements with the clock, but
    registers its characteristic through this devicerola.

REASONS for tracking plant types here :
* When a plant type (PLty) is modified, we need to BroadcastChange() to all the plants.
* When a PLty is deleted, we need to gracefully and robustly remove all plants of that type.

REASONS for managing the Plant Menu :
* We handle everything to do with all the items in the plant menu - so it makes perfect sense.

Pros:
Plant behaviour is consistent and robust.

Cons:
None, that i can detect

30-August-1999 RvH searching for plants safer, saving to simulation file halfway there.
*/


#include "core/common/Constants.h"
#include "core/common/RmAssert.h"
//#include "core/common/CropSimCommands.h"
#include "core/macos_compatibility/MacResourceManager.h"
#include "core/utility/Utility.h"

#include "simulation/common/IdentifierUtility.h"
#include "simulation/common/SimulationEngine.h"

#include "simulation/data_access/common/DataAccessManager.h"

#include "simulation/process/common/ProcessCoordinator.h"
#include "simulation/process/common/ProcessUtility.h"
#include "simulation/process/plant/PlantCoordinator.h"
#include "simulation/process/plant/Plant.h"
#include "simulation/process/plant/PlantType.h"
#include "simulation/process/plant/PlantSummary.h"
#include "simulation/process/plant/PlantSummaryHelper.h"
#include "simulation/process/plant/Axis.h"
#include "simulation/process/plant/Branch.h"
#include "simulation/process/plant/RootTip.h"
#include "simulation/process/plant/PlantSummaryRegistration.h"
#include "simulation/process/plant/PlantSummarySupplier.h"

#include "simulation/process/shared_attributes/PlantSharedAttributeSupport.h"
#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"
#include "simulation/process/shared_attributes/ProcessAttributeSupplier.h"
#include "simulation/process/shared_attributes/ProcessSharedAttributeOwner.h"


#include "simulation/scoreboard/Scoreboard.h"
#include "simulation/scoreboard/ScoreboardCoordinator.h"
#include "simulation/process/modules/VolumeObjectCoordinator.h"
#include "simulation/process/modules/VolumeObject.h"

#include "core/common/ExtraStuff.h"

#include <limits>
#include <exception>


#define PLANT_ITERATION_BEGIN for (PlantArray::iterator p_iter = m_plants.begin() ; p_iter != m_plants.end() ; ++p_iter) { Plant * plant_ = (*p_iter);
#define PLANT_ITERATION_END }
#define PLANT_CONST_ITERATION_BEGIN for (PlantArray::const_iterator p_iter = m_plants.begin() ; p_iter != m_plants.end() ; ++p_iter) { const Plant * plant_ = (*p_iter);
#define PLANT_CONST_ITERATION_END }

#define PLANT_TYPE_ITERATION_BEGIN for (PlantTypeArray::iterator pt_iter = m_plantTypes.begin() ; pt_iter != m_plantTypes.end() ; ++pt_iter) { PlantType * plant_type_ = (*pt_iter);
#define PLANT_TYPE_ITERATION_END }

#define PLANT_SUMMARY_ITERATION_BEGIN for (PlantSummaryArray::iterator ps_iter = m_plantSummariesALL.begin() ; ps_iter != m_plantSummariesALL.end() ; ++ps_iter) { PlantSummary * plant_summary_ = (*ps_iter);
#define PLANT_SUMMARY_ITERATION_END }

#define PLANT_SUMMARY_REGISTRATION_ITERATION_BEGIN for ( PlantSummaryRegistrationSet::iterator psr_iter = m_plantSummaryRegistrations.begin() ; psr_iter != m_plantSummaryRegistrations.end() ; ++psr_iter) { PlantSummaryRegistration * plant_summary_registration_ = (*psr_iter);
#define PLANT_SUMMARY_REGISTRATION_ITERATION_END }

namespace rootmap
{
    IMPLEMENT_DYNAMIC_CLASS(PlantCoordinator, ProcessGroupLeader)
        RootMapLoggerDefinition(PlantCoordinator);

    // Now in MathLib
    // const double PI = 3.141592654;

    PlantCoordinator::PlantCoordinator()
        : ProcessGroupLeader()
        , plant_minimum(0.0)
        , plant_default(0.0)
        , plant_maximum(Utility::infinity())
        , maximum_root_branching_order(3)
        , plant_growth_time_step(PostOffice::CalculateTimeSpan(0, 0, 0, 4, 0, 0)) // 4 hours
        , reference_temperature(15)
        , temperature_index(-1)
        , m_previousWakingTimeStamp(0)
        , time_betw_responses(0)
        , mySharedAttributeOwner(__nullptr)
        , m_scoreboardCoordinator(__nullptr)
        , m_volumeObjectCoordinator(__nullptr)
        , m_soil_scoreboard_Issue15(__nullptr)
    {
        RootMapLoggerInitialisation("rootmap.PlantCoordinator");
        PlantSharedAttributeVariation* v1 = new PlantSharedAttributeVariation("Plant", this);
        PlantTypeSharedAttributeVariation* v2 = new PlantTypeSharedAttributeVariation("PlantType", this);
        RootOrderSharedAttributeVariation* v3 = new RootOrderSharedAttributeVariation("RootOrder", 4);

        SharedAttributeRegistrar::RegisterVariation(v1);
        SharedAttributeRegistrar::RegisterVariation(v2);
        SharedAttributeRegistrar::RegisterVariation(v3);

        mySharedAttributeOwner = new ProcessSharedAttributeOwner("PlantCoordinator", this);
        SharedAttributeRegistrar::RegisterOwner(mySharedAttributeOwner);

        ProcessAttributeSupplier* s1 = new ProcessAttributeSupplier(this, "PlantCoordinator");
        SharedAttributeRegistrar::RegisterSupplier(s1);
    }


    void PlantCoordinator::SetVolumeObjectCoordinator(const VolumeObjectCoordinator& voc)
    {
        m_volumeObjectCoordinator = &voc;

        // MSA TODO: Ascertain whether it's too late to register this variation here
        if (m_volumeObjectCoordinator != __nullptr)
        {
            VolumeObjectSharedAttributeVariation* v4 = new VolumeObjectSharedAttributeVariation("VolumeObject", m_volumeObjectCoordinator->GetVolumeObjectList().size() + 1);
            SharedAttributeRegistrar::RegisterVariation(v4);
        }
    }

    void PlantCoordinator::Initialise(SimulationEngine& engine)
    {
        DataAccessManager& dam = engine.GetDataAccessManager();
        m_scoreboardCoordinator = &(engine.GetScoreboardCoordinator());

        dam.constructPlants(this);
    }

    /* ~PlantCoordinator
    Dispose of the list of Summaries… and the Plants !! */
    PlantCoordinator::~PlantCoordinator()
    {
        // delete plant summaries
        PLANT_SUMMARY_ITERATION_BEGIN
            delete plant_summary_;
        PLANT_SUMMARY_ITERATION_END

            // delete plant summary registrations
            PLANT_SUMMARY_REGISTRATION_ITERATION_BEGIN
            delete plant_summary_registration_;
        PLANT_SUMMARY_REGISTRATION_ITERATION_END

            // delete plants
            // On the other hand, plants are added to m_processListALL, which is now
            // the list iterated over for deletion
            //PLANT_ITERATION_BEGIN
            //delete plant_;
            //PLANT_ITERATION_END

            // delete plant types
            PLANT_TYPE_ITERATION_BEGIN
            delete plant_type_;
        PLANT_TYPE_ITERATION_END
    }

    bool PlantCoordinator::DoesOverride() const
    {
        return (true);
    }


    long PlantCoordinator::GetNumPlants()
    {
        return (m_plants.size());
    }

    long int PlantCoordinator::GetNumPlantTypes()
    {
        return (m_plantTypes.size());
    }

    /* GetPlant
    By its index in the itsPlants list. This is the easy method. */
    Plant* PlantCoordinator::GetPlant(long plant_number)
    {
        return (m_plants.at(plant_number));
    }

    DoubleCoordinate PlantCoordinator::GetOrigin() const
    {
        using Utility::CSMin;
        using Utility::CSMax;
        double minX = 0, minY = 0, minZ = 0, maxX = 0, maxY = 0, maxZ = 0;
        PLANT_CONST_ITERATION_BEGIN
            DoubleCoordinate dc = plant_->GetOrigin();
        minX = CSMin(minX, dc.x);
        minY = CSMin(minY, dc.y);
        minZ = CSMin(minZ, dc.z);
        maxX = CSMax(maxX, dc.x);
        maxY = CSMax(maxY, dc.y);
        maxZ = CSMax(maxZ, dc.z);
        PLANT_CONST_ITERATION_END
            return DoubleCoordinate((minX == 0) ? maxX : (minX + (maxX - minX) / 2.0),
            (minY == 0) ? maxY : (minY + (maxY - minY) / 2.0),
                (minZ == 0) ? maxZ : (minZ + (maxZ - minZ) / 2.0));
    }


    /* ***************************************************************************
    */
    long int PlantCoordinator::Register(ProcessActionDescriptor* action)
    {
        Use_ProcessCoordinator;
        Use_ScoreboardCoordinator;

        ScoreboardStratum stratum(ScoreboardStratum::Soil);
        m_soil_scoreboard_Issue15 = scoreboardcoordinator->GetScoreboard(stratum);

        // MSA 11.01.18 Making all these vary by VolumeObject also.

        // *** PER-PLANT PLANTSUMMARIES ***

        //
        // Make sure that per-Plant NOT per-RootOrder RootLength is available
        PlantSummaryHelper helper(processcoordinator, this);
        PlantSummaryRegistration* psr = helper.MakePlantSummaryRegistration(action);
        psr->RegisterByPerBox(true);
        psr->RegisterAllPlantCombos();
        psr->RegisterAllVolumeObjectCombos();
        psr->RegisterAllBranchOrders();
        psr->RegisterSummaryType(srRootLength);
        psr->RegisterModule((Process *)this);
        helper.SendPlantSummaryRegistration(psr, action, this);

        // MSA 10.09.24 Enabling Root Tip Count per-Plant NOT per-RootOrder
        PlantSummaryRegistration* psrc = helper.MakePlantSummaryRegistration(action);
        psrc->RegisterByPerBox(true);
        psrc->RegisterAllPlantCombos();
        psrc->RegisterAllVolumeObjectCombos();
        psrc->RegisterAllBranchOrders();
        psrc->RegisterSummaryType(srRootTipCount);
        psrc->RegisterModule((Process *)this);
        helper.SendPlantSummaryRegistration(psrc, action, this);

        // MSA 10.09.27 and Branch Count per-Plant NOT per-RootOrder
        PlantSummaryRegistration* psrbc = helper.MakePlantSummaryRegistration(action);
        psrbc->RegisterByPerBox(true);
        psrbc->RegisterAllPlantCombos();
        psrbc->RegisterAllVolumeObjectCombos();
        psrbc->RegisterAllBranchOrders();
        psrbc->RegisterSummaryType(srRootBranchCount);
        psrbc->RegisterModule((Process *)this);
        helper.SendPlantSummaryRegistration(psrbc, action, this);

        // MSA 10.09.27 and Branch Density per-Plant NOT per-RootOrder
        PlantSummaryRegistration* psrbd = helper.MakePlantSummaryRegistration(action);
        psrbd->RegisterByPerBox(true);
        psrbd->RegisterAllPlantCombos();
        psrbd->RegisterAllVolumeObjectCombos();
        psrbd->RegisterAllBranchOrders();
        psrbd->RegisterSummaryType(srRootBranchCountDensity);
        psrbd->RegisterModule((Process *)this);
        helper.SendPlantSummaryRegistration(psrbd, action, this);

        // MSA 10.09.27 and Root Length Density per-Plant NOT per-RootOrder
        PlantSummaryRegistration* psrld = helper.MakePlantSummaryRegistration(action);
        psrld->RegisterByPerBox(true);
        psrld->RegisterAllPlantCombos();
        psrld->RegisterAllVolumeObjectCombos();
        psrld->RegisterAllBranchOrders();
        psrld->RegisterSummaryType(srRootLengthDensity);
        psrld->RegisterModule((Process *)this);
        helper.SendPlantSummaryRegistration(psrld, action, this);


        // *** PER-PLANT PER-ROOTORDER PLANTSUMMARIES ***

        // MSA 10.09.28 and Root Length per-Plant per-RootOrder
        PlantSummaryRegistration* psrplo = helper.MakePlantSummaryRegistration(action);
        psrplo->RegisterByPerBox(true);
        psrplo->RegisterAllPlantCombos();
        psrplo->RegisterAllVolumeObjectCombos();
        psrplo->RegisterAllBranchOrderCombos();
        psrplo->RegisterSummaryType(srRootLength);
        psrplo->RegisterModule((Process *)this);
        helper.SendPlantSummaryRegistration(psrplo, action, this);

        // MSA 10.09.28 and Root Tip Count per-Plant per-RootOrder
        PlantSummaryRegistration* psrcpro = helper.MakePlantSummaryRegistration(action);
        psrcpro->RegisterByPerBox(true);
        psrcpro->RegisterAllPlantCombos();
        psrcpro->RegisterAllVolumeObjectCombos();
        psrcpro->RegisterAllBranchOrderCombos();
        psrcpro->RegisterSummaryType(srRootTipCount);
        psrcpro->RegisterModule((Process *)this);
        helper.SendPlantSummaryRegistration(psrcpro, action, this);

        // MSA 10.09.28 and Branch Count per-Plant per-RootOrder
        PlantSummaryRegistration* psrbcpro = helper.MakePlantSummaryRegistration(action);
        psrbcpro->RegisterByPerBox(true);
        psrbcpro->RegisterAllPlantCombos();
        psrbcpro->RegisterAllVolumeObjectCombos();
        psrbcpro->RegisterAllBranchOrderCombos();
        psrbcpro->RegisterSummaryType(srRootBranchCount);
        psrbcpro->RegisterModule((Process *)this);
        helper.SendPlantSummaryRegistration(psrbcpro, action, this);

        // MSA 10.09.28 and Branch Density per-Plant per-RootOrder
        PlantSummaryRegistration* psrbdpro = helper.MakePlantSummaryRegistration(action);
        psrbdpro->RegisterByPerBox(true);
        psrbdpro->RegisterAllPlantCombos();
        psrbdpro->RegisterAllVolumeObjectCombos();
        psrbdpro->RegisterAllBranchOrderCombos();
        psrbdpro->RegisterSummaryType(srRootBranchCountDensity);
        psrbdpro->RegisterModule((Process *)this);
        helper.SendPlantSummaryRegistration(psrbdpro, action, this);

        // MSA 10.09.28 and Root Length Density per-Plant per-RootOrder
        PlantSummaryRegistration* psrldpro = helper.MakePlantSummaryRegistration(action);
        psrldpro->RegisterByPerBox(true);
        psrldpro->RegisterAllPlantCombos();
        psrldpro->RegisterAllVolumeObjectCombos();
        psrldpro->RegisterAllBranchOrderCombos();
        psrldpro->RegisterSummaryType(srRootLengthDensity);
        psrldpro->RegisterModule((Process *)this);
        helper.SendPlantSummaryRegistration(psrldpro, action, this);

        return kNoError;
    }


    long int PlantCoordinator::Initialise(ProcessActionDescriptor* action)
    {
        Use_ProcessCoordinator;
        Process* aProcessModule = Find_Process_By_Process_Name("Nitrate");
        itsNitrateModuleID = aProcessModule->GetProcessID();

        return (Process::Initialise(action));
    }

    /* ***************************************************************************
    */
    long int PlantCoordinator::StartUp(ProcessActionDescriptor* action)
    {
        // Initialise m_previousWakingTimeStamp.
        Use_Time;
        m_previousWakingTimeStamp = time;

        m_scoreboardCoordinator = action->GetScoreboardCoordinator();

        Use_ProcessCoordinator;

        if (temperature_index < 0)
        {
            temperature_index = Find_Characteristic_Index_By_Characteristic_Name_Other_Volume("Temperature");
        }

        bool atLeastOnePlantStarted = false;

        PLANT_ITERATION_BEGIN
            if (plant_->IsUnstarted())
            {
                if (!plant_->Start(action)) // 
                { // The "Start()" function should return false if the plant_ did not make it out of
                    // the stables. We should put it out of its misery and stop wasting our time
                    RemovePlant(plant_);
                }
                else
                {
                    //  otherwise, the plant_ started ok. We can commence regular behaviour
                    atLeastOnePlantStarted = true;
                }
            }
        PLANT_ITERATION_END

            if (atLeastOnePlantStarted)
            {
                Use_PostOffice;
                SetPeriodicDelay(kNormalMessage, plant_growth_time_step);
                Send_GeneralPurpose(time + plant_growth_time_step, this, __nullptr, kNormalMessage);
            }

        return (Process::StartUp(action));
    }

    long int PlantCoordinator::Ending(ProcessActionDescriptor* action)
    {
        return (Process::Ending(action));
    }

    long int PlantCoordinator::WakeUp(ProcessActionDescriptor* action)
    {
        Use_Code;
        Use_ReturnValue;


        switch (code)
        {
        case kRegisterGlobalPlantSummaryMessage:
            return_value = DoRegisterGlobalPlantSummary(action);
            break;

        case kRegisterScoreboardPlantSummaryMessage:
            return_value = DoRegisterScoreboardPlantSummary(action);
            break;

        default:
            return_value = Process::WakeUp(action);
            break;
        }

        return (return_value);
    }

    long int PlantCoordinator::DoNormalWakeUp(ProcessActionDescriptor* action)
    {
        Use_Time;

        const ProcessTime_t time_step = (time - GetPreviousTimeStamp()) / PROCESS_HOURS(1);

        PLANT_ITERATION_BEGIN
            plant_->DynamicResourceAllocationToRoots(action);
        PLANT_ITERATION_END

            //
            // Normal Growing
            PLANT_ITERATION_BEGIN
            try
        {
            plant_->NextTimeStep(time_step);
        }
        catch (const std::exception& stdex)
        {
            LOG_ALERT << "Caught std::exception calling PlantCoordinator::DoNormalWakeUp() : " << stdex.what();
            RmAssert(false, "Caught std::exception in PlantCoordinator::DoNormalWakeUp()");
        }
        catch (...)
        {
            RmAssert(false, "Caught Exception in PlantCoordinator::DoNormalWakeUp()");
        }
        PLANT_ITERATION_END

            m_previousWakingTimeStamp = time;

        return (Process::DoNormalWakeUp(action));
    }


    //
    // VMD 17/04/00  This routine has been included so that the nitrate uptake
    // routine in NitrateLeaching.cpp can be used to wake up
    // PlantCoordinator:: Root ResponseNitrate
    long int PlantCoordinator::DoExternalWakeUp(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;

        // we want to know who sent the wake up, so we can decide how to act.
        Use_Source;
        Use_Data;

        long source_module_id = -1;
        if (source != 0)
        {
            source_module_id = source->GetProcessID();
        }

        // now check if it is the water module
        if (source_module_id == itsNitrateModuleID)
        {
            if (data == 0)
            {
                //return_value = RootResponseNitrate(action);  //A message has been send from PNitrateLeaching::UptakeN
            }
        }

        return (return_value);
    }

    long int PlantCoordinator::DoDelayedReceivalReaction(ProcessActionDescriptor* action)
    {
        return (Process::DoDelayedReceivalReaction(action));
    }

    long int PlantCoordinator::DoRegisterGlobalPlantSummary(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_PlantSummaryRegistration;

        return (return_value);
    }


    //
    // Function:
    //  DoRegisterScoreboardPlantSummary
    //
    //
    // Description:
    //  Performed when the "WakeUp" function is called with code 'RsPs'
    //  (kRegisterScoreboardPlantSummaryMessage)
    //
    //  Other modules may request the plantcoordinator to instigate plant summaries, for
    //  whatever reason it is that is useful to that other module. When doing so,
    //  the module may request a variety of combinations of summaries. This function
    //  handles the request, and fills in the answer PlantSummaryRegistration
    //
    //
    // Working Notes:
    //  Technical notes about algorithms and other implementation-level details.
    //  
    //
    // Modifications:
    //  YYYYMMDD RvH - 
    //
    long int PlantCoordinator::DoRegisterScoreboardPlantSummary(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_PlantSummaryRegistration;
        bool psr_already_registered = false;

        PLANT_SUMMARY_REGISTRATION_ITERATION_BEGIN
            if (*plant_summary_registration_ == *psr)
            {
                plant_summary_registration_->AlreadyMadeScoreboardSummaries(action, this);
                psr_already_registered = true;
                break;
            }
        PLANT_SUMMARY_REGISTRATION_ITERATION_END

            if (!psr_already_registered)
            {
                psr->MakeScoreboardSummaries(action, this);
                m_plantSummaryRegistrations.insert(psr);
            }
            else
            {
                delete psr;
            }

        return (return_value);
    }

    /* **************************************************************************
    //  Don't really need this, as it doesn't really tell us anything, however we
    //  register so we should do the
    */
    long int PlantCoordinator::DoScoreboardPlantSummaryRegistered(ProcessActionDescriptor* action)
    {
        return (Process::DoScoreboardPlantSummaryRegistered(action));
    }

    long int PlantCoordinator::DoRaytracerOutput(RaytracerData* raytracerData)
    {
        PLANT_ITERATION_BEGIN
            plant_->DoRaytracerOutput(raytracerData);
        PLANT_ITERATION_END

            return kNoError;
    }


    bool PlantCoordinator::DoesSaveProcessData() const
    {
        return (true);
    }

    bool PlantCoordinator::DoesRaytracerOutput() const
    {
        return (true);
    }

    long int PlantCoordinator::DoActivateRaytracerOutput()
    {
        PLANT_ITERATION_BEGIN
            plant_->DoActivateRaytracerOutput();
        PLANT_ITERATION_END
            return kNoError;
    }

    Process* PlantCoordinator::FindProcessByProcessID(ProcessIdentifier process_id)
    {
        return (FindPlantByProcessID(process_id));
    }

    Process* PlantCoordinator::FindProcessByProcessName(const std::string& pname)
    {
        return (FindPlantByPlantName(pname));
    }


    void PlantCoordinator::AddPlant(Plant* new_plant)
    {
        if (new_plant != 0)
        {
            m_plants.push_back(new_plant);

#ifdef CS_DEBUG_STREAM
            Str255 pname;
            new_plant->GetProcessName(pname);
            char pname_c[256];
            TCLptocstrcpy(pname_c, pname);
            CS_DEBUG_STREAM << "PlantCoordinator::AddPlant - " << pname_c << endl;
#endif // #ifdef CS_DEBUG_STREAM
            //        std::string s;
            //        StringUtility::GetProcessName(new_plant,s);
            //        mySharedAttributeOwner->VariationChange("Plant",s.c_str(),+1);
            mySharedAttributeOwner->VariationChange("Plant", +1);
        }
    }

    void PlantCoordinator::RemovePlant(Plant* rplant)
    {
        PLANT_ITERATION_BEGIN
            if (plant_ == rplant)
            {
                m_plants.erase(p_iter); // p_iter the iterator for this loop
                break;
            }
        PLANT_ITERATION_END
    }

    /* FindPlantByProcessID
    Given a supposed Plant's process ID, find the plant in our list.
    This is called by ProcessCoordinator::FindProcessByProcessID()
    30-Aug-1999 RvH check for plants, first
    */
    Plant* PlantCoordinator::FindPlantByProcessID(long plant_processID)
    {
        PLANT_ITERATION_BEGIN
            if (plant_->GetProcessID() == plant_processID)
            {
                return (plant_);
            }
        PLANT_ITERATION_END

            return (0);
    }

    /* FindPlantByPlantName
    Given the Plant's name, find it in our list. The search uses the iterative
    method, matching names with the Mac Toolbox routine "EqualString" :-
    case-sensitivity is ON, diacritical-sensitivity OFF
    30-Aug-1999 RvH check for plants, first
    */
    Plant* PlantCoordinator::FindPlantByPlantName(const std::string& search_name)
    {
        PLANT_ITERATION_BEGIN
            if (plant_->GetProcessName() == search_name)
            {
                return (plant_);
            }
        PLANT_ITERATION_END

            return (0);
    }

    /* CheckIfPlantExists
    Given a pointer to a supposed Plant, make sure that it is listed.
    This is a useful sanity check, for when other modules give us
    a supposed Plant pointer - which we cannot assume is valid.
    30-Aug-1999 RvH check for plants, first
     */
    bool PlantCoordinator::DoesPlantExist(Plant* p)
    {
        if ((0 == p) || (m_plants.empty()))
        {
            return false;
        }

        PLANT_ITERATION_BEGIN
            if (plant_ == p)
            {
                return (true);
            }
        PLANT_ITERATION_END

            return (false);
    }

    /* GetNewPlantID
    Finds a new, unused number to associate with a Plant.*/
    PlantIdentifier PlantCoordinator::GetNewPlantId()
    {
        return (IdentifierUtility::instance().useNextPlantIdentifier());
    }


    void PlantCoordinator::AddSummary(PlantSummary* new_summary)
    {
        if (new_summary != 0)
        {
            long summary_type_index = new_summary->GetSummaryTypeArrayIndex();
            // long other_summary_type_index = summary_type_index-1; // MSA 09.10.07 Unused


            // Now add this new summary to the end of the multimap
            m_plantSummaries.insert(std::pair<long, PlantSummary*>(summary_type_index, new_summary));

            m_plantSummariesALL.push_back(new_summary);

            ProcessModuleBase::AddCharacteristic(new_summary->GetCharacteristicDescriptor());
            m_scoreboardCoordinator->RegisterCharacteristic(new_summary->GetCharacteristicDescriptor());
        } // if (new_summary != 0)
    }


    PlantSummary* PlantCoordinator::FindSummaryByCharacteristicIndex(long characteristic_index)
    {
        PLANT_SUMMARY_ITERATION_BEGIN
            if (plant_summary_->GetCharacteristicIndex() == characteristic_index)
            {
                return (plant_summary_);
            }
        PLANT_SUMMARY_ITERATION_END

            return (0);
    }

    PlantSummary* PlantCoordinator::FindSummaryByID(PlantSummaryIdentifier summary_id)
    {
        PLANT_SUMMARY_ITERATION_BEGIN
            if (plant_summary_->GetCharacteristicID() == summary_id)
            {
                return (plant_summary_);
            }
        PLANT_SUMMARY_ITERATION_END

            return (0);
    }


    /* GetNewSummaryID
    Finds a new, unused number to associate with a Plant.*/
    PlantSummaryIdentifier PlantCoordinator::GetNewSummaryID(SummaryRoutineGroup /* summary_type_index */, const ScoreboardStratum& stratum)
    {
        return (IdentifierUtility::instance().useNextPlantSummaryIdentifier(stratum));
    }


    void PlantCoordinator::AddPlantType(PlantType* new_type)
    {
        if (new_type != 0)
        {
            m_plantTypes.push_back(new_type);

#ifdef CS_DEBUG_STREAM
            Str255 pname;
            new_type->GetName(pname);
            char pname_c[256];
            TCLptocstrcpy(pname_c, pname);
            CS_DEBUG_STREAM << "PlantCoordinator::AddPlantType - " << pname_c << endl;
#endif // #ifdef CS_DEBUG_STREAM

            //        mySharedAttributeOwner->VariationChange("PlantType",pname_c,+1);
            mySharedAttributeOwner->VariationChange("PlantType", +1);
        }
    }

    /* GetPlantType
    By its index in the itsPlants list. This is the easy method. */
    PlantType* PlantCoordinator::GetPlantType(long planttype_index)
    {
        return (m_plantTypes.at(planttype_index));
    }


    // 30-August-1999 RvH check for plant types, first
    PlantType* PlantCoordinator::FindPlantTypeByName(const std::string& name)
    {
        if (m_plantTypes.empty() || name.empty()) return (0);

        PLANT_TYPE_ITERATION_BEGIN
            if (plant_type_->GetName() == name)
            {
                return (plant_type_);
            }
        PLANT_TYPE_ITERATION_END

            return (0);
    }

    // 30-August-1999 RvH check for plant types, first
    PlantType* PlantCoordinator::FindPlantTypeByID(long plant_type_id)
    {
        if (m_plantTypes.empty()) return (0);

        PLANT_TYPE_ITERATION_BEGIN
            if (plant_type_->GetID() == plant_type_id)
            {
                return (plant_type_);
            }
        PLANT_TYPE_ITERATION_END

            return (0);
    }

    long PlantCoordinator::GetNewPlantTypeID() const
    {
        return (IdentifierUtility::instance().useNextPlantTypeIdentifier());
    }


    const double PlantCoordinator::TempAdjustRate(const double& rate_to_adjust, const BoxIndex& box, const double& temp_of_zero_growth) const
    {
        double temp = m_soil_scoreboard_Issue15->GetCharacteristicValue(temperature_index, box);

        if (temp > temp_of_zero_growth)
        {
            return (((temp - temp_of_zero_growth) * rate_to_adjust) / (reference_temperature - temp_of_zero_growth));
        }

        return (0);
    }

    /*    FindBoxByDoubleCoordinate
    *    Three (3) versions, with varying amounts of information returned
    */
    BoxIndex PlantCoordinator::FindBoxByDoubleCoordinate(const DoubleCoordinate* position)
    {
        BoxCoordinate bc;
        Scoreboard* scoreboard = m_scoreboardCoordinator->FindBoxBySoilCoordinate(&bc, position);
        return (scoreboard->GetBoxIndex(bc.x, bc.y, bc.z));
    }

    BoxIndex PlantCoordinator::FindBoxByDoubleCoordinate(const DoubleCoordinate* position, WrapDirection* wrapping)
    {
        BoxCoordinate bc;
        Scoreboard* scoreboard = m_scoreboardCoordinator->FindBoxBySoilCoordinate(&bc, position, wrapping);
        return (scoreboard->GetBoxIndex(bc.x, bc.y, bc.z));
    }

    BoxIndex PlantCoordinator::FindBoxByDoubleCoordinate(const DoubleCoordinate* position, WrapDirection* wrapping, BoxCoordinate* box_coord)
    {
        Scoreboard* scoreboard = m_scoreboardCoordinator->FindBoxBySoilCoordinate(box_coord, position, wrapping);
        return (scoreboard->GetBoxIndex(box_coord->x, box_coord->y, box_coord->z));
    }


    /* AdjustRootLength
    Subtracts or adds the length to the summary in the scoreboard box. A future
    version will accept the end points of the length, and put appropriate proportions
    of it, if necessary, into all of the boxes through which it passes, as well as putting
    it to use in any other relevant summaries at hand.

    98.09.16    Note that "Scoreboard::AddCharacteristicLength()" does that.*/
    void PlantCoordinator::AdjustRootLength
    (Plant* p,
        const double& length,
        const DoubleCoordinate* first_point,
        const BoxCoordinate* first_box_coord,
        const BoxIndex& first_box,
        const size_t& volumeObjectIndex,
        const DoubleCoordinate* last_point,
        const long& branch_order,
        const WrapDirection& wrap)
    {
        RmAssert(first_box >= 0, "First box index given was <0");

        // calculate the maximum number of summaries to check
        long summary_count = m_plantSummaries.count(summary_type_root_length_dependent);
        if (summary_count < 1) return;

        static bool KLUDGE = true;

        if (KLUDGE)
        {
            RmAssert(m_soil_scoreboard_Issue15->GetCoincidentProportion(first_box, volumeObjectIndex) > 0, "Error: This volume is logically zero, cannot add root length.");
            KLUDGE = false;
        }

        BoxIndex last_box = InvalidBoxIndex;

        // MSA 11.06.15 A thought: Geometrically speaking, if two points are inside a convex polyhedron, is the line segment joining them
        // guaranteed to be entirely inside the polyhedron?

        std::pair<PlantSummaryMultimap::iterator, PlantSummaryMultimap::iterator> subrange = m_plantSummaries.equal_range(summary_type_root_length_dependent);
        for (PlantSummaryMultimap::iterator iter = subrange.first; iter != subrange.second; ++iter)
        {
            PlantSummary* temp_plant_summary = iter->second;

            if (temp_plant_summary->DoSummarise(p, wrap, volumeObjectIndex, branch_order))
            {
                switch (temp_plant_summary->GetSummaryType())
                {
                case srRootLength:
                    last_box = m_soil_scoreboard_Issue15->AddCharacteristicLength(temp_plant_summary->GetCharacteristicIndex(), length, first_point, first_box_coord, first_box, volumeObjectIndex, last_point, false);
                    break;
                case srRootLengthDensity:
                    last_box = m_soil_scoreboard_Issue15->AddCharacteristicLength(temp_plant_summary->GetCharacteristicIndex(), length, first_point, first_box_coord, first_box, volumeObjectIndex, last_point, true);
                    break;
                case srRootVolume:
                    break;
                case srRootSurfaceArea:
                    break;
                default: break;
                } // switch (temp_plant_summary->GetSummaryType())
            }
            else
            {
                //LOG_DEBUG << "AdjustRootLength did not use PlantSummary " << *temp_plant_summary;
                //LOG_DEBUG << "AdjustRootLength given Root: {BranchOrder:" << branch_order << "} {wrap:" << wrap << "}";
            }
        }

        // if the root tip appears to have moved to another box, track it !
        if (last_box != first_box)
        {
            if (InvalidBoxIndex != first_box)
            {
                // MSA 11.01.18 Here I assume that the tip was previously in the box containing first_point. MSA TODO Is this valid?
                // MSA 11.01.31 No, it's not valid. Results in negative tip counts.
                DoubleCoordinate* fpclone = new DoubleCoordinate(*first_point);
                AdjustTipCount(p, -1, first_box, fpclone, volumeObjectIndex, branch_order, wrap);
                delete fpclone;
            }

            // if there were no rootlength or rootlengthdensity summaries that this
            // was summmarised to, the last_box won't have been determined yet.
            if (InvalidBoxIndex == last_box)
            {
                BoxCoordinate last_coord;
                WrapDirection wrap_dir;
                m_soil_scoreboard_Issue15->FindBoxBySoilCoordinate(&last_coord, last_point, &wrap_dir);
                last_box = m_soil_scoreboard_Issue15->GetBoxIndex(last_coord);
            }

            if (InvalidBoxIndex != last_box)
            {
                // MSA 11.01.18 Here I assume that the tip is now in the box containing last_point. MSA TODO Is this valid?
                DoubleCoordinate* lpclone = new DoubleCoordinate(*last_point);
                AdjustTipCount(p, +1, last_box, lpclone, volumeObjectIndex, branch_order, wrap);
                delete lpclone;
            }
        }
    } // AdjustRootLength

    /* AdjustBranchCount
    Subtracts or adds the countlength to the summary in the scoreboard box. */
    void PlantCoordinator::AdjustBranchCount
    (Plant* p,
        long count,
        BoxIndex box,
        DoubleCoordinate* /*position*/,
        const size_t& volumeObjectIndex,
        long branch_order,
        WrapDirection wrap)
    {
        RmAssert(box >= 0, "Box index given was <0"); // Debug build
        if (box < 0) return; // Release build

        // calculate the maximum number of summaries to check
        long summary_count = m_plantSummaries.count(summary_type_branch_count_dependent);
        if (summary_count < 1) return;

        long wrap_x = wrap & wraporderX;
        long wrap_y = wrap & wraporderY;
        std::pair<PlantSummaryMultimap::iterator, PlantSummaryMultimap::iterator> subrange = m_plantSummaries.equal_range(summary_type_branch_count_dependent);
        for (PlantSummaryMultimap::iterator iter = subrange.first; iter != subrange.second; ++iter)
        {
            PlantSummary* temp_plant_summary = iter->second;

            // 1) Ensure this plant is included in the summary.
            if (!temp_plant_summary->IncludesPlant(p)) continue; // If not, go to next summary

            // 2) Check if the wrap orders match.
            if (!temp_plant_summary->IsWrapOrderX() && wrap_x != 0) continue; // If not, go to next summary
            if (!temp_plant_summary->IsWrapOrderY() && wrap_y != 0) continue; // If not, go to next summary

            // 3) Check if the spatial subsection matches (i.e. the new tip is located within this subsection)
            if (volumeObjectIndex != temp_plant_summary->GetVolumeObjectIndex()) continue; // If not, go to next summary

            // 4) Check if the root orders match.
            const long summaryBO = temp_plant_summary->GetBranchOrder();
            if (summaryBO != branch_order && summaryBO >= 0) continue; // If not, go to next summary

            // 5) Finally, if all these checks have been passed, we can summarise.
            switch (temp_plant_summary->GetSummaryType())
            {
            case srRootBranchCount:
                m_soil_scoreboard_Issue15->AddCharacteristicValue(temp_plant_summary->GetCharacteristicIndex(), count, box);
                break;
            case srRootBranchCountDensity:
                m_soil_scoreboard_Issue15->AddCharacteristicValue(temp_plant_summary->GetCharacteristicIndex(), count / m_soil_scoreboard_Issue15->GetBoxVolume(box), box);
                break;
            default: break;
            }
        }
    }

    /* AdjustTipCount
    Subtracts or adds the countlength to the summary in the scoreboard box. */
    void PlantCoordinator::AdjustTipCount
    (Plant* p,
        const long count,
        const BoxIndex box,
        const DoubleCoordinate* /*position*/,
        const size_t& volumeObjectIndex,
        const long branch_order,
        const WrapDirection wrap)
    {
        RmAssert(box >= 0, "Box index given was <0");
        if (box < 0) return;

        // calculate the maximum number of summaries to check
        long summary_count = m_plantSummaries.count(summary_type_tip_count_dependent);
        if (summary_count < 1) return;

        long wrap_x = wrap & wraporderX;
        long wrap_y = wrap & wraporderY;
        std::pair<PlantSummaryMultimap::iterator, PlantSummaryMultimap::iterator> subrange = m_plantSummaries.equal_range(summary_type_tip_count_dependent);
        for (PlantSummaryMultimap::iterator iter = subrange.first; iter != subrange.second; ++iter)
        {
            PlantSummary* temp_plant_summary = iter->second;

            // 1) Ensure this plant is included in the summary.
            if (!temp_plant_summary->IncludesPlant(p)) continue; // If not, go to next summary

            // 2) Check if the wrap orders match.
            if (!temp_plant_summary->IsWrapOrderX() && wrap_x != 0) continue; // If not, go to next summary
            if (!temp_plant_summary->IsWrapOrderY() && wrap_y != 0) continue; // If not, go to next summary

            // 3) Check if the spatial subsection matches (i.e. the new tip is located within this subsection)
            if (volumeObjectIndex != temp_plant_summary->GetVolumeObjectIndex()) continue; // If not, go to next summary

            // 4) Check if the root orders match.
            const long summaryBO = temp_plant_summary->GetBranchOrder();
            if (summaryBO != branch_order && summaryBO >= 0) continue; // If not, go to next summary

            // 5) Finally, if all these checks have been passed, we can summarise.
            switch (temp_plant_summary->GetSummaryType())
            {
            case srRootTipCount:
                /*
                {
                if(summaryBO>=0)
                {
                    wxString str = "RootTip ID ";
                    str<<elid<< " RootOrder "<<branch_order;
                    str<< (count<0 ? " moved out of box " : " moved into box ");
                    str<<box<<" [@ point (" << position->x << "," << position->y << "," << position->z << ")]";

                    LOG_ALERT<<str.c_str();
                }
                */
                m_soil_scoreboard_Issue15->AddCharacteristicValue(temp_plant_summary->GetCharacteristicIndex(), count, box);
                break;
            case srRootTipCountDensity:
                m_soil_scoreboard_Issue15->AddCharacteristicValue(temp_plant_summary->GetCharacteristicIndex(), count / m_soil_scoreboard_Issue15->GetBoxVolume(box), box);
                break;
            default: break;
            }
        }
    }
} /* namespace rootmap */


