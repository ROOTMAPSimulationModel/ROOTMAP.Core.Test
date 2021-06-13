/////////////////////////////////////////////////////////////////////////////
// Name:        Water.cpp
// Purpose:     Implementation of the Water class
// Created:     April 1998
// Author:      Vanessa Dunbabin
// $Date: 2009-05-12 (Tues, 12th May 2009) $
// $Revision: 41 $
// Copyright:   ©2006 University of Tasmania, Dr Vanessa Dunbabin, Centre for Legumes in Meditteranean Agriculture, Grains Research and Development Corporation, Department of Agriculture and Food Western Australia
// 
// Water.cpp This is the water movement routine.  Each box is initialised with a certain field capacity
// and any incoming water - such as a rain event - fills each box to field capacity.  Boxes continue to fill down 
// through the profile until all the water has been taken in.
// 
/////////////////////////////////////////////////////////////////////////////

//Heritage
#include "simulation/process/common/Process.h"
#include "simulation/process/common/ProcessDrawing.h"
#include "simulation/process/modules/Water.h"
#include "simulation/process/modules/WaterNotification.h"
#include "simulation/process/modules/VolumeObject.h"

#include "core/utility/Utility.h"
#include "core/common/RmAssert.h"

#include "simulation/process/plant/PlantSummaryRegistration.h"
#include "simulation/process/plant/PlantSummaryHelper.h"
#include "simulation/process/shared_attributes/ProcessSharedAttributeOwner.h"
#include "simulation/process/shared_attributes/ProcessAttributeSupplier.h"
#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"
#include "simulation/process/shared_attributes/SharedAttributeManager.h"
#include "simulation/process/shared_attributes/SharedAttributeSearchHelper.h"
#include "simulation/process/shared_attributes/SharedAttribute.h"
#include "simulation/process/common/ProcessUtility.h"

#include "simulation/data_access/interface/ProcessSpecialDAI.h"
#include "simulation/process/raytrace/RaytracerData.h"
#include "simulation/process/modules/VolumeObjectCoordinator.h"

#include "simulation/data_access/interface/ProcessDAI.h"
#include "simulation/data_access/tinyxmlconfig/TXCharacteristicDA.h"
#include "simulation/data_access/tinyxmlconfig/TXProcessDA.h"

#include "wx/log.h"
#include "wx/gdicmn.h"

#include <cstdlib>
#include <cmath>


#define WAS_DEBUGGER(s) if ( (s) ) { RootMapLogError(LOG_LINE << "Encountered error in logic : " << s); }

namespace rootmap
{
    using Utility::CSMax;
    using Utility::CSMin;
    RootMapLoggerDefinition(Water);
    IMPLEMENT_DYNAMIC_CLASS(Water, Process)

        /* **********************************************************************************
        */
        Water::Water()
        : Process()
        , water_TimeOfPreviousLeaching(-1)
        , water_timeBetweenWakings(PROCESS_HOURS(4))
        , water_TimeOfPreviousWaterUptake(-1)
        , water_TimeOfPreviousEvaporation(-1)
        , water_TimeOfPreviousRedistribution(-1)
        , water_TimeOfPreviousCall(-1)
        , water_TimeOfPreviousRain(-1.0)
        , water_CumulativeEvaporation(0.0)
        , water_AverageEvaporation(0.0)
        , mySharedAttributeOwner(__nullptr)
        , m_scoreboardCoordinator(__nullptr)
        , m_volumeObjectCoordinator(__nullptr)
        , m_IS_NUTRIENT_SOLUTION(false)
        , m_CALCULATE_EVAPORATION(false)
        , m_MAINTAIN_WATER_CONTENT(false)
        , m_DRAW_WETTING_FRONT(false)
    {
        RootMapLoggerInitialisation("rootmap.Water");
        mySharedAttributeOwner = new ProcessSharedAttributeOwner("Water", this);
        SharedAttributeRegistrar::RegisterOwner(mySharedAttributeOwner);

        ProcessAttributeSupplier* supplier = new ProcessAttributeSupplier(this, "Water");
        SharedAttributeRegistrar::RegisterSupplier(supplier);
        m_wettingFrontInfo.drawable = false;
        m_wettingFrontInfo.colour = wxColour(0, 0, 128, 128); // Set the desired wetting front colour here. Currently 50% transparent navy blue
        m_wettingFrontInfo.eraseMode = false;
    }

    Water::~Water()
    {
        // RainfallEvents are deleted as they are applied. If a full simulation has not been run,
        // we will probably need to delete some RainfallEvents here.
        for (std::list<RainfallEvent *>::iterator iter = m_rainfallEvents.begin();
            iter != m_rainfallEvents.end(); ++iter)
        {
            delete (*iter);
        }
    }

    void Water::SetVolumeObjectCoordinator(const VolumeObjectCoordinator& voc)
    {
        m_volumeObjectCoordinator = &voc;
    }

    bool Water::DoesOverride() const
    {
        return (true);
    }

    bool Water::DoesDrawing() const
    {
        return (true);
    }

    bool Water::DoesRaytracerOutput() const
    {
        return (true);
    }

    long int Water::DoRaytracerOutput(RaytracerData* raytracerData)
    {
        if (__nullptr == raytracerData || __nullptr == m_scoreboardCoordinator) return (kNoError);

        // MSA 09.11.19 For now, only the wetting front is raytraced.
        if (m_wettingFrontInfo.drawable)
        {
            std::vector<BoxCoordinate>::iterator iter = m_wettingFrontInfo.boxen.begin();
            std::vector<double>::iterator iter2 = m_wettingFrontInfo.frontLocations.begin();
            while (iter != m_wettingFrontInfo.boxen.end() && iter2 != m_wettingFrontInfo.frontLocations.end())
            {
                DoubleBox dbox;
                m_scoreboardCoordinator->GetBoxSoil(&dbox, m_wettingFrontInfo.stratum, &(*iter++));
                raytracerData->AddWettingFront(DoubleCoordinate(dbox.left, dbox.front, *iter2), DoubleCoordinate(dbox.right, dbox.back, *iter2), m_wettingFrontInfo.colour);
                ++iter2;
            }
        }
        return kNoError;
    }


    void Water::InitialiseSpecial(const ProcessSpecialDAI& data)
    {
        try
        {
            const ProcessSpecialDAI::FloatArray& rainfall_events = data.getNamedFloatArray("Rainfall Events");
            const size_t num_values = rainfall_events.size();

            // MSA 09.09.09 Changed this modulus from 2 to 6, as RainfallEvent now consists of 6 values
            if ((num_values % 6) != 0)
            {
                LOG_ERROR << "Rainfall event array is malformed: number of elements is not a multiple of 6 (size=" << rainfall_events.size() << ")";
            }

            for (size_t float_index = 0; float_index < num_values; float_index += 6)
            {
                const double rainfall_rad = rainfall_events[float_index]; //MJ/m2
                const double rainfall_maxT = rainfall_events[float_index + 1]; //degC
                const double rainfall_minT = rainfall_events[float_index + 2]; //degC
                const double rainfall_amount = rainfall_events[float_index + 3]; //cm
                const double rainfall_pan = rainfall_events[float_index + 4]; //mm        
                const double rainfall_days = rainfall_events[float_index + 5];

                const ProcessTime_t rainfall_time = floor(PROCESS_DAYS(rainfall_days));

                RainfallEvent* rainfall_event = new RainfallEvent;
                rainfall_event->rainfall_rad = rainfall_rad;
                rainfall_event->rainfall_maxT = rainfall_maxT;
                rainfall_event->rainfall_minT = rainfall_minT;
                rainfall_event->rainfall_amount = rainfall_amount;
                rainfall_event->rainfall_pan = rainfall_pan;
                rainfall_event->rainfall_time = rainfall_time;

                m_rainfallEvents.push_back(rainfall_event); // This pointer used and deleted in Water::DoSpecialInput()
            }
        }
        catch (const RmException& /* ex */)
        {
            LOG_ERROR << "Caught RmException trying to InitialiseSpecial - probably no \"Rainfall Events\" available";
        }
    }

    void Water::Initialise(const ProcessDAI& data)
    {
        if (m_volumeObjectCoordinator != __nullptr)
        {
            // MSA 10.12.22 List the Characteristics NOT affected by barrier modelling here; exclude from the makeVariants process.
            std::vector<std::string> invariantCharacteristicNames;
            invariantCharacteristicNames.push_back("Draw Wetting Front");
            invariantCharacteristicNames.push_back("Water Routine Number");
            invariantCharacteristicNames.push_back("Maintain Initial Water Content");
            ProcessDAI* ptr = m_volumeObjectCoordinator->AccomodateVolumeObjects(data, invariantCharacteristicNames);
            Process::Initialise(*ptr);
            delete ptr;
        }
        else
        {
            Process::Initialise(data);
        }
    }

    /* **********************************************************************************
    */
    //At this stage nothing is calling the water movement routine so must set periodic wakeup
    long int Water::Initialise(ProcessActionDescriptor* action)
    {
        Use_ProcessCoordinator;
        Use_Scoreboard;
        Use_VolumeObjectCoordinator;

        // MSA 09.11.19 Grab a pointer to the ScoreboardCoordinator for use with DoRaytracerOutput()
        // (for converting the wetting front from BoxCoordinates and ViewDirection to 3D-space coordinates)
        m_scoreboardCoordinator = action->GetScoreboardCoordinator();

        // set up a local characteristic index cache
        // Search by name, so that finding them is independent of the characteristics being ordered correctly in the resource

        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Water_Content_Indices, "Water Content");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Drained_Upper_Limit_Indices, "Drained Upper Limit");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Wilting_Point_Indices, "Wilting Point");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Wetting_Front_Indices, "Wetting Front");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Direction_Moved_Indices, "Direction Moved");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Water_Moved_Indices, "Water Moved");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Rain_Out_Bottom_Indices, "Rain Out Bottom");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Rained_Amount_Indices, "Rained Amount");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Cumul_Local_Water_Uptake_Indices, "Cumul Local Water Uptake");

        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Coefficient_A_Indices, "Coefficient A");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Coefficient_B_Indices, "Coefficient B");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Alpha_Indices, "Alpha");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Saturation_Indices, "Water Saturation");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Coefficient_M_Indices, "Coefficient m");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Coefficient_N_Indices, "Coefficient n");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, K_Saturation_Indices, "Saturated Hydraulic Conductivity");

        water_DrawWettingFrontIndex = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Draw Wetting Front");
        water_WaterRoutineIndex = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Water Routine Number");
        water_MaintainWaterIndex = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Maintain Initial Water Content");

        // Find the Nitrate module
        // 98.10.14 RvH. Simplified.
        water_NitrateModule = Find_Process_By_Process_Name("Nitrate");

        // Find the Phosphorus module
        water_PhosphorusModule = Find_Process_By_Process_Name("Phosphorus");

        // Find the PlantCoordinator
        // 98.10.14 RvH. Simplified.
        water_PlantCoordinatorModule = Find_Process_By_Process_Name("PlantCoordinator");

        water_OrganicMatterModule = Find_Process_By_Process_Name("OrganicMatter");

        // MSA 09.11.09 Determine the Scoreboard dimensions here, save doing it repeatedly
        Get_Number_Of_Layers(NUM_X, NUM_Y, NUM_Z); // number of layers in each dimension
        BOX_COUNT = scoreboard->GetNumberOfBoxes(); // How many boxes do we have?

        // MSA 11.02.10 For concentration-based Characteristics, we may need to modify the defaults so they make logical sense W.R.T. barrier modelling.
        // That is, logically zero volumes must have zero concentrations.
        // MSA 11.06.22 What? No. No, logically zero volumes should not have their concentrations modified at all,
        // because it breaks code further down the line which expects sane values.
        // A logically zero volume should just have the same old default Water Content, 
        // because Water Amount = Water Content * Box Volume
        // so Water Amount = defaultWaterContent * 0 = 0.
        /*
        for(BoxIndex box_index=0; box_index < BOX_COUNT; ++box_index)
        {
            size_t i=0;
            for(CharacteristicIndices::const_iterator iter=Water_Content_Indices.begin(); iter!=Water_Content_Indices.end(); ++iter)
            {
                if(scoreboard->GetCoincidentProportion(box_index, i)<=0.0)
                {
                    scoreboard->SetCharacteristicValue(Water_Content_Indices[i], 0.0, box_index);
                }
                ++i;
            }
        }
        */

        SetPeriodicDelay(kNormalMessage, water_timeBetweenWakings);
        Use_PostOffice;
        Send_GeneralPurpose(action->GetTime() + water_timeBetweenWakings, this, __nullptr, kNormalMessage);

        PrepareSharedAttributes(action);

        ProcessTime_t start = postoffice->GetStart();
        for (std::list<RainfallEvent *>::iterator iter = m_rainfallEvents.begin();
            iter != m_rainfallEvents.end(); ++iter)
        {
            //TODO: allow Rainfall Events to be specified in XML as absolute.
            //Current strategy causes us to assume time is relative to start time.
            //This is only ok if we start just once.
            postoffice->sendMessage((start + (*iter)->rainfall_time), this, this, (*iter), kSpecialInputDataMessage);
        }
        return (Process::Initialise(action));
    }

    /* ***************************************************************************
    */
    long int Water::Register(ProcessActionDescriptor* action)
    {
        Use_ProcessCoordinator;
        PlantSummaryHelper helper(processcoordinator, 0);

        // Per-Plant Per-RootOrder Per-Box (ie. scoreboard)
        PlantSummaryRegistration* psr = helper.MakePlantSummaryRegistration(action);

        psr->RegisterByPerBox(true);
        psr->RegisterAllPlantCombos();
        psr->RegisterAllBranchOrders();
        psr->RegisterSummaryType(srRootLength);
        //
        // making the wrapping false in both directions means that if a Plant's roots
        // grow outside the scoreboard in one direction, they won't be summarised when
        // they come back in the other side.  This is currently what happens with the
        // PRootLength process module, so i'm emulating that here.
        //    psr->RegisterWrapping(false,false);
        psr->RegisterModule((Process*)this);
        helper.SendPlantSummaryRegistration(psr, action, this);
        return kNoError;
    }


    /* ********************************************************************************** */
    void Water::PrepareSharedAttributes(ProcessActionDescriptor* action)
    {
        Use_SharedAttributeManager;
        Use_Scoreboard;
        SharedAttributeSearchHelper searchHelper(action->GetSharedAttributeManager(), mySharedAttributeOwner);

        // Load expected SharedAttributes
        //int branch_lag_time_per_rootorder_index = searchHelper.SearchForClusterIndex("Branch Lag Time","RootOrder");

        std::vector<std::string> variation_names;
        variation_names.push_back("Plant");
        variation_names.push_back("RootOrder");

        saWaterUptakePerPlantIndex = searchHelper.SearchForClusterIndex("Local Plant Water Uptake", "Plant", __nullptr);
        saCumulWaterUptakePerPlantIndex = searchHelper.SearchForClusterIndex("Cumul Local Plant Water Uptake", "Plant", __nullptr);
        saTotalPlantWaterUptakeIndex = searchHelper.SearchForClusterIndex("Cumul Total Plant Water Uptake", "Plant", __nullptr);
        saWaterFluxPerPlantIndex = searchHelper.SearchForClusterIndex("Water Flux", "Plant", variation_names);
        saWaterUpregulationLimitIndex = searchHelper.SearchForClusterIndex("Water Upregulation Limit", "Plant", __nullptr);

        saRainAmount = searchHelper.SearchForAttribute("Rain Amount");
        saPanEvaporation = searchHelper.SearchForAttribute("Pan Evaporation");
        saMaxTemperature = searchHelper.SearchForAttribute("Max Temperature");
        saMinTemperature = searchHelper.SearchForAttribute("Min Temperature");
        saRadiation = searchHelper.SearchForAttribute("Radiation");

        saGreenCover = searchHelper.SearchForAttribute("Green Cover");
        saAvgGreenCoverFactor = searchHelper.SearchForAttribute("Avg Green Cover Factor");
        saResidueCover = searchHelper.SearchForAttribute("Residue Cover");
        saCumulEvapBeforeGermination = searchHelper.SearchForAttribute("Cumulative Evaporation Before Germination");
        saCumulEvapAfterGermination = searchHelper.SearchForAttribute("Cumulative Evaporation After Germination");
        saDegreeDays = searchHelper.SearchForAttribute("Degree Days");
        saGrowStartDegreeDays = searchHelper.SearchForAttribute("Degree Days at Growth Start");
        saCalculateEvap = searchHelper.SearchForAttribute("Calculate Evaporation");

        saGerminationLagIndex = searchHelper.SearchForClusterIndex("Germination Lag", "Plant", __nullptr);
        saSeedingTimeIndex = searchHelper.SearchForClusterIndex("Seeding Time", "Plant", __nullptr); //Time from zero (start of simulation) in hours when the seeds are sown
        saTimeFullCoverIndex = searchHelper.SearchForClusterIndex("Full Cover Days", "Plant", __nullptr); //days after germination that full cover occurs
        saTimeRipeningIndex = searchHelper.SearchForClusterIndex("Ripening Days", "Plant", __nullptr); //days after germination that ripening occurs
        saTimeHarvestIndex = searchHelper.SearchForClusterIndex("Harvest Days", "Plant", __nullptr); //days after germination that harvest occurs
        saDegDaysFullCoverIndex = searchHelper.SearchForClusterIndex("Full Cover Degree Days", "Plant", __nullptr); //degree days after germination that full cover occurs
        saDegDaysRipeningIndex = searchHelper.SearchForClusterIndex("Ripening Degree Days", "Plant", __nullptr); //degree days after germination that ripening occurs
        saDegDaysHarvestIndex = searchHelper.SearchForClusterIndex("Harvest Degree Days", "Plant", __nullptr); //degree days after germination that harvest occurs
        saGroundCoverMaxIndex = searchHelper.SearchForClusterIndex("Ground Cover Max", "Plant", __nullptr);
        saGroundCoverHarvestIndex = searchHelper.SearchForClusterIndex("Ground Cover Harvest", "Plant", __nullptr);
        saDegreeDaysModelIndex = searchHelper.SearchForClusterIndex("Degree Days Model", "Plant", __nullptr);
        saNutrientSolutionIndex = searchHelper.SearchForClusterIndex("Nutrient Solution", "Plant", __nullptr);
        saNutrientSolnRenewIndex = searchHelper.SearchForClusterIndex("Nutrient Solution Renew", "Plant", __nullptr);


        // want Root Radius only per-RootOrder. Well apparently its per-Plant/RootOrder
        saRootRadiusPerPlantIndex = searchHelper.SearchForClusterIndex("Root Radius", "Plant", variation_names);

        // search for root length, to iterate by Plant, varied per Plant
        saRootLengthPerPlantIndex = searchHelper.SearchForClusterIndex("Root Length Wrap None", "Plant", __nullptr);

        // search for Root Length, to iterate by Plant, varied per Plant+RootOrder (variation_names)
        saRootLengthPerPlantPerRootOrderIndex = searchHelper.SearchForClusterIndex("Root Length Wrap None", "Plant", variation_names);

        // MSA 10.11.10 Determine if it's a nutrient solution experiment and if we want to calculate evaporation
        const SharedAttributeCluster& cluster = Get_Cluster("Plant");
        SharedAttributeCluster::const_iterator cluster_iter = cluster.begin();

        const SharedAttributeList& sa_list = (*cluster_iter).second;
        SharedAttribute* nutrient_solution_sa = sa_list[saNutrientSolutionIndex];
        m_IS_NUTRIENT_SOLUTION = nutrient_solution_sa->GetValue() == 1.0; // 0 = soil experiment, 1 = nutrient solution experiment
        m_CALCULATE_EVAPORATION = saCalculateEvap->GetValue() == 1; // 1 == calculate evaporation, 0 == don't calculate evaporation
        m_MAINTAIN_WATER_CONTENT = Get_Concentration_Of_BoxIndex(water_MaintainWaterIndex, 0) == 1.0; // 1 == maintain water content, 0 == let water content be reduced
        m_DRAW_WETTING_FRONT = Get_Concentration_Of_BoxIndex(water_DrawWettingFrontIndex, 0) != 0.0; //does the user want to draw the wetting front on the screen or not?
    }


    /* ********************************************************************************** */
    long int Water::StartUp(ProcessActionDescriptor* action)
    {
        // normalise time_of_previous_X values if required
        Use_Time;
        if (water_TimeOfPreviousLeaching < 0.0)
        {
            water_TimeOfPreviousLeaching = time;
        }
        if (water_TimeOfPreviousWaterUptake < 0.0)
        {
            water_TimeOfPreviousWaterUptake = time;
        }
        if (water_TimeOfPreviousEvaporation < 0.0)
        {
            water_TimeOfPreviousEvaporation = time;
        }
        if (water_TimeOfPreviousRedistribution < 0.0)
        {
            water_TimeOfPreviousRedistribution = time;
        }
        if (water_TimeOfPreviousCall < 0.0)
        {
            water_TimeOfPreviousCall = time;
        }
        if (water_TimeOfPreviousRain < 0.0)
        {
            water_TimeOfPreviousRain = time;
        }
        return (Process::StartUp(action));
    }


    /* ********************************************************************************** */
    // MSA 09.09.10 Changed all time variables to type ProcessTime_t
    long int Water::DoNormalWakeUp(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_Time;
        Use_Scoreboard;

        // MSA 09.10.01    Unused
        /*
        const ProcessTime_t dT = (time - water_TimeOfPreviousCall);
        const ProcessTime_t current_time = time - action->GetPostOffice()->GetStart();
        */
        const int Water_Routine_Number = Get_Concentration_Of_BoxIndex(water_WaterRoutineIndex, 0);

        if (!m_IS_NUTRIENT_SOLUTION) //roots are growing in soil
        {
            if (m_CALCULATE_EVAPORATION)
            {
                return_value |= EvaporateWater(action);
            }
            return_value |= UptakeWater(action);

            RmAssert(Water_Routine_Number == 1 || Water_Routine_Number == 2, "Specified Water Redistribution Routine must be 1 or 2");
            return_value |= RedistributeWater(action, Water_Routine_Number);
        }
        else //roots are growing in nutrient solution
        {
            return_value |= UptakeWater(action); //nutrient solution in which there is good aeration and nutrient mixing and solution is changed every x days
        }

        water_TimeOfPreviousCall = time;

        return_value |= Process::DoNormalWakeUp(action);

        return (return_value);
    }


    /* ********************************************************************************** */

    long int Water::DoScoreboardPlantSummaryRegistered(ProcessActionDescriptor* action)
    {
        Use_PlantSummaryRegistration;

        water_first_plant_summary_index = psr->GetStartIndex();
        water_number_of_plants = psr->GetNumberOfPlants();
        water_number_of_branch_orders = psr->GetNumberOfBranchOrders();

        return (Process::DoScoreboardPlantSummaryRegistered(action));
    }

    /* ********************************************************************************** */

    /* DoSpecialInput
    98.10.22 RvH    inclusion */
    long int Water::DoSpecialInput(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        RainfallEvent* rainfall_event = (RainfallEvent *)(action->GetData());

        if (rainfall_event)
        {
            return_value = AddRainfall(action, rainfall_event->rainfall_rad,
                rainfall_event->rainfall_maxT, rainfall_event->rainfall_minT,
                rainfall_event->rainfall_amount, rainfall_event->rainfall_pan);

            delete rainfall_event;
            // Remove the deleted pointer from the local list
            m_rainfallEvents.erase(std::find(m_rainfallEvents.begin(), m_rainfallEvents.end(), rainfall_event));
        }

        return (return_value);
    }

    /* ********************************************************************************** */

    /* DoSpecialInput
    98.10.22 RvH    inclusion */
    long int Water::AddRainfall(ProcessActionDescriptor* action, double rad, double maxT, double minT, double amount, double pan)
    {
        Use_ReturnValue;
        Use_Scoreboard;
        Use_VolumeObjectCoordinator;
        // MSA 09.09.10 removed unnecessary Use_ macro calls here (and elsewhere)

        // MSA 09.10.12 Implemented rangewise setting of rained amount

        // MSA 10.12.02 Updated to support barrier modelling.
        // We assume, using this, that rain may freely enter the top of VolumeObjects and is uniformly distributed.
        // Gotta partition rainfall into restricted and unrestricted segments of Boxes.
        for (long x = 1; x <= NUM_X; ++x)
        {
            for (long y = 1; y <= NUM_Y; ++y)
            {
                const BoxIndex box_index = scoreboard->GetBoxIndex(x, y, 1); // Recall that we're only interested in the top layer (z==1) here.

                SUBSECTION_ITER_BEGIN

                    // MSA 11.06.07 Important note: Rained amount is in units of cm. As in reading off a rain gauge.
                    // So every subsection will get the same amount rained into it, regardless of size.
                    // Rained amount is NOT a volume or mass of water.
                    scoreboard->SetCharacteristicValue(Rained_Amount_Indices[idx], amount, box_index);

                SUBSECTION_ITER_END
            }
        }

        // Finally, set all other boxes to 0
        for (CharacteristicIndices::const_iterator citer = Rained_Amount_Indices.begin(); citer != Rained_Amount_Indices.end(); ++citer)
        {
            scoreboard->SetCharacteristicValueRange(*citer, 0.0, 1, 1, 2, NUM_X, NUM_Y, NUM_Z);
        }

        // MSA 09.10.09 Converted each weather datum to an individual [Water] ProcessSharedAttribute
        saPanEvaporation->SetValue(pan);
        saRainAmount->SetValue(amount);
        saMaxTemperature->SetValue(maxT);
        saMinTemperature->SetValue(minT);
        saRadiation->SetValue(rad);

        if (amount > 0.0)
        {
            return_value = Drain(action); //if there has been some rain move it down through the profile
        }

        return (return_value);
    }


    /* ********************************************************************************** */
    // VMD 30/09/98  The redistribute routine has been separated from the water uptake and the evaporation.  Redistribution is now based
    // on Darcy's law for soil water movement rather than being an averaging routine.  This removes the dependance on box size - ie
    // will get the same water movement regardless of box size.
    // VMD 18/07/2009 Rewritring the redistribute routine to provide the user with 2 levels of modelling.  Level 1: water movement is based on
    // soil capacity parameters only.  Level 2: water movement is soil water potential based, using van Genuchten's relation for hydraulic conductivity.
    //
    // MSA 09.09.11 Rewriting RedistributeWater code to use one function, parameterised for multiple (currently 2) modelling routines,
    // Level 1 and Level 2 as above
    // MSA 10.12.02 Rewriting the whole method for easier integration with barrier modelling.
    // MSA 11.01.07 ...and rewriting the rewrite for better functionality and just about an order of magnitude less LOC
    long int Water::RedistributeWater(ProcessActionDescriptor* action, const int& routineNumber)
    {
        Use_ReturnValue;
        Use_Time;
        Use_Scoreboard;
        Use_PostOffice;
        Use_VolumeObjectCoordinator;
        Use_ScoreboardCoordinator;
        Use_Boundaries;

        // MSA 09.09.10 Changed all time variables to type ProcessTime_t
        const ProcessTime_t dT = (time - water_TimeOfPreviousRedistribution);
        water_TimeOfPreviousRedistribution = time;

        if (dT == 0)
        {
            return kNoError;
        }
        RmAssert(dT > 0, "Time delta since previous water redistribution is negative");

        //Look at each box in the scoreboard and calculate the water movement to surrounding boxes.
        for (BoxCoordinateIndex y = 1; y <= NUM_Y; ++y)
        {
            for (BoxCoordinateIndex x = 1; x <= NUM_X; ++x)
            {
                for (BoxCoordinateIndex z = 1; z <= NUM_Z; ++z)
                {
                    const BoxIndex thisBox = scoreboard->GetBoxIndex(x, y, z);
                    std::vector<BoxCoordinate> nextBoxCoordinates;
                    nextBoxCoordinates.push_back(BoxCoordinate(x, y, z));
                    if (x < NUM_X) nextBoxCoordinates.push_back(BoxCoordinate(x + 1, y, z));
                    if (x > 1) nextBoxCoordinates.push_back(BoxCoordinate(x - 1, y, z));
                    if (y < NUM_Y) nextBoxCoordinates.push_back(BoxCoordinate(x, y + 1, z));
                    if (y > 1) nextBoxCoordinates.push_back(BoxCoordinate(x, y - 1, z));
                    if (z < NUM_Z) nextBoxCoordinates.push_back(BoxCoordinate(x, y, z + 1));
                    if (z > 1) nextBoxCoordinates.push_back(BoxCoordinate(x, y, z - 1));

                    for (std::vector<BoxCoordinate>::const_iterator bciter = nextBoxCoordinates.begin(); bciter != nextBoxCoordinates.end(); ++bciter)
                    {
                        const BoxIndex nextBox = scoreboard->GetBoxIndex(bciter->x, bciter->y, bciter->z);

                        Dimension plane;
                        bool isInPositiveDir;
                        scoreboard->GetDirectionBetweenColinearBoxes(thisBox, nextBox, plane, isInPositiveDir);

                        const double fromDist = (plane == X) ? (Get_Layer_Width(x)) : ((plane == Y) ? (Get_Layer_Depth(y)) : (Get_Layer_Height(z)));
                        const double toDist = (plane == X) ? (Get_Layer_Width(bciter->x)) : ((plane == Y) ? (Get_Layer_Depth(bciter->y)) : (Get_Layer_Height(bciter->z)));

                        const TransferRestrictionData& trd = scoreboard->GetTransferRestrictionData(thisBox, nextBox, scoreboard->GetTransferRestrictedDimension(plane, isInPositiveDir));
                        //const size_t thisBackgroundVOIndex = trd.backgroundVolumeObjectFrom==NULL ? 0 : trd.backgroundVolumeObjectFrom->GetIndex(); // MSA 11.07.12 Unused
                        const size_t nextBackgroundVOIndex = trd.backgroundVolumeObjectTo == __nullptr ? 0 : trd.backgroundVolumeObjectTo->GetIndex();

                        const bool thisRestricted = trd.restrictedProportionFrom > 0.0;
                        const bool nextRestricted = trd.restrictedProportionTo > 0.0;

                        SUBSECTION_ITER_WITH_TO_AND_FROM_BOXES_AND_PROPORTIONS_BEGIN

                            if (thisProportion <= 0.0 || nextProportion <= 0.0) continue;

                        // MSA 10.12.02 We start by applying any redistribution applicable WITHIN this box.
                        if (nextBox == thisBox)
                        {
                            // MSA This is a nested invocation of VolumeObject iteration.
                            // As such we don't use the macros here, because there would be variable naming conflicts.
                            double nnp = 1;
                            VolumeObjectList::const_iterator voliter = volumeobjectcoordinator->GetVolumeObjectList().begin();
                            for (size_t i = 0; i <= VO_COUNT; ++i)
                            {
                                // Get pointer to the VolumeObject, or a NULL pointer if it's the last iteration of the loop. Also determine proportions.
                                double np;
                                VolumeObject* volumeObject = __nullptr;
                                if (i < VO_COUNT)
                                {
                                    volumeObject = *voliter;
                                    ++voliter;
                                    np = scoreboard->GetCoincidentProportion(nextBox, volumeObject->GetIndex());
                                    nnp -= np;
                                }
                                else
                                {
                                    np = Utility::CSMax(0.0, nnp);
                                }

                                if (np <= 0.0) continue;

                                const size_t index = volumeObject == __nullptr ? 0 : volumeObject->GetIndex();

                                if (index == idx) continue; // Can't redistribute within the same subsection!

                                const double permeability = trd.voFromPermeability;//OLD: volumeObject!=NULL ? volumeObject->GetFluidPermeability() : vo->GetFluidPermeability();

                                RmAssert(permeability == trd.voToPermeability, "Error: VolumeObject permeabilities must be the same (both should refer to the same VO)");

                                if (permeability <= 0.0) continue; // Saves a bit of computation time in the event that barriers are impermeable

                                // Call the actual redistribution method twice, once for each direction. Redistribution will only occur if the from-section's water content
                                // is higher than the to-section's.
                                for (size_t i = 0; i < 2; ++i)
                                {
                                    const size_t thisFromIndex = i == 0 ? idx : index;
                                    const size_t thisToIndex = i == 0 ? index : idx;
                                    const double thisProportion = scoreboard->GetCoincidentProportion(thisBox, thisFromIndex);
                                    const double nextProportion = scoreboard->GetCoincidentProportion(thisBox, thisToIndex);

                                    // Attempt to redistribute some water from:    (i==0)    the restricted section to the unrestricted, then
                                    //                                            (i==1)    the unrestricted section to the restricted.
                                    DoRedistribution(scoreboard, routineNumber, thisBox, thisBox, thisFromIndex, thisToIndex, thisProportion, nextProportion, fromDist, toDist, dT, permeability);
                                }
                            }
                        }
                        // Then do redistribution from this box section to the adjacent box's corresponding section.
                        else
                        {
                            VolumeObject* backgroundVOFrom = trd.backgroundVolumeObjectFrom;
                            VolumeObject* backgroundVOTo = trd.backgroundVolumeObjectTo;
                            // N.B. backgroundVOs will usually be NULL. So vo==backgroundVO USUALLY means vo==NULL, i.e. the no-VO subsection. But not always.
                            // Couple of ugly logic stacks here:
                            const bool thisSectionRestricted = thisRestricted // Obviously if the box has no restriction, the section is not restricted
                                && (vo == backgroundVOFrom ? (trd.unrestrictedTransferWithinVO) // If this is the no-VO section, it's only restricted if the TRD says so
                                    : (!trd.unrestrictedTransferWithinVO && (idx == trd.volumeObjectFrom->GetIndex()))); // If this is a VO section, it's restricted if restricted transfer is within the VO and the VO we're examining is the same as in the TRD.
                            const bool nextSectionRestricted = nextRestricted // Obviously if the box has no restriction, the section is not restricted
                                && (vo == backgroundVOTo ? (trd.unrestrictedTransferWithinVO) // If this is the no-VO section, it's only restricted if the TRD says so
                                    : (!trd.unrestrictedTransferWithinVO && (idx == trd.volumeObjectTo->GetIndex()))); // If this is a VO section, it's restricted if restricted transfer is within the VO and the VO we're examining is the same as in the TRD.

                            if (!thisSectionRestricted && !nextSectionRestricted)
                            {
                                const double thisProportion = scoreboard->GetCoincidentProportion(thisBox, idx);
                                const double nextProportion = scoreboard->GetCoincidentProportion(nextBox, idx);
                                DoRedistribution(scoreboard, routineNumber, thisBox, nextBox, idx, idx, thisProportion, nextProportion, fromDist, toDist, dT);
                            }
                        }
                        SUBSECTION_ITER_WITH_TO_AND_FROM_BOXES_AND_PROPORTIONS_END


                            //Finally, handle for the special case in which thisBox is 100% restricted and the surface separating it from nextBox is permeable.
                            if (trd.restrictedProportionFrom == 1.0 && trd.voFromPermeability > 0.0)
                            {
                                const size_t subsectionToTransferTo = (trd.unrestrictedTransferWithinVO) ? (trd.volumeObjectTo == __nullptr ? nextBackgroundVOIndex : trd.volumeObjectTo->GetIndex()) : (nextBackgroundVOIndex);

                                const double thisProportion = scoreboard->GetCoincidentProportion(thisBox, trd.volumeObjectFrom->GetIndex());
                                const double nextProportion = scoreboard->GetCoincidentProportion(nextBox, subsectionToTransferTo);
                                DoRedistribution(scoreboard, routineNumber, thisBox, nextBox, trd.volumeObjectFrom->GetIndex(), subsectionToTransferTo, thisProportion, nextProportion, fromDist, toDist, dT, trd.voFromPermeability);
                            }
                    }
                } //for (z=1; z<=NUM_Z; ++z)
            } //for (x=1; x<=NUM_X; ++x)
        } //for (y=1; y<=NUM_Y; ++y)

        // Because there has been some massflow of water there needs to be mass flow of nitrate
        Send_GeneralPurpose(time, water_NitrateModule, __nullptr, kWaterRedistributed);

        return (return_value);
    }


    // MSA 11.02.09 This method does the actual redistribution between two spatial-subsections of adjacent boxes.
    // It does not check for transfer restriction; callers of this method should first ensure that the requested redistribution is legal.
    // The parameter volumeObjectSurfacePermeability is only used in the case of redistribution between different spatial subsections of the same Box.
    bool Water::DoRedistribution(Scoreboard* scoreboard, const int& routineNumber, const BoxIndex& fromBox, const BoxIndex& toBox, const size_t& fromVOIndex, const size_t& toVOIndex, const double& fromProportion, const double& toProportion, const double& fromBoxDistance, const double& toBoxDistance, const ProcessTime_t& dT, const double& volumeObjectSurfacePermeability /* = 0.0 */)
    {
        if (fromBox == toBox && (fromVOIndex == toVOIndex || volumeObjectSurfacePermeability <= 0.0)) return false;

        if (fromProportion <= 0.0 || toProportion <= 0.0) return false;

        const double redistributionRestrictionModifier = fromVOIndex != toVOIndex ? volumeObjectSurfacePermeability : 1.0;

        if (redistributionRestrictionModifier <= 0.0) return false; // Shortcut exit

        // MSA Note to self: These two Characteristics are conceptually linked.
        // If the water_moved amount is nonzero, it must have a direction.
        scoreboard->SetCharacteristicValue(Water_Moved_Indices[fromVOIndex], 0.0, fromBox);
        scoreboard->SetCharacteristicValue(Direction_Moved_Indices[fromVOIndex], 0.0, fromBox);
        const double volumeFrom = fromProportion * Get_BoxIndex_Volume(fromBox);
        const double volumeTo = toProportion * Get_BoxIndex_Volume(toBox);

        double waterContentFrom = scoreboard->GetCharacteristicValue(Water_Content_Indices[fromVOIndex], fromBox);
        double waterContentTo = scoreboard->GetCharacteristicValue(Water_Content_Indices[toVOIndex], toBox);
        double waterAmountFrom = waterContentFrom * volumeFrom; //cm3 H2O

        // Now ascertain that conditions are such that redistribution will occur.
        if (routineNumber == 1)
        {
            // If using routine 1, we can check it here.
            if (waterContentTo >= waterContentFrom) return false;
        }
        // If using routine 2, we need to use water potentials, so we'll check it later.


        // MSA These read-only parameters (cannot be const because they must be initially assigned inside a switch case)
        // are only used by the Level 1 routine...
        // MSA 09.09.25 Initialised parameters to 0.0 where necessary to silence compiler warnings
        double wiltingPointFrom, coeffAFrom = 0.0, coeffBFrom = 0.0;
        double wiltingPointTo, coeffATo = 0.0, coeffBTo = 0.0;
        // ... and these are only used by the Level 2 routine.
        double wiltingPointPotential;
        double KsatFrom = 0.0, alphaFrom = 0.0, waterSatFrom = 0.0, coeffMFrom = 0.0, coeffNFrom = 0.0, waterPotentialFrom = 0.0;
        double KsatTo = 0.0, alphaTo = 0.0, waterSatTo = 0.0, coeffMTo = 0.0, coeffNTo = 0.0, waterPotentialTo = 0.0;
        // These read/write parameters are only used be the Level 2 routine.

        switch (routineNumber)
        {
        case 1:
            wiltingPointFrom = scoreboard->GetCharacteristicValue(Wilting_Point_Indices[fromVOIndex], fromBox);
            coeffAFrom = scoreboard->GetCharacteristicValue(Coefficient_A_Indices[fromVOIndex], fromBox); //user defined indices for the soil water diffusivityFrom curve
            coeffBFrom = scoreboard->GetCharacteristicValue(Coefficient_B_Indices[fromVOIndex], fromBox);
            if (waterContentFrom <= wiltingPointFrom) return false; // MSA Water content below wilting point: No redistribution for this box/subsection

            wiltingPointTo = scoreboard->GetCharacteristicValue(Wilting_Point_Indices[toVOIndex], toBox);
            coeffATo = scoreboard->GetCharacteristicValue(Coefficient_A_Indices[toVOIndex], toBox); //user defined indices for the soil water diffusivityFrom curve
            coeffBTo = scoreboard->GetCharacteristicValue(Coefficient_B_Indices[toVOIndex], toBox);
            break;
        case 2:
            wiltingPointPotential = -15000; //cmH20
            KsatFrom = scoreboard->GetCharacteristicValue(K_Saturation_Indices[fromVOIndex], fromBox);
            alphaFrom = scoreboard->GetCharacteristicValue(Alpha_Indices[fromVOIndex], fromBox); //user defined indicies for the van Genuchten relation
            waterSatFrom = scoreboard->GetCharacteristicValue(Saturation_Indices[fromVOIndex], fromBox);
            coeffMFrom = scoreboard->GetCharacteristicValue(Coefficient_M_Indices[fromVOIndex], fromBox);
            coeffNFrom = scoreboard->GetCharacteristicValue(Coefficient_N_Indices[fromVOIndex], fromBox);
            waterPotentialFrom = -(1 / alphaFrom) * ProcessUtility::guardedPow((ProcessUtility::guardedPow((waterSatFrom / waterContentFrom), (1 / coeffMFrom), _logger) - 1), (1 / coeffNFrom), _logger); //-cmH20

            if (waterPotentialFrom <= wiltingPointPotential) return false; // MSA Water potential below wilting point: No redistribution for this box/subsection

            KsatTo = scoreboard->GetCharacteristicValue(K_Saturation_Indices[toVOIndex], toBox);
            alphaTo = scoreboard->GetCharacteristicValue(Alpha_Indices[toVOIndex], toBox); //user defined indicies for the van Genuchten relation
            waterSatTo = scoreboard->GetCharacteristicValue(Saturation_Indices[toVOIndex], toBox);
            coeffMTo = scoreboard->GetCharacteristicValue(Coefficient_M_Indices[toVOIndex], toBox);
            coeffNTo = scoreboard->GetCharacteristicValue(Coefficient_N_Indices[toVOIndex], toBox);
            waterPotentialTo = -(1 / alphaTo) * ProcessUtility::guardedPow((ProcessUtility::guardedPow((waterSatTo / waterContentTo), (1 / coeffMTo), _logger) - 1), (1 / coeffNTo), _logger); //-cmH20
            if (waterPotentialTo >= waterPotentialFrom) return false;
            break;
        }

        double waterAmountTo = waterContentTo * volumeTo; //cm3 H20
        double waterAmountTotal = waterAmountFrom + waterAmountTo; //cm3 H20

        double fromSectionDistance = fromProportion * fromBoxDistance;
        double toSectionDistance = toProportion * toBoxDistance;

        // Important note: If we are redistributing between subsections of a single box, we must also halve the effective distance.

        if (fromBox == toBox)
        {
            fromSectionDistance /= 2.0;
            toSectionDistance /= 2.0;
        }

        double avgDistance = 0.0;
        if (volumeFrom < 1e-3 && volumeTo > 1e-2)
        {
            avgDistance = fromSectionDistance;
        }
        else if (volumeFrom > 1e-2 && volumeTo < 1e-3)
        {
            avgDistance = toSectionDistance;
        }
        else
        {
            avgDistance = (fromSectionDistance + toSectionDistance) / 2.0;
        }

        // MSA Currently there are two possible water redistribution routines (VMD 09.07.18):
        //        Level 1: water movement is based on soil capacity parameters only. 
        //        Level 2: water movement is soil water potential based, using van Genuchten's relation for hydraulic conductivity.

        // MSA 10.09.20 We also check the restriction in the given direction due to barrier modelling, before applying the water movement.
        switch (routineNumber)
        {
        case 1:
            // MSA This pair of braces is required to limit the scope of the local variables
            // (deltaW, waterEquilFrom, etc) to this case
        {
            const double diffusivityFrom = coeffAFrom * exp(coeffBFrom * waterContentFrom); //cm2/s
            const double diffusivityTo = coeffATo * exp(coeffBTo * waterContentTo); //cm2/s

            double avgDiffusivity = diffusivityFrom * (fromSectionDistance / (fromSectionDistance + toSectionDistance)) + diffusivityTo * (toSectionDistance / (fromSectionDistance + toSectionDistance)); //average diffusivity

            if (avgDiffusivity < 0.0) avgDiffusivity = 0.0;

            const double deltaW = redistributionRestrictionModifier * avgDiffusivity * (waterContentFrom - waterContentTo) * avgDistance * dT;

            const double waterEquilTo = waterAmountTotal / ((volumeFrom / volumeTo) + 1.0);
            const double waterEquilFrom = waterAmountTotal - waterEquilTo;

            if (deltaW > (waterAmountFrom - waterEquilFrom))
            {
                //Water moved is in cm3 water/cm2 soil.
                const double waterMoved = ((waterAmountFrom - waterEquilFrom) / volumeFrom) * avgDistance;
                scoreboard->SetCharacteristicValue(Water_Moved_Indices[fromVOIndex], waterMoved, fromBox);

                waterAmountFrom = waterEquilFrom;
                waterAmountTo = waterEquilTo;
            }
            else
            {
                const double waterMoved = ((waterAmountFrom - waterEquilFrom) / volumeFrom) * avgDistance;
                scoreboard->SetCharacteristicValue(Water_Moved_Indices[fromVOIndex], waterMoved, fromBox);

                waterAmountFrom -= deltaW;
                waterAmountTo += deltaW;
            }
        }
        break;
        case 2:
            // MSA This pair of braces is required to limit the scope of the local variables
            // (deltaW, KunsatTo, etc) to this case
        {
            const double KunsatTo = KsatTo * ProcessUtility::guardedPow(1.0 - ProcessUtility::guardedPow(alphaTo * (-waterPotentialTo), coeffNTo - 1.0, _logger) * ProcessUtility::guardedPow(1.0 + ProcessUtility::guardedPow(alphaTo * (-waterPotentialTo), coeffNTo, _logger), -coeffMTo, _logger), 2.0, _logger) / ProcessUtility::guardedPow(1 + ProcessUtility::guardedPow(alphaTo * (-waterPotentialTo), coeffNTo, _logger), coeffMTo / 2.0, _logger); //cm.s-1
            const double KunsatFrom = KsatFrom * ProcessUtility::guardedPow(1.0 - ProcessUtility::guardedPow(alphaFrom * (-waterPotentialFrom), coeffNFrom - 1.0, _logger) * ProcessUtility::guardedPow(1 + ProcessUtility::guardedPow(alphaFrom * (-waterPotentialFrom), coeffNFrom, _logger), -coeffMFrom, _logger), 2.0, _logger) / ProcessUtility::guardedPow(1 + ProcessUtility::guardedPow(alphaFrom * (-waterPotentialFrom), coeffNFrom, _logger), coeffMFrom / 2.0, _logger); //cm.s-1

            const double avgKunsat = KunsatFrom * (fromSectionDistance / (fromSectionDistance + toSectionDistance)) + KunsatTo * (toSectionDistance / (fromSectionDistance + toSectionDistance));

            const double deltaW = redistributionRestrictionModifier * avgKunsat * (waterPotentialFrom - waterPotentialTo) * dT * avgDistance; //cm3 H20

            //Water moved is in cm3 water/cm2 soil.
            const double waterMoved = (deltaW / volumeFrom) * avgDistance;
            scoreboard->SetCharacteristicValue(Water_Moved_Indices[fromVOIndex], waterMoved, fromBox);

            waterAmountFrom -= deltaW;
            waterAmountTo += deltaW;
            break;
        }
        }

        waterContentFrom = waterAmountFrom / volumeFrom;
        waterContentTo = waterAmountTo / volumeTo;
        RmAssert(waterContentFrom >= 0.0 && waterContentTo >= 0.0, "Logical error: water content cannot be negative");

        // Find direction moved
        int direction = 0;
        Dimension plane;
        bool isInPositiveDir;
        if (scoreboard->GetDirectionBetweenColinearBoxes(fromBox, toBox, plane, isInPositiveDir))
        {
            direction = plane == X ? 1 : (plane == Y ? 2 : 3);
            if (!isInPositiveDir) direction *= -1;
        }

        // MSA TODO Is it safe for within-Box redistribution to have a 0 direction?

        scoreboard->SetCharacteristicValue(Water_Content_Indices[fromVOIndex], waterContentFrom, fromBox);
        scoreboard->SetCharacteristicValue(Water_Content_Indices[toVOIndex], waterContentTo, toBox);
        scoreboard->SetCharacteristicValue(Direction_Moved_Indices[fromVOIndex], direction, fromBox);

        // MSA Water redistribution for this box section complete.

        return true;
    }


    /* ********************************************************************************** */
    // VMD 30/09/98 The evaporate routine separated from water uptake and redistribution. It is initially set up such that water is only taken from the top
    // row of boxes to satisfy the evaporation demand.  This requires the evaporation routine to be called often so that only a small amount
    // of water is taken out at a time, and the redistribution routine is called to "pull" the water up from lower boxes.
    long int Water::EvaporateWater(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_Time;

        //  20030328 RvH have aggregated all the objects into one declaration, which
        //           also now includes the new SharedAttributeManager
        //UseObjects;
        Use_Scoreboard;
        Use_ScoreboardCoordinator;
        Use_SharedAttributeManager;
        Use_VolumeObjectCoordinator;
        Use_Boundaries;

        const ProcessTime_t dT = (time - water_TimeOfPreviousEvaporation);
        water_TimeOfPreviousEvaporation = time;

        if (dT == 0) { return kNoError; }
        RmAssert(dT > 0, "Time delta since previous water evaporation is negative");

        //VMD At this stage time does not start from 0/0/0 0:0:0, but from a specified date, so need to subtract the start time to
        //get the time since the start of the simulation.
        const ProcessTime_t current_time = time - action->GetPostOffice()->GetStart();

        const SharedAttributeCluster& cluster = Get_Cluster("Plant");
        SharedAttributeCluster::const_iterator cluster_iter = cluster.begin();

        // This is "use the next Plant X attribute list".
        // Note that the name of the Plant/Cluster is in (*cluster_iter).first
        const SharedAttributeList& sa_list = (*cluster_iter).second;

        SharedAttribute* germination_lag_sa = sa_list[saGerminationLagIndex]; //Time in hours for the seeds to germinate after being sown
        // MSA 09.09.14 Explicitly showing the cast from double to ProcessTime_t
        const ProcessTime_t time_of_germination = static_cast<ProcessTime_t>(PROCESS_HOURS(germination_lag_sa->GetValue())); //converting the germination lag from hours to seconds
        SharedAttribute* seeding_time_sa = sa_list[saSeedingTimeIndex]; //Time from zero (start of simulation) in hours when the seeds are sown
        const ProcessTime_t time_of_seeding = static_cast<ProcessTime_t>(PROCESS_HOURS(seeding_time_sa->GetValue())); //converting the seeding time from hours to seconds
        const ProcessTime_t grow_start_time = time_of_seeding + time_of_germination; //Time in seconds from the start of the simulation (time zero)when seed started to grow

        const double residue_cover = saResidueCover->GetValue();
        //all plants could have a unique tops growth.  This is for calculating the average ground cover (see further comments below)    
        const double average_gc_factor = CSMin(1.0, saAvgGreenCoverFactor->GetValue() + residue_cover);

        double potential_evaporation = saPanEvaporation->GetValue(); //pan evaporation mm.day-1
        potential_evaporation /= static_cast<double>(PROCESS_DAYS(10)); //[cm3.cm-2.s-1], cm.s-1
        potential_evaporation *= (1.0 - average_gc_factor); //[cm3.cm-2.s-1], cm.s-1

        //Working our way through the scoreboard, z=1
        const double box_height = Get_Layer_Height(1);

        for (long y = 1; y <= NUM_Y; ++y)
        {
            for (long x = 1; x <= NUM_X; ++x)
            {
                const BoxIndex box_index = scoreboard->GetBoxIndex(x, y, 1);

                SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                    if (thisProportion <= 0.0) continue;

                double water_content = scoreboard->GetCharacteristicValue(Water_Content_Indices[idx], box_index);
                const double wilting_point = scoreboard->GetCharacteristicValue(Wilting_Point_Indices[idx], box_index);
                const double drained_upper_limit = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], box_index);

                //level 1 modelling 'fraction extractable' function represents the generic moisture release curve
                //with a water availability of 0.9999 at the DUL, and 0.00001 at the wilting point
                const double coeff_a = 1.0 / exp(-(((20.723 * wilting_point) / (drained_upper_limit - wilting_point)) + 11.513));
                const double coeff_b = 20.723 / (drained_upper_limit - wilting_point);

                // MSA 10.10.28 Should barrier modelling apply here? If there's a VO surface covering this Scoreboard Box, can it lose water to evaporation?
                const double fraction_extractable = 1.0 / (1.0 + coeff_a * exp(-coeff_b * water_content));
                water_CumulativeEvaporation += fraction_extractable * potential_evaporation * dT; // cm3.cm-2, cmH2O

                if (current_time < grow_start_time)
                {
                    saCumulEvapBeforeGermination->SetValue(water_CumulativeEvaporation);
                }
                else
                {
                    saCumulEvapAfterGermination->SetValue(water_CumulativeEvaporation);
                }

                // MSA 11.02.07 Applied the barrier modelling proportion here, because we are calculating a proportion, not a raw amount.
                // TODO ensure this is valid
                // MSA 11.03.08 No, it's not. Fraction_extractable is derived from variables which are all specific to this spatial subsection.
                // So, the relative volume of this subsection is irrelevant.
                const double evapProportion = (fraction_extractable * potential_evaporation * dT) / (box_height); // cm3H2O/cm3Soil

                water_content -= evapProportion;

                if (water_content < 0.0)
                {
                    water_content = 0.0;
                }
                scoreboard->SetCharacteristicValue(Water_Content_Indices[idx], water_content, box_index);
                water_AverageEvaporation += thisProportion * fraction_extractable * potential_evaporation; //cm3.cm-2.s-1

                SUBSECTION_ITER_WITH_PROPORTIONS_END
            } //for (x=1; x<=NUM_X; ++x)
        } // for (y=1; y<=NUM_Y; ++y) 

        water_AverageEvaporation /= static_cast<double>(NUM_Y * NUM_X);

        return (return_value);
    }

    /* ********************************************************************************** */

    long int Water::UptakeWater(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_Time;
        Use_Scoreboard;
        Use_ScoreboardCoordinator;
        Use_SharedAttributeManager;
        Use_VolumeObjectCoordinator;
        Use_Boundaries;

        //Time variables
        const ProcessTime_t dT = (time - water_TimeOfPreviousWaterUptake);
        water_TimeOfPreviousWaterUptake = time;
        if (dT == 0)
        {
            return kNoError;
        }
        RmAssert(dT > 0, "Time delta since previous water uptake is negative");
        //VMD At this stage time does not start from 0/0/0 0:0:0, but from a specified date, so need to subtract the start time to
        //get the time since the start of the simulation.
        const ProcessTime_t current_time = time - action->GetPostOffice()->GetStart();
        long plant_counter = 0;
        double soil_block_depth = 0.0;
        double soil_block_width = 0.0;
        for (long y = 1; y <= NUM_Y; ++y) { soil_block_depth += Get_Layer_Depth(y); }
        for (long x = 1; x <= NUM_X; ++x) { soil_block_width += Get_Layer_Width(x); }
        const double soil_block_area = soil_block_depth * soil_block_width;

        double average_gc_factor = 0.0;
        double residue_cover = 0.0;

        // This attempts (usually successfully :-) to get a Cluster of
        // SharedAttributeLists, clustered according to their variation by Plant
        const SharedAttributeCluster& cluster = Get_Cluster("Plant");

        //Calculating the ground cover first
        for (SharedAttributeCluster::const_iterator cluster_iter = cluster.begin();
            cluster_iter != cluster.end();
            ++cluster_iter
            )
        {
            // RvH This is "use the next Plant X attribute list".
            // Note that the name of the Plant/Cluster is in (*cluster_iter).first
            const SharedAttributeList& sa_list = (*cluster_iter).second;

            SharedAttribute* germination_lag_sa = sa_list[saGerminationLagIndex]; //Time in hours for the seeds to germinate after being sown
            // MSA 09.09.14 Explicitly showing the cast from double to ProcessTime_t
            const ProcessTime_t time_of_germination = static_cast<ProcessTime_t>(PROCESS_HOURS(germination_lag_sa->GetValue())); //converting the germination lag from hours to seconds
            SharedAttribute* seeding_time_sa = sa_list[saSeedingTimeIndex]; //Time from zero (start of simulation) in hours when the seeds are sown
            const ProcessTime_t time_of_seeding = static_cast<ProcessTime_t>(PROCESS_HOURS(seeding_time_sa->GetValue())); //converting the seeding time from hours to seconds
            const ProcessTime_t grow_start_time = time_of_seeding + time_of_germination; //Time in seconds from the start of the simulation (time zero)when seed started to grow

            if (current_time <= grow_start_time) { return (return_value); } //no water uptake by this plant before germination. Go to next plant.

            SharedAttribute* time_full_cover_sa = sa_list[saTimeFullCoverIndex]; //time, days after germination that full cover occurs
            SharedAttribute* time_ripening_sa = sa_list[saTimeRipeningIndex]; //time, days after germination that ripening occurs
            SharedAttribute* time_harvest_sa = sa_list[saTimeHarvestIndex]; //time, days after germination that harvest occurs
            SharedAttribute* dd_full_cover_sa = sa_list[saDegDaysFullCoverIndex]; //degree days after germination that full cover occurs
            SharedAttribute* dd_ripening_sa = sa_list[saDegDaysRipeningIndex]; //degree days after germination that ripening occurs
            SharedAttribute* dd_harvest_sa = sa_list[saDegDaysHarvestIndex]; //degree days after germination that harvest occurs
            SharedAttribute* dd_model_sa = sa_list[saDegreeDaysModelIndex]; //is the crop cover model based on degree days (1), or time in days (0)
            SharedAttribute* ground_cover_max_sa = sa_list[saGroundCoverMaxIndex];
            SharedAttribute* ground_cover_harvest_sa = sa_list[saGroundCoverHarvestIndex];

            residue_cover = 0.0;

            ++plant_counter;
            //VMD There are two ways to generate the plant/ground cover factor.  Can either 'hard wire' it in using field data or crop modelling data (basic level),
            //or can calculate it using the version of ROOTMAP that is coupled to the Perfect wheat model.

            //VMD The ground cover factor can vary per plant type, so need to take account of the fact that there may be a mixture of plant types in a simulation.
            //For now, won't make the ground cover spacially explicit, since this is complicated and monoculture crops are likely to be the usual modelling scenario.
            //The ground cover will just be an average of the ground covers for individual plants.  

            //VMD Basic-Level green cover factor:
            double green_cover_factor = 0.0;
            // MSA 09.09.25 Attempting conversion of these to ProcessSharedAttributes (that is, 0-dimensional / non-spatial Attributes

            const bool useDegreeDaysModel = dd_model_sa->GetValue() == 1.0;
            const double green_cover_max = ground_cover_max_sa->GetValue(); //0-1
            const double green_cover_harvest = ground_cover_harvest_sa->GetValue(); //0-1

            residue_cover = saResidueCover->GetValue();

            //Before the crop is sown, ground cover = any residues
            if (current_time < grow_start_time)
            {
                green_cover_factor = 0.0;
            }

            if (!useDegreeDaysModel) //using the time in days approach
            {
                // Retrieve time values
                // The user enters the time of full cover, ripening & harvest as days from germination.  It is then converted here
                // to seconds from the start of the simulation
                const ProcessTime_t time_of_full_cover = static_cast<const ProcessTime_t>(PROCESS_DAYS(time_full_cover_sa->GetValue()) + grow_start_time); //Days from start of simulation (time zero)
                const ProcessTime_t time_of_ripening = static_cast<const ProcessTime_t>(PROCESS_DAYS(time_ripening_sa->GetValue()) + grow_start_time); //Days from start of simulation (time zero)
                const ProcessTime_t time_of_harvest = static_cast<const ProcessTime_t>(PROCESS_DAYS(time_harvest_sa->GetValue()) + grow_start_time); //Days from start of simulation (time zero)

                //Linear increase in cover between sowing and full cover
                if ((current_time >= grow_start_time) && (current_time < time_of_full_cover))
                {
                    green_cover_factor = ((green_cover_max / (time_of_full_cover - grow_start_time)) * (current_time - grow_start_time));
                }

                //Maximum cover between full cover and ripening
                if ((current_time >= time_of_full_cover) && (current_time < time_of_ripening)) { green_cover_factor = green_cover_max; }

                //Linear decline in ground cover up to harvest
                if ((current_time >= time_of_ripening) && (current_time <= time_of_harvest))
                {
                    green_cover_factor = (((green_cover_harvest - green_cover_max) / (time_of_harvest - time_of_ripening)) * (current_time - time_of_ripening)) + green_cover_max;
                }
            }
            else //using the degree days approach (useDegreeDaysModel == true)
            {
                // Retrieve degree day values
                // The user enters the degree days of full cover, ripening & harvest as degree days from germination.
                const double degree_days_of_full_cover = dd_full_cover_sa->GetValue(); //degree days after germination that full cover occurs
                const double degree_days_of_ripening = dd_ripening_sa->GetValue(); //degree days after germination that ripening occurs
                const double degree_days_of_harvest = dd_harvest_sa->GetValue(); //degree days after germination that harvest occurs
                const double current_time_deg_days = saDegreeDays->GetValue(); // Current degree days since the start of the simulation, calculated in Nitrate::CalculateSoilTemp
                const double grow_start_degree_days = saGrowStartDegreeDays->GetValue(); //grow start is the degree days at germination and is calculated in Nitrate::CalculateSoilTemp
                const double growth_time_deg_days = current_time_deg_days - grow_start_degree_days; // Degree days since germination

                //Linear increase in cover between sowing and full cover
                if ((current_time >= grow_start_time) && (growth_time_deg_days < degree_days_of_full_cover))
                {
                    green_cover_factor = (((green_cover_max) / (degree_days_of_full_cover)) * (growth_time_deg_days));
                }

                //Maximum cover between full cover and ripening
                if ((growth_time_deg_days >= degree_days_of_full_cover) && (growth_time_deg_days < degree_days_of_ripening)) { green_cover_factor = green_cover_max; }

                //Linear decline in ground cover up to harvest
                if ((growth_time_deg_days >= degree_days_of_ripening) && (growth_time_deg_days <= degree_days_of_harvest))
                {
                    green_cover_factor = (((green_cover_harvest - green_cover_max) / (degree_days_of_harvest - degree_days_of_ripening)) * (growth_time_deg_days)) + green_cover_max;
                }
            }//else

            average_gc_factor += green_cover_factor;
        } //for ( SharedAttributeCluster

        average_gc_factor = average_gc_factor / plant_counter;
        if (average_gc_factor > 1.0)
        {
            int check_here = 1.0;
        }
        saAvgGreenCoverFactor->SetValue(average_gc_factor);
        saGreenCover->SetValue(average_gc_factor); //save the calculated crop (green) cover for calculating transpirational demand in water uptake    

        // Now calculate water uptake
        for (SharedAttributeCluster::const_iterator cluster_iter = cluster.begin();
            cluster_iter != cluster.end();
            ++cluster_iter
            )
        {
            // RvH
            // This is "use the next Plant X attribute list".
            // Note that the name of the Plant/Cluster is in (*cluster_iter).first
            const SharedAttributeList& sa_list = (*cluster_iter).second;

            SharedAttribute* water_uptake_sa = sa_list[saWaterUptakePerPlantIndex]; //water uptake from each soil volume by individual plants this timestep
            SharedAttribute* cumul_water_uptake_sa = sa_list[saCumulWaterUptakePerPlantIndex]; //water uptake from each soil volume by individual plants, cumulative since germination
            SharedAttribute* total_uptake_sa = sa_list[saTotalPlantWaterUptakeIndex]; //cumulative total water taken up by each individual plant
            SharedAttribute* water_flux_sa = sa_list[saWaterFluxPerPlantIndex];
            SharedAttribute* root_radius_sa = (saRootRadiusPerPlantIndex >= 0) ? sa_list[saRootRadiusPerPlantIndex] : saRootRadius;
            SharedAttribute* root_length_sa = sa_list[saRootLengthPerPlantIndex];
            SharedAttribute* root_length_ro_sa = sa_list[saRootLengthPerPlantPerRootOrderIndex];
            SharedAttribute* water_upregulation_limit_sa = sa_list[saWaterUpregulationLimitIndex];


            // MSA 09.09.29 TODO Convert all these magic number references to ProcessSharedAttributes    DONE
            // pan evaporation mm.day-1, converted to [cm3.cm-2.s-1], cm.s-1
            const double pan_evaporation = saPanEvaporation->GetValue() / PROCESS_DAYS(10); // [cm3.cm-2.s-1], cm.s-1
            const double crop_cover_factor = saGreenCover->GetValue();
            // MSA 10.08.20 Unused
            //const double average_temp = (saMaxTemperature->GetValue() + saMinTemperature->GetValue())/2;

            // MSA 09.09.29 Improving scoping & organisation of variables
            double root_lengths[ROOT_ORDER_COUNT] = { 0.0,0.0,0.0,0.0 }; // [0] = tap root, [1] = first root order, etc
            double root_radii[ROOT_ORDER_COUNT] = { root_radius_sa->GetValue(0, 0), root_radius_sa->GetValue(0, 1), root_radius_sa->GetValue(0, 2), root_radius_sa->GetValue(0, 3) };

            SUBSECTION_ITER_BEGIN

                // MSA 09.09.14 Changed this and all other while loops to for loops.
                // Limits the scope of each box_index variable to within its loop,
                // so it cannot be accidentally referenced and does not require explicitly setting to 0 between each loop
                for (BoxIndex box_index = 0; box_index < BOX_COUNT; ++box_index)
                {
                    for (size_t j = 0; j < ROOT_ORDER_COUNT; ++j)
                    {
                        // MSA 11.06.24 Fool! Index transpose bug here resulted in indexing the Root Order - that is, 0 to 3 -
                        // by the VolumeObject index! No wonder it was corrupting the stack, you clown!
                        root_lengths[j] += root_length_ro_sa->GetValue(box_index, j, idx);
                    }
                }

            SUBSECTION_ITER_END

                double potential_transpiration = CSMin((crop_cover_factor * pan_evaporation), (pan_evaporation - water_AverageEvaporation)); //cm.s-1
                //potential_transpiration = CSMax((potential_transpiration), (0.6*average_temp/20.0)); //cm.s-1
                //potential_transpiration = CSMin((potential_transpiration), (pan_evaporation - water_AverageEvaporation)); //cm.s-1

            if (potential_transpiration <= 0.0) { return (return_value); }

            const double total_root_surface_area = RootSurfaceArea(root_radii, root_lengths, ROOT_ORDER_COUNT);
            if (total_root_surface_area <= 1e-5) { continue; } // Root surface area too low. Go to next spatial subsection.

            //Potential transpiration based on evaporative demand
            potential_transpiration *= (soil_block_area / water_number_of_plants) * dT / total_root_surface_area; //cm3H2O/cm2root.plant

            //Total potential water supply by the root system based on local root length and soil water properties, combined with evaporative demand
            double total_potential_supply = 0.0;
            double total_potential_transpiration = 0.0;
            for (BoxIndex box_index = 0; box_index < BOX_COUNT; ++box_index)
            {
                const double box_volume = Get_BoxIndex_Volume(box_index);

                SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                    if (thisProportion == 0.0)
                    {
                        continue; // Can't uptake water from a zero volume
                    }

                const double thisVolume = box_volume * thisProportion;

                const double root_length = root_length_sa->GetValue(box_index, idx);

                if (root_length > 0.0)
                {
                    const double water_content = scoreboard->GetCharacteristicValue(Water_Content_Indices[idx], box_index);

                    for (size_t i = 0; i < ROOT_ORDER_COUNT; ++i)
                    {
                        root_lengths[i] = root_length_ro_sa->GetValue(box_index, i, idx);
                    }

                    if (!m_IS_NUTRIENT_SOLUTION) //roots growing in soil
                    {
                        const double wilting_point = scoreboard->GetCharacteristicValue(Wilting_Point_Indices[idx], box_index);
                        const double drained_upper_limit = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], box_index);

                        //Level 1 modelling 'fraction extractable' function represents the moisture release curve
                        const double coeff_a = 1 / exp(-(((20.723 * wilting_point) / (drained_upper_limit - wilting_point)) + 11.513));
                        const double coeff_b = 20.723 / (drained_upper_limit - wilting_point);
                        const double fraction_extractable = 1 / (1 + coeff_a * exp(-coeff_b * water_content));

                        total_potential_supply += (water_content - wilting_point) * thisVolume * fraction_extractable;

                        total_potential_transpiration += potential_transpiration * RootSurfaceArea(root_radii, root_lengths, ROOT_ORDER_COUNT) * fraction_extractable; //cm3H20
                    }//roots growing in soil
                    else //roots growing in nutrient solution
                    {
                        total_potential_supply += water_content * thisVolume;
                        total_potential_transpiration += potential_transpiration * RootSurfaceArea(root_radii, root_lengths, ROOT_ORDER_COUNT); //cm3H20
                    } //roots growing in nutrient solution
                }
                SUBSECTION_ITER_WITH_PROPORTIONS_END
            }//for(BoxIndex box_index=0; box_index<BOX_COUNT; ++box_index) //Total potential water supply by the root system    

            //The actual supply of water by the root system
            for (BoxIndex box_index = 0; box_index < BOX_COUNT; ++box_index)
            {
                const double box_volume = Get_BoxIndex_Volume(box_index);

                SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                    if (thisProportion == 0.0)
                    {
                        continue; // Can't uptake water from a zero volume
                    }

                const double thisVolume = box_volume * thisProportion;
                const double root_length = root_length_sa->GetValue(box_index, idx);

                if (root_length > 0.0)
                {
                    const double wilting_point = scoreboard->GetCharacteristicValue(Wilting_Point_Indices[idx], box_index);
                    const double drained_upper_limit = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], box_index);
                    double water_content = scoreboard->GetCharacteristicValue(Water_Content_Indices[idx], box_index);
                    double available_water, fraction_extractable;

                    //level 1 modelling 'fraction extractable' function represents the moisture release curve
                    if (!m_IS_NUTRIENT_SOLUTION) //roots growing in soil
                    {
                        const double coeff_a = 1 / exp(-(((20.723 * wilting_point) / (drained_upper_limit - wilting_point)) + 11.513));
                        const double coeff_b = 20.723 / (drained_upper_limit - wilting_point);
                        fraction_extractable = 1 / (1 + coeff_a * exp(-coeff_b * water_content));
                        available_water = (water_content - wilting_point) * thisVolume; //cm3H20, plant available water
                    }
                    else //roots growing in well aerated nutrient solution
                    {
                        fraction_extractable = 1;
                        available_water = 1.0;
                    }

                    for (size_t i = 0; i < ROOT_ORDER_COUNT; ++i)
                    {
                        root_lengths[i] = root_length_ro_sa->GetValue(box_index, i, idx);
                    }

                    double local_supply[ROOT_ORDER_COUNT] = { 0.0,0.0,0.0,0.0 };
                    double sum_local_supply = 0.0;

                    if (available_water > 0.0)
                    {
                        for (size_t i = 0; i < ROOT_ORDER_COUNT; ++i)
                        {
                            local_supply[i] = potential_transpiration * fraction_extractable * 2 * PI * (root_radii[i] * root_lengths[i]); //cm3/cm2 * cm2 = cm3/plant
                        }

                        //Whole plant demand for water is greater than can be supplied by the whole root system based on root surface area alone.
                        //Supply needs to be upregulated locally to try to meet the whole-plant demand.
                        if (total_potential_supply < total_potential_transpiration)
                        {
                            // MSA 09.10.01 Modified to guard against divide by 0
                            double required_upregulation = (0 == total_potential_supply) ? 0 : (total_potential_transpiration / total_potential_supply);
                            //if(0==total_potential_transpiration){required_upregulation = 0;}    // MSA 09.10.01 Redundant
                            const double upregulation_limit = water_upregulation_limit_sa->GetValue();
                            if (required_upregulation > upregulation_limit) { required_upregulation = upregulation_limit; } //required up-regulation and upregulation limit are plant-specific
                            for (size_t i = 0; i < ROOT_ORDER_COUNT; ++i)
                            {
                                local_supply[i] *= required_upregulation;
                            }
                        }
                        sum_local_supply = (local_supply[0] + local_supply[1] + local_supply[2] + local_supply[3]);
                        if (available_water < sum_local_supply) //scale the local supply back to match the amount of water locally available
                        {
                            for (size_t i = 0; i < ROOT_ORDER_COUNT; ++i)
                            {
                                local_supply[i] *= fraction_extractable * available_water / sum_local_supply; //cm3 water / plant

                                if (m_IS_NUTRIENT_SOLUTION) //roots growing in nutrient solution
                                {
                                    local_supply[i] *= available_water / sum_local_supply; //cm3 water / plant
                                }
                            }
                            sum_local_supply = (local_supply[0] + local_supply[1] + local_supply[2] + local_supply[3]);
                        }
                    } // if (available_water > 0)

                    if (!m_IS_NUTRIENT_SOLUTION &&
                        (sum_local_supply <= 0 ||
                            water_content - (sum_local_supply / thisVolume) < (0.9 * wilting_point)
                            )
                        )

                    {
                        for (size_t i = 0; i < ROOT_ORDER_COUNT; ++i)
                        {
                            local_supply[i] = 0;
                        }
                        sum_local_supply = 0;
                    }

                    if (!m_MAINTAIN_WATER_CONTENT) //allow soil water to deplete
                    {
                        water_content -= (sum_local_supply / box_volume); //cm3/cm3
                    }

                    if (!m_IS_NUTRIENT_SOLUTION) //roots growing in soil
                    {
                        scoreboard->SetCharacteristicValue(Water_Content_Indices[idx], water_content, box_index);
                    }

                    double local_water_uptake = sum_local_supply + water_uptake_sa->GetValue(box_index, idx); //cm3 uptake from each local soil volume by each plant in this timestep
                    water_uptake_sa->SetValue(local_water_uptake, box_index, idx);//cm3 uptake from each local soil volume by each plant in this timestep
                    local_water_uptake = scoreboard->GetCharacteristicValue(Cumul_Local_Water_Uptake_Indices[idx], box_index); //cm3
                    local_water_uptake += sum_local_supply; //cumulative uptake from each local soil volume by all plants
                    scoreboard->SetCharacteristicValue(Cumul_Local_Water_Uptake_Indices[idx], local_water_uptake, box_index);
                    local_water_uptake = cumul_water_uptake_sa->GetValue(box_index, idx); //cm3/plant   cumulative water uptake from the local soil volume by individual plants
                    local_water_uptake += sum_local_supply;
                    cumul_water_uptake_sa->SetValue(local_water_uptake, box_index, idx);

                    for (size_t i = 0; i < ROOT_ORDER_COUNT; ++i)
                    {
                        const double this_flux = (root_lengths[i] > 0) ? (local_supply[i] / (2 * PI * root_radii[i] * root_lengths[i] * dT)) : 0; // cm/s across the root surface
                        water_flux_sa->SetValue(this_flux, box_index, i, idx);
                    }
                    const double total_water_uptake = total_uptake_sa->GetValue(); //mls of water
                    total_uptake_sa->SetValue(total_water_uptake + sum_local_supply);
                } //if (root_length > 0)
                SUBSECTION_ITER_WITH_PROPORTIONS_END
            } //for(BoxIndex box_index=0; box_index<BOX_COUNT; ++box_index)
        } //for ( SharedAttributeCluster

        return (return_value);
    }


    long int Water::Drain(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_Time;
        Use_Scoreboard;
        Use_ScoreboardCoordinator;
        Use_PostOffice;
        Use_Boundaries;
        Use_VolumeObjectCoordinator;

        long int dT = (time - water_TimeOfPreviousLeaching);
        water_TimeOfPreviousLeaching = time;

        if (dT == 0)
        {
            return kNoError;
        }
        RmAssert(dT > 0, "Time delta since previous water draining/leaching is negative");

        //When a wetting front is part way through a box the water content above the wetting front
        //will be different to that below.  water_content is the average for the box

        // MSA 09.09.25 Commented this out to silence compiler warnings
        // double front_pos_thisbox=0.0;

        for (long y = 1; y <= NUM_Y; ++y)
        {
            for (long x = 1; x <= NUM_X; ++x)
            {
                for (CharacteristicIndices::const_iterator iter = Rain_Out_Bottom_Indices.begin(); iter != Rain_Out_Bottom_Indices.end(); ++iter)
                {
                    scoreboard->SetCharacteristicValue(*iter, 0, scoreboard->GetBoxIndex(x, y, NUM_Z)); //reset the rain out the bottom of the scoreboard.
                }
            }
        }

        // Erase old wetting fronts and reset wetting front information
        m_wettingFrontInfo.eraseMode = true;
        DrawScoreboard(DoubleRect(), __nullptr);
        m_wettingFrontInfo.boxen.clear();
        m_wettingFrontInfo.frontLocations.clear();
        m_wettingFrontInfo.eraseMode = false;

        //Working our way through the scoreboard
        //do one column at a time as each column has its own total amount of rain to be leached
        for (long y = 1; y <= NUM_Y; ++y)
        {
            for (long x = 1; x <= NUM_X; ++x)
            {
                // Is there any more rain to be drained?
                // That is, is it worth continuing down this column?
                bool continueDraining = true;
                for (long z = 1; z <= NUM_Z && continueDraining; ++z)
                {
                    // We keep draining if and only if at least one call to DoDrain results in rain draining below thisBox. 
                    continueDraining = false;
                    const BoxIndex thisBox = scoreboard->GetBoxIndex(x, y, z);
                    const BoxIndex nextBox = z == NUM_Z ? thisBox : scoreboard->GetBoxIndex(x, y, z + 1); // Important! Make sure nextBox is a valid box
                    const double box_bottom = Get_Layer_Bottom(z);
                    const double box_top = Get_Layer_Top(z);

                    // MSA Assumption: rain can transfer freely out of the bottom of the Scoreboard, regardless of any VOs. Justifiable?
                    // MSA 11.04.28 Notice that if z==NUM_Z here, thisBox==nextBox and GetTransferRestrictionData() will return unrestricted.
                    // MSA 11.07.01 No longer the case. Restricted transfer within one box is now supported.
                    const TransferRestrictionData& trd = scoreboard->GetTransferRestrictionData(thisBox, nextBox, scoreboard->GetTransferRestrictedDimension(Z, true));
                    const size_t thisBackgroundVOIndex = trd.backgroundVolumeObjectFrom == __nullptr ? 0 : trd.backgroundVolumeObjectFrom->GetIndex();
                    const size_t nextBackgroundVOIndex = trd.backgroundVolumeObjectTo == __nullptr ? 0 : trd.backgroundVolumeObjectTo->GetIndex();

                    const bool thisRestricted = trd.unrestrictedProportionFrom < 1;
                    const bool nextRestricted = trd.unrestrictedProportionTo < 1;

                    if (thisRestricted)
                    {
                        const size_t RIndex = trd.unrestrictedTransferWithinVO ? thisBackgroundVOIndex : trd.volumeObjectFrom->GetIndex();
                        if (trd.voFromPermeability > 0.0)
                        {
                            // We must first calculate the draining from the restricted section (the section inaccessible to the next box) to the unrestricted section.
                            const size_t UIndex = trd.unrestrictedTransferWithinVO ? trd.volumeObjectFrom->GetIndex() : thisBackgroundVOIndex;

                            // Attempt to drain from the restricted section to the unrestricted.
                            continueDraining = DoDrain(scoreboard, thisBox, thisBox, RIndex, UIndex,
                                scoreboard->GetCoincidentProportion(thisBox, RIndex), scoreboard->GetCoincidentProportion(thisBox, UIndex),
                                box_top, box_bottom, z == NUM_Z, trd.voFromPermeability) || continueDraining;
                        }
                        else
                        {
                            // else: no draining between restricted and unrestricted sections possible - surface is impermeable.
                            // MSA 11.07.20 Must place all the undrainable water into the water content of the box.
                            scoreboard->AddCharacteristicValue(Water_Content_Indices[RIndex], (box_bottom - box_top) * scoreboard->GetCharacteristicValue(Rained_Amount_Indices[RIndex], thisBox), thisBox);
                            scoreboard->SetCharacteristicValue(Rained_Amount_Indices[RIndex], 0.0, thisBox);
                            // Finally: we may need to draw the wetting front here, but only if the column is completely obstructed.
                            if (m_DRAW_WETTING_FRONT && (trd.volumeObjectFrom == trd.volumeObjectTo || trd.volumeObjectFrom == trd.backgroundVolumeObjectTo) && trd.unrestrictedTransferWithinVO)
                            {
                                BoxCoordinate boxcoord;
                                scoreboard->GetXYZ(thisBox, boxcoord);
                                m_wettingFrontInfo.drawable = true;
                                m_wettingFrontInfo.boxen.push_back(boxcoord);
                                m_wettingFrontInfo.frontLocations.push_back(box_top + (box_bottom - box_top) * trd.unrestrictedProportionFrom); // Approximate location, not exact
                                m_wettingFrontInfo.stratum = GetProcessStratum();
                                m_wettingFrontInfo.vDir = vTop;
                            }
                        }
                    }

                    /*
                        Any draining within the box has now been completed.
                        -------------------------------------------------------------------------------------------
                    */

                    // 1) If neither Box is restricted, we just drain from Box to Box within each spatial subsection.

                    // 2) If both Boxen are restricted, we drain from the unrestricted spatial subsection of one Box to the unrestricted spatial subsection of the other Box.

                    // 3) If only one Box is restricted, we drain from the unrestricted spatial subsection of the restricted Box to the same spatial subsection
                    //      (i.e. the one corresponding with the same VO / lack of VO) of the other Box. 
                    //      IMPORTANT NOTE: This assumes that the unrestricted subsection of a restricted Box and the entirety of an adjacent unrestricted Box
                    //    are perfectly contiguous.


                    // 1)
                    if (!thisRestricted && !nextRestricted)
                    {
                        Use_VolumeObjectCoordinator;

                        SUBSECTION_ITER_WITH_TO_AND_FROM_BOXES_AND_PROPORTIONS_BEGIN

                            // If there is no volume available to transfer out of or into, naturally we can't do any draining.
                            if (thisProportion == 0 || nextProportion == 0) continue;

                        continueDraining = DoDrain(scoreboard, thisBox, (z == NUM_Z ? InvalidBoxIndex : nextBox), idx, idx, thisProportion, nextProportion,
                            box_top, box_bottom, z == NUM_Z) || continueDraining;

                        SUBSECTION_ITER_WITH_TO_AND_FROM_BOXES_AND_PROPORTIONS_END
                    }

                    // 2)
                    else if (thisRestricted && nextRestricted)
                    {
                        const size_t unrestrictedIndex = trd.unrestrictedTransferWithinVO ? trd.volumeObjectFrom->GetIndex() : thisBackgroundVOIndex;
                        continueDraining = DoDrain(scoreboard, thisBox, (z == NUM_Z ? InvalidBoxIndex : nextBox), unrestrictedIndex, unrestrictedIndex,
                            scoreboard->GetCoincidentProportion(thisBox, unrestrictedIndex), scoreboard->GetCoincidentProportion(nextBox, unrestrictedIndex),
                            box_top, box_bottom, z == NUM_Z) || continueDraining;
                    }

                    // 3)
                    else
                    {
                        const size_t unrestrictedIndex = thisRestricted ? (trd.unrestrictedTransferWithinVO ? trd.volumeObjectFrom->GetIndex() : thisBackgroundVOIndex)
                            : (trd.unrestrictedTransferWithinVO ? trd.volumeObjectTo->GetIndex() : nextBackgroundVOIndex);
                        continueDraining = DoDrain(scoreboard, thisBox, (z == NUM_Z ? InvalidBoxIndex : nextBox), unrestrictedIndex, unrestrictedIndex,
                            scoreboard->GetCoincidentProportion(thisBox, unrestrictedIndex), scoreboard->GetCoincidentProportion(nextBox, unrestrictedIndex),
                            box_top, box_bottom, z == NUM_Z) || continueDraining;
                    }

                    //Finally, handle for the special case in which thisBox is 100% restricted and the surface separating it from nextBox is permeable.
                    if (trd.restrictedProportionFrom == 1.0 && trd.voFromPermeability > 0.0)
                    {
                        const size_t subsectionToDrainTo = (trd.unrestrictedTransferWithinVO) ? (trd.volumeObjectTo == __nullptr ? nextBackgroundVOIndex : trd.volumeObjectTo->GetIndex()) : (nextBackgroundVOIndex);

                        const double thisProportion = scoreboard->GetCoincidentProportion(thisBox, trd.volumeObjectFrom->GetIndex());
                        const double nextProportion = scoreboard->GetCoincidentProportion(nextBox, subsectionToDrainTo);
                        continueDraining = DoDrain(scoreboard, thisBox, nextBox, trd.volumeObjectFrom->GetIndex(), subsectionToDrainTo,
                            thisProportion, nextProportion,
                            box_top, box_bottom, z == NUM_Z, trd.voFromPermeability) || continueDraining;
                    }

                    // Draining complete for this box.
                } //for (z=1; z<=NUM_Z && continueDraining; ++z)    
            } //for (x=1; x<=NUM_X; ++x)
        } //for (y=1; y<=NUM_Y; ++y)


        if (water_PhosphorusModule != __nullptr)
        {
            Send_GeneralPurpose(time, water_PhosphorusModule, __nullptr, kWaterRedistributed);

            Send_GeneralPurpose(time, water_PhosphorusModule, __nullptr, kWaterDrained);

            //Send_External_WakeUp(time, water_PhosphorusModule, __nullptr);  //Because there has been a rain event
            //                                                       //phosphate needs to be leached.
            //Send_External_WakeUp(time, water_PhosphorusModule, 3);  //Because there has been a rain event
            //                                                       //phosphate needs to diffuse.
        }

        // 98.10.14 RvH.  New macro parameters
        if (water_NitrateModule != __nullptr)
        {
            // Because there has been a rain event nitrate needs to be leached.
            //Send_GeneralPurpose(time, water_NitrateModule, __nullptr, kWaterRedistributed);  
            Send_GeneralPurpose(time, water_NitrateModule, __nullptr, kWaterDrained);

            //Send_External_WakeUp(time, water_NitrateModule, __nullptr);  
        }

        // Draw the new wetting fronts.
        DrawScoreboard(DoubleRect(), __nullptr); // Both the area and Scoreboard pointers are unused

        return (return_value);
    } // Drain

    /* **************************************************************************************** */

    // MSA 11.01.27 This method does the actual draining between two spatial-subsections of adjacent boxes.
    // It does not check for transfer restriction; callers of this method should first ensure that the requested draining is legal.
    // The parameter volumeObjectSurfacePermeability is only used in the case of draining between different spatial subsections of the same Box.
    //
    // MSA 11.04.27 Another important note: If isBottomLayer, then toBox may be one of two values:
    // 1) fromBox. In this case we are draining within the box at the bottom of this particular column.
    // 2) InvalidBoxIndex.
    bool Water::DoDrain(Scoreboard* scoreboard, const BoxIndex& fromBox, const BoxIndex& toBox, const size_t& fromVOIndex, const size_t& toVOIndex, const double& fromProportion, const double& toProportion, const double& boxTopFrom, const double& boxBottomFrom, const bool& isBottomLayer, const double& volumeObjectSurfacePermeability /* = 0 */)
    {
        // MSA 11.04.28 Moved this conditional to first, as it will likely be evaluating to true the most frequently.
        if (fromProportion <= 0.0 || toProportion <= 0.0) return false;

        if (isBottomLayer)
        {
            if (fromBox != toBox && toBox != InvalidBoxIndex) return false; // Invalid value given for toBox.
        }
        else if (toBox == InvalidBoxIndex) return false; // toBox may only be InvalidBoxIndex if we are on the bottom layer.

        if (fromBox == InvalidBoxIndex) return false; // fromBox==InvalidBoxIndex is never allowed!

        if (fromBox == toBox && (!isBottomLayer) && (fromVOIndex == toVOIndex || volumeObjectSurfacePermeability <= 0.0)) return false;

        const double boxHeightFrom = boxBottomFrom - boxTopFrom;

        const double deltaWRestrictionModifier = fromVOIndex != toVOIndex ? volumeObjectSurfacePermeability : 1.0;

        if (deltaWRestrictionModifier <= 0) return false; // Shortcut exit (because no draining can occur through an impermeable surface)

        double waterContentFrom = scoreboard->GetCharacteristicValue(Water_Content_Indices[fromVOIndex], fromBox);
        // MSA 11.03.25 deltaWRestrictionModifier limits the amount of water that can flow through the VO surface.
        const double rainedAmountFrom = deltaWRestrictionModifier * scoreboard->GetCharacteristicValue(Rained_Amount_Indices[fromVOIndex], fromBox);

        double wettingFrontFrom = scoreboard->GetCharacteristicValue(Wetting_Front_Indices[fromVOIndex], fromBox); //position of the wetting front
        const double drainedUpperLimitFrom = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[fromVOIndex], fromBox);

        const double previousWaterContentFrom = waterContentFrom;
        const double waterDeficitFrom = (drainedUpperLimitFrom - waterContentFrom) * boxHeightFrom;

        scoreboard->SetCharacteristicValue(Wetting_Front_Indices[fromVOIndex], 0, fromBox); //reset the previous wetting front ready for the new one

        if (waterContentFrom < drainedUpperLimitFrom) // Then add some water to this box.
        {
            if (rainedAmountFrom > waterDeficitFrom)
                // Will fill this box and some of the next, so the wetting front will be below this box.
            {
                waterContentFrom = drainedUpperLimitFrom;
                scoreboard->SetCharacteristicValue(Water_Content_Indices[fromVOIndex], waterContentFrom, fromBox);

                //Must set the wetting front to zero wherever there isn't a wetting front otherwise things won't work properly
                scoreboard->SetCharacteristicValue(Wetting_Front_Indices[fromVOIndex], 0.0, fromBox);

                const double remainingRainedAmount = rainedAmountFrom - waterDeficitFrom;

                if (isBottomLayer) //we have so much rain that the wetting front is below the bottom of the scoreboard.
                {
                    //assume that boxes below the bottom box have the same characteristics as the bottom box
                    //and the same water content as the bottom box had (before it was filled to DUL).
                    scoreboard->SetCharacteristicValue(Rain_Out_Bottom_Indices[fromVOIndex], remainingRainedAmount, fromBox);
                    wettingFrontFrom = (remainingRainedAmount / (drainedUpperLimitFrom - previousWaterContentFrom)) + boxBottomFrom;
                    scoreboard->SetCharacteristicValue(Wetting_Front_Indices[fromVOIndex], wettingFrontFrom, fromBox);
                }
                else
                {
                    // MSA 11.04.28 Note to self: toBox is guaranteed !=InvalidBoxIndex here
                    scoreboard->SetCharacteristicValue(Rained_Amount_Indices[toVOIndex], remainingRainedAmount, toBox); //this is the amount of rain moving into the next box down
                }

                // Draining complete for this box; there is still rained water available.
                return true;
            } // if (rainedAmountFrom > waterDeficitFrom)


            if (rainedAmountFrom == waterDeficitFrom)
                //Will fill this box exactly and the wetting front will be at the bottom of this box.
            {
                waterContentFrom = drainedUpperLimitFrom;
                scoreboard->SetCharacteristicValue(Water_Content_Indices[fromVOIndex], waterContentFrom, fromBox);

                //set the wetting front to be at the bottom of this box                        
                wettingFrontFrom = boxBottomFrom;
                scoreboard->SetCharacteristicValue(Wetting_Front_Indices[fromVOIndex], wettingFrontFrom, fromBox);

                //VMD 02/03/2009 visualising wetting fronts - prepare the new wetting front (DrawScoreboard is called at the end if this function).
                if (m_DRAW_WETTING_FRONT && (wettingFrontFrom != 0))
                {
                    BoxCoordinate boxcoord;
                    scoreboard->GetXYZ(fromBox, boxcoord);
                    m_wettingFrontInfo.drawable = true;
                    m_wettingFrontInfo.boxen.push_back(boxcoord);
                    m_wettingFrontInfo.frontLocations.push_back(wettingFrontFrom);
                    m_wettingFrontInfo.stratum = GetProcessStratum();
                    m_wettingFrontInfo.vDir = vTop;
                }

                // There will be no rain transferred to the next box as the wetting front is at the bottom of this box.
                // It is important that all boxes that have not had rain transferred to them have their rained amounts
                // reset to zero, so that the displacement of nitrate packets is calculated properly.

                if (!isBottomLayer)
                {
                    // MSA 11.04.28 Note to self: toBox is guaranteed !=InvalidBoxIndex here
                    // No rained amount makes it down to toBox
                    scoreboard->SetCharacteristicValue(Rained_Amount_Indices[toVOIndex], 0.0, toBox);
                }

                // Draining complete for this box; there is no rained water left to drain.
                return false;
            } //if(rainedAmountFrom == waterDeficitFrom)


            if ((rainedAmountFrom > 0.00000001) && (rainedAmountFrom < waterDeficitFrom))
                //Will not completely fill this box and the wetting front will be part way through this box.
            {
                waterContentFrom = previousWaterContentFrom + (rainedAmountFrom / boxHeightFrom);
                scoreboard->SetCharacteristicValue(Water_Content_Indices[fromVOIndex], waterContentFrom, fromBox);
                wettingFrontFrom = ((rainedAmountFrom / waterDeficitFrom) * boxHeightFrom) + boxTopFrom;
                scoreboard->SetCharacteristicValue(Wetting_Front_Indices[fromVOIndex], wettingFrontFrom, fromBox);

                //VMD 02/03/2009 visualising wetting fronts - prepare the new wetting front (DrawScoreboard is called at the end if this function).
                if ((wettingFrontFrom != 0) && (m_DRAW_WETTING_FRONT))
                {
                    BoxCoordinate boxcoord;
                    scoreboard->GetXYZ(fromBox, boxcoord);
                    m_wettingFrontInfo.drawable = true;
                    m_wettingFrontInfo.boxen.push_back(boxcoord);
                    m_wettingFrontInfo.frontLocations.push_back(wettingFrontFrom);
                    m_wettingFrontInfo.stratum = GetProcessStratum();
                    m_wettingFrontInfo.vDir = vTop;
                }

                if (!isBottomLayer)
                {
                    // MSA 11.04.28 Note to self: toBox is guaranteed !=InvalidBoxIndex here
                    // No rained amount makes it down to toBox
                    scoreboard->SetCharacteristicValue(Rained_Amount_Indices[toVOIndex], 0.0, toBox);
                }

                // Draining complete for this box; there is no rained water left to drain.
                return false;
            } //if ((rainedAmountFrom != 0.0) && (rainedAmountFrom < waterDeficitFrom))

            if (rainedAmountFrom == 0.0)
            {
                if (!isBottomLayer)
                {
                    // MSA 11.04.28 Note to self: toBox is guaranteed !=InvalidBoxIndex here                        
                    scoreboard->SetCharacteristicValue(Rained_Amount_Indices[toVOIndex], rainedAmountFrom, toBox);
                    scoreboard->SetCharacteristicValue(Wetting_Front_Indices[fromVOIndex], 0, fromBox);
                } //if (!isBottomLayer)

                // Draining complete for this box; there is no rained water left to drain.
                return false;
            } //if (rainedAmountFrom == 0)
        } //if (waterContentFrom < drainedUpperLimitFrom)


        // Now check the case where fromBox has sufficent water content that it won't absorb any of this rainfall
        if (waterContentFrom >= drainedUpperLimitFrom)
        {
            if (isBottomLayer)
            {
                scoreboard->SetCharacteristicValue(Rain_Out_Bottom_Indices[fromVOIndex], rainedAmountFrom, fromBox);
                wettingFrontFrom = rainedAmountFrom + boxBottomFrom;
                scoreboard->SetCharacteristicValue(Wetting_Front_Indices[fromVOIndex], wettingFrontFrom, fromBox);
                //rainedAmountFrom = 0.0;  //for the moment the extra rain magically disappears
            }
            else
            {
                // MSA 11.04.28 Note to self: toBox is guaranteed !=InvalidBoxIndex here
                scoreboard->SetCharacteristicValue(Rained_Amount_Indices[toVOIndex], rainedAmountFrom, toBox);
                scoreboard->SetCharacteristicValue(Wetting_Front_Indices[fromVOIndex], 0.0, fromBox);
            }

            // Draining complete for this box; there is still rained water available.
            return true;
        } //if (!already_leached && (waterContentFrom >= drainedUpperLimitFrom))


        // Draining complete for this box; there is no rained water left to drain.
        return false;
    }

    /* ****************************************************************** */


    void Water::DrawScoreboard(const DoubleRect& /* area */, Scoreboard* /* scoreboard */)
    {
        // MSA 09.10.20 For now, only the wetting front is drawn.
        if (m_wettingFrontInfo.drawable)
        {
            BEGIN_DRAWING
                std::vector<BoxCoordinate>::iterator iter = m_wettingFrontInfo.boxen.begin();
            std::vector<double>::iterator iter2 = m_wettingFrontInfo.frontLocations.begin();
            while (iter != m_wettingFrontInfo.boxen.end() && iter2 != m_wettingFrontInfo.frontLocations.end())
            {
                if (m_wettingFrontInfo.eraseMode)
                {
                    drawing.RemoveRectangle(m_wettingFrontInfo.stratum, &(*iter++), m_wettingFrontInfo.vDir, *iter2++);
                }
                else
                {
                    drawing.DrawRectangle(m_wettingFrontInfo.stratum, &(*iter++), m_wettingFrontInfo.vDir, *iter2++, m_wettingFrontInfo.colour);
                }
            }
            END_DRAWING
                m_wettingFrontInfo.eraseMode = false;
        }
    }
} /* namespace rootmap */
