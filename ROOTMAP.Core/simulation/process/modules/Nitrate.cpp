/////////////////////////////////////////////////////////////////////////////
// Name:        Nitrate.cpp
// Purpose:     Implementation of the Nitrate class
// Created:     DD/04/1998
// Author:      Vanessa Dunbabin, Art Diggle
// $Date: August 2010, VMD updating code $
// $Revision: 49, 17/08/2016 VMD $
// Copyright:   ©2006 University of Tasmania, Dr Vanessa Dunbabin, Centre for Legumes in Mediterranean Agriculture, Grains Research and Development Corporation, Department of Agriculture and Food Western Australia
//
// Nitrate contains the nitrate processes including the nitrate leaching routine.  Nitrate leaching keeps track of little packets of
// nitrate each one being x ugN.  These little nitrates are summed to give an average nitrate
// concentration for the box.  Nitrate leaching uses the water movement process to determine
// the nitrate movement.  Each nitrate will move according to a distribution of which the mean
// corresponds to the amount of water that has gone passed the nitrate and the variance is dependent
// on both the mean and the displacement dependent dispersivity.
//
//////////////////////////////////////////////////////////////////////////////

//Heritage
#include "simulation/process/modules/Nitrate.h"
#include "simulation/process/common/ProcessUtility.h"
#include "simulation/process/common/Process.h"
#include "simulation/process/common/ProcessDrawing.h"
#include "simulation/process/modules/WaterNotification.h"

#include "simulation/data_access/interface/ProcessDAI.h"
#include "simulation/data_access/tinyxmlconfig/TXCharacteristicDA.h"
#include "simulation/data_access/tinyxmlconfig/TXProcessDA.h"

#include "simulation/process/plant/PlantSummaryRegistration.h"
#include "simulation/process/plant/PlantSummaryHelper.h"
#include "simulation/process/shared_attributes/ProcessSharedAttributeOwner.h"
#include "simulation/process/shared_attributes/ProcessAttributeSupplier.h"
#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"
#include "simulation/process/shared_attributes/SharedAttributeManager.h"
#include "simulation/process/shared_attributes/SharedAttributeSearchHelper.h"
#include "simulation/process/shared_attributes/SharedAttribute.h"
#include "simulation/process/modules/VolumeObject.h"

#include "core/common/RmAssert.h"
#include "core/common/ExtraStuff.h"
#include "core/scoreboard/BoundaryArray.h"
#include "core/utility/Utility.h"

#include <cmath>
#include <algorithm>
#include <time.h>


#define LOG_ERROR_IF(s) if ( (s) ) { RootMapLogError(LOG_LINE "Encountered error in logic : " #s); }


// The frequency distribution will not be calculated past 5% of the frequency at the mean.
const double FREQ_CUTOFF = 0.05;

namespace rootmap
{
    using Utility::Random1;
    RootMapLoggerDefinition(Nitrate);

    IMPLEMENT_DYNAMIC_CLASS(Nitrate, Process)

        Nitrate::Nitrate()
        : mySharedAttributeOwner(__nullptr)
        , m_prev_soln_change(0)
        , m_time_of_previous_diffusion(-1)
        , m_time_prev_waking(-1)
        , m_time_prev_temp(-1)
        , m_time_of_prev_solution_change(-1)
        , m_time_of_prev_nutr_rnw(-1)
        , m_time_between_wakings(PROCESS_HOURS(4))
        , m_total_upper_uptake(0)
        , m_total_lower_uptake(0)
        , m_cumulativeMineralisation(0)
        , m_dotColour(wxColour(200, 111, 91))
        , m_volumeObjectCoordinator(__nullptr)
        , m_boundarySet(__nullptr)
        , m_NITRATE_DRAW_PROPORTION(0.5)
        , m_DIFFUSION_COEFF_SOLN(1.9e-5) //cm2/sec nitrate diffusion coeff in solution
        , m_IS_NUTRIENT_SOLUTION(false)
        , m_CALCULATE_NITRATE_MINERALISATION(false)
        , m_NITRATE_PACKET_SIZE(3.875)
    {
        RootMapLoggerInitialisation("rootmap.Nitrate");

        mySharedAttributeOwner = new ProcessSharedAttributeOwner("Nitrate", this);
        SharedAttributeRegistrar::RegisterOwner(mySharedAttributeOwner);

        ProcessAttributeSupplier* supplier = new ProcessAttributeSupplier(this, "Nitrate");
        SharedAttributeRegistrar::RegisterSupplier(supplier);
    }

    Nitrate::~Nitrate()
    {
        // NO. owners are deleted as part of Engine destruction
        //    delete mySharedAttributeOwner;
        // MSA 09.10.27 ProcessAttributeSupplier * supplier is also, presumably, deleted in SharedAttributeRegistrar::DeregisterAll() as part of Engine destruction

        // MSA 09.10.30 All Nitrate packet storage is now handled by a std::list, which does its own cleanup
        delete m_boundarySet;
    }

    void Nitrate::SetVolumeObjectCoordinator(const VolumeObjectCoordinator& voc)
    {
        m_volumeObjectCoordinator = &voc;
    }

    void Nitrate::Initialise(const ProcessDAI& data)
    {
        if (m_volumeObjectCoordinator != __nullptr)
        {
            // MSA 10.12.22 List the Characteristics NOT affected by barrier modelling here; exclude from the makeVariants process.
            std::vector<std::string> invariantCharacteristicNames;
            invariantCharacteristicNames.push_back("Estimate Initial Soil Temp Profile");
            invariantCharacteristicNames.push_back("Radiation Extinction Coeff");
            ProcessDAI* ptr = m_volumeObjectCoordinator->AccomodateVolumeObjects(data, invariantCharacteristicNames);
            Process::Initialise(*ptr);
            delete ptr;
        }
        else
        {
            Process::Initialise(data);
        }
    }

    long int Nitrate::GetPeriodicDelay(long int /*characteristic_number*/) { return (m_time_between_wakings); }

    /* ***************************************************************************
    */
    long int Nitrate::Register(ProcessActionDescriptor* action)
    {
        Use_ProcessCoordinator;
        PlantSummaryHelper helper(processcoordinator, 0);

        // MSA TODO 10.09.28 Why is this here, and not in PlantCoordinator?

        // Per-Plant Per-Box
        PlantSummaryRegistration* psr = helper.MakePlantSummaryRegistration(action);
        psr->RegisterByPerBox(true);
        psr->RegisterAllPlantCombos();
        psr->RegisterAllBranchOrders();
        psr->RegisterAllVolumeObjectCombos();
        psr->RegisterSummaryType(srRootLength);
        psr->RegisterModule((Process*)this);
        helper.SendPlantSummaryRegistration(psr, action, this);

        // Per-Plant Per-RootOrder Per-Box (ie. scoreboard)
        PlantSummaryRegistration* psr2 = helper.MakePlantSummaryRegistration(action);
        psr2->RegisterByPerBox(true);
        psr2->RegisterAllPlantCombos();
        psr2->RegisterAllBranchOrderCombos();
        psr->RegisterAllVolumeObjectCombos();
        psr2->RegisterSummaryType(srRootLength);
        psr2->RegisterModule((Process*)this);
        helper.SendPlantSummaryRegistration(psr2, action, this);

        return kNoError;
    }

    /* ***************************************************************************
    */
    long int Nitrate::Initialise(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_Scoreboard;
        Use_ProcessCoordinator;
        Use_VolumeObjectCoordinator;

        Use_ScoreboardCoordinator;
        Use_Boundaries;
        m_boundarySet = new BoundarySet(X_boundaries, Y_boundaries, Z_boundaries);

        // Determine Nitrate display packet size.
        //SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Packet_Size_Indices, "Packet Size");
        // MSA 11.02.11 For now, just draw 'em all the same.
        //m_NITRATE_PACKET_SIZE = Get_Concentration_Of_BoxIndex(Packet_Size_Indices[0], 0);

        //m_dotColour = wxColour(200,111,91); // MSA 09.10.30 Moved to constructor initialisation list

        // Determine the characteristic indices of the water process module's characteristics
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Water_Content_Indices, "Water Content");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Rained_Amount_Indices, "Rained Amount");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Drained_Upper_Limit_Indices, "Drained Upper Limit");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Wilting_Point_Indices, "Wilting Point");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Dispersivity_Indices, "Dispersivity");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Water_Moved_Indices, "Water Moved");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Direction_Moved_Indices, "Direction Moved");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Rain_Out_Indices, "Rain Out Bottom");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Wetting_Front_Indices, "Wetting Front");

        // Determine the characteristic indices from other processes
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Bulk_Density_Indices, "Bulk Density");

        // set up a local characteristic index cache
        // Search by name, so that finding them is independent of the characteristics being ordered correctly in the resource
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Nitrate_Concentration_Indices, "Nitrate Concentration");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Nitrate_Amount_Indices, "Nitrate Amount");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Nitrate_Renew_Indices, "Nitrate Renew");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Nitrate_AmountCheck_Indices, "Nitrate Amount Check");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Dispersivity_Indices, "Dispersivity");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Nitrate_Leached_Indices, "Nitrate Leached");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, FertFracN_Indices, "Fraction of Fertiliser N");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Coeff_C_Indices, "Coefficient c");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Coeff_D_Indices, "Coefficient d");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, FertiliserN_Indices, "Fertiliser N");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, BackgroundN_Indices, "Background N");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Added_N_Indices, "Added N");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Added_N2_Indices, "Added N2");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Rate_Constant_Indices, "Rate Constant");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Rate_Constant_Temperature_Indices, "Rate Constant Temperature");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Thermal_Diffusivity_Indices, "Thermal Diffusivity");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Q10_Indices, "Q10");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Mineralisable_N_Indices, "Mineralisable N");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Mineralised_N_Indices, "Mineralised N");

        Estimate_Soil_Temp_Index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Estimate Initial Soil Temp Profile");
        Radiation_Extinction_Coeff_Index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Radiation Extinction Coeff");
        Temperature_Index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Temperature");

        // 98.10.14 RvH
        // Determine the Water and PlantCoordinator module id's
        Process* theProcessModule = Find_Process_By_Process_Name("Water");
        itsWaterModuleID = theProcessModule->GetProcessID();

        nitrate_PlantCoordinatorModule = Find_Process_By_Process_Name("PlantCoordinator");

        // MSA 09.11.09 Determine the Scoreboard dimensions here, save doing it repeatedly
        Get_Number_Of_Layers(NUM_X, NUM_Y, NUM_Z); // number of layers in each dimension
        BOX_COUNT = scoreboard->GetNumberOfBoxes(); // How many boxes do we have?

        SetPeriodicDelay(kNormalMessage, m_time_between_wakings);
        Use_PostOffice;
        Send_GeneralPurpose(action->GetTime() + m_time_between_wakings, this, __nullptr, kNormalMessage);

        PrepareSharedAttributes(action);

        return_value = InitialisePackets(action);

        Process::Initialise(action);

        return (return_value);
    } // long int Nitrate::Initialise(ProcessActionDescriptor *action)

    void Nitrate::PrepareSharedAttributes(ProcessActionDescriptor* action)
    {
        Use_SharedAttributeManager;

        SharedAttributeSearchHelper searchHelper(sharedattributemanager, mySharedAttributeOwner);

        //SharedAttribute * nitrateUptakeRO    = helper.SearchForAttribute("RO Nitrate Uptake", "RootOrder");
        //SharedAttribute * nitrateUptakeTotal = helper.SearchForAttribute("Total Nitrate Uptake", "Plant");

        std::vector<std::string> variation_names;
        variation_names.push_back("Plant");
        variation_names.push_back("RootOrder");

        // Load expected SharedAttributes
        saNitrateUptakePerPlantIndex = searchHelper.SearchForClusterIndex("Plant Nitrate Uptake", "Plant", __nullptr); //Total nitrate taken up by each plant at each growth time step and each soil volume (spatial), used in PlantDynamicResourceAllocationToRoots
        saCumulNitrateUptakePerPlantIndex = searchHelper.SearchForClusterIndex("Cumul Plant Nitrate Uptake", "Plant", __nullptr); //Cumulative total nitrate taken up by each plant (non-spatial)
        saCumulFertNUptakePerPlantIndex = searchHelper.SearchForClusterIndex("Cumul Plant FertN Uptake", "Plant", __nullptr); //Cumulative total nitrate taken up by each plant (non-spatial)
        saNitrateUptakePerPlantPerRootOrderIndex = searchHelper.SearchForClusterIndex("Cumul RO Nitrate Uptake", "Plant", variation_names); //Cumulative total nitrate taken up by each root order of each plant (non-spatial)
        saLocalNitrateUptakePerPlantIndex = searchHelper.SearchForClusterIndex("Cumul Local Nitrate Uptake", "Plant", __nullptr); //Cumulative total nitrate taken up from each soil volume by each plant (spatial)
        saLocalFertNUptakePerPlantIndex = searchHelper.SearchForClusterIndex("Cumul Local FertN Uptake", "Plant", __nullptr); //Cumulative total fertilier N taken up from each soil volume by each plant (spatial)

        saGerminationLagIndex = searchHelper.SearchForClusterIndex("Germination Lag", "Plant", __nullptr);

        saSeedingTimeIndex = searchHelper.SearchForClusterIndex("Seeding Time", "Plant", __nullptr);
        saAddFertNIndex = searchHelper.SearchForClusterIndex("Added Fertiliser N", "Plant", __nullptr);
        saAbsorptionPowerPerPlantIndex = searchHelper.SearchForClusterIndex("N Absorption Power", "Plant", __nullptr);
        saBaseTempIndex = searchHelper.SearchForClusterIndex("Degree Days Base Temp", "Plant", __nullptr);

        saWaterFluxPerPlantPerRootOrderIndex = searchHelper.SearchForClusterIndex("Water Flux", "Plant", variation_names);

        // search for root length, per plant.
        saRootLengthPerPlantIndex = searchHelper.SearchForClusterIndex("Root Length Wrap None", "Plant", __nullptr);

        // search for root length, per Plant+RootOrder
        saRootLengthPerPlantPerRootOrderIndex = searchHelper.SearchForClusterIndex("Root Length Wrap None", "Plant", variation_names);

        saRootRadiusPerPlantPerRootOrderIndex = searchHelper.SearchForClusterIndex("Root Radius", "Plant", variation_names);


        // Search for Root Length, per-Plant, per-VolumeObject, per-RootOrder
        variation_names.push_back("VolumeObject");
        saRootLengthPerPlantPerRootOrderBoogalooIndex = searchHelper.SearchForClusterIndex("Root Length Wrap None", "Plant", variation_names);

        variation_names.clear();
        variation_names.push_back("Plant");
        variation_names.push_back("RootOrder");
        variation_names.push_back("VolumeObject");
        // Search for Root Length, per-Plant, per-VolumeObject
        saRootLengthPerPlantBoogalooIndex = searchHelper.SearchForClusterIndex("Root Length Wrap None", "Plant", variation_names);

        //int anothertest = searchHelper.SearchForClusterIndex("Root Length Wrap None","Plant,VolumeObject",variation_names);


        saMaxTemperature = searchHelper.SearchForAttribute("Max Temperature");
        saMinTemperature = searchHelper.SearchForAttribute("Min Temperature");
        saGreenCover = searchHelper.SearchForAttribute("Green Cover");
        saDegreeDays = searchHelper.SearchForAttribute("Degree Days");
        saGrowStartDegreeDays = searchHelper.SearchForAttribute("Degree Days at Growth Start");
        saMineralisationDepth = searchHelper.SearchForAttribute("Mineralisation Depth");
        saCalcNitrateMineralisation = searchHelper.SearchForAttribute("Calculate Nitrate Mineralisation");

        saNutrientSolutionIndex = searchHelper.SearchForClusterIndex("Nutrient Solution", "Plant", __nullptr);
        saNutrientSolnRenewIndex = searchHelper.SearchForClusterIndex("Nutrient Solution Renew", "Plant", __nullptr);
        saNutrientRenewIndex = searchHelper.SearchForClusterIndex("Nutrient Renew", "Plant", __nullptr);

        // MSA 10.11.10 Determine if it's a nutrient solution experiment
        const SharedAttributeCluster& cluster = Get_Cluster("Plant");
        SharedAttributeCluster::const_iterator cluster_iter = cluster.begin();

        const SharedAttributeList& sa_list = (*cluster_iter).second;
        SharedAttribute* nutrient_solution_sa = sa_list[saNutrientSolutionIndex];
        m_IS_NUTRIENT_SOLUTION = nutrient_solution_sa->GetValue() == 1; //0 = soil experiment, 1 = nutrient solution experiment

        m_CALCULATE_NITRATE_MINERALISATION = saCalcNitrateMineralisation->GetValue() == 1; //0 = don't calculate nitrate mineralisation, 1 = do calculate

        // Future conversion to SA?
        //saSeedingTime = searchHelper.SearchForAttribute("Seeding Time");
        //saAddFertN = searchHelper.SearchForAttribute("Added Fertiliser N");

        // if any SharedAttributes were not found, log an error and
    }


    /* ************************************************************************** */

    long int Nitrate::StartUp(ProcessActionDescriptor* action)
    {
        Use_Time;
        Use_Scoreboard;

        // if we've never diffused before, make this the initial time - to avoid
        // having a humungous dT in DiffuseN
        if (m_time_of_previous_diffusion < 0)
        {
            m_time_of_previous_diffusion = time;
        }
        // ditto for other dT1
        if (m_time_prev_waking < 0)
        {
            m_time_prev_waking = time;
        }
        if (m_time_prev_temp < 0)
        {
            m_time_prev_temp = time;
        }
        if (m_time_of_prev_solution_change < 0)
        {
            m_time_of_prev_solution_change = time;
        }
        if (m_time_of_prev_nutr_rnw < 0)
        {
            m_time_of_prev_nutr_rnw = time;
        }

        if (!m_IS_NUTRIENT_SOLUTION) //roots are growing in soil
        {
            const bool estimate_soil_temp = Get_Concentration_Of_BoxIndex(Estimate_Soil_Temp_Index, 0) == 1.0;

            if (estimate_soil_temp) //the user has not entered an initial soil temp profile, it needs to be estimated from air temp
            {
                CalculateSoilTemp(action, true); // true --> initialising the soil temperature profile
            }
        }


        return (Process::StartUp(action));
    }

    /* ************************************************************************** */

    long int Nitrate::InitialisePackets(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_Scoreboard;
        Use_VolumeObjectCoordinator;

        // MSA 11.04.28 Replaces Use_Boundaries;
        const BoundaryArray& X_boundaries = m_boundarySet->X_boundaries;
        const BoundaryArray& Y_boundaries = m_boundarySet->Y_boundaries;
        const BoundaryArray& Z_boundaries = m_boundarySet->Z_boundaries;

        // MSA 10.11.08 NOTE:
        // When initialising packet CONCENTRATIONS, we assume that everything is uniform
        // (i.e. we don't take VolumeObjects into account yet).
        // Of course, for AMOUNTS, we have to take VOs into account.

        for (BoxIndex box_index = 0; box_index < BOX_COUNT; ++box_index)
        {
            // Calculating the number of nitrate packets in each box based upon the amount of nitrate µgN in the box
            // MSA 09.11.13 Nitrates stored in a std::list now; 

            const double box_volume = Get_BoxIndex_Volume(box_index); // Total box volume in mL

            SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                double nitrate_amount = scoreboard->GetCharacteristicValue(Nitrate_Amount_Indices[idx], box_index); //µg/g, nitrate is initially input as units of ug/g

            if (!m_IS_NUTRIENT_SOLUTION) // If a soil simulation (not a solution simulation), initialise the fertiliser fraction to 0 and calculate nitrate amount using bulk density
            {
                scoreboard->SetCharacteristicValue(FertFracN_Indices[idx], 0.0, box_index);
                const double bulkDensity = scoreboard->GetCharacteristicValue(Bulk_Density_Indices[idx], box_index);
                nitrate_amount *= (box_volume * bulkDensity * thisProportion); //µgN
                const double water_content = scoreboard->GetCharacteristicValue(Water_Content_Indices[idx], box_index);
                if (thisProportion <= 0.0 || water_content <= 0.0)
                {
                    scoreboard->SetCharacteristicValue(Nitrate_Concentration_Indices[idx], 0.0, box_index); //µg/mL = µg/cm3
                }
                else
                {
                    scoreboard->SetCharacteristicValue(Nitrate_Concentration_Indices[idx], nitrate_amount / (thisProportion * box_volume * water_content), box_index); //µg/mL = µg/cm3
                }
            }
            else
            {
                // Set concentration using amount
                if (thisProportion > 0)
                {
                    scoreboard->SetCharacteristicValue(Nitrate_Concentration_Indices[idx], nitrate_amount, box_index); //µg/mL = µg/cm3
                }
                else
                {
                    scoreboard->SetCharacteristicValue(Nitrate_Concentration_Indices[idx], 0.0, box_index); //µg/mL = µg/cm3
                }
                // Now adjust amount
                nitrate_amount *= (box_volume * thisProportion); //µgN
            }

            scoreboard->SetCharacteristicValue(Nitrate_Amount_Indices[idx], nitrate_amount, box_index); //µgN

            SUBSECTION_ITER_WITH_PROPORTIONS_END
        } // for(BoxIndex box_index=0; box_index<BOX_MAX; ++box_index)


        // Iterate through the scoreboard allocating nitrate packets to random positions within each box.
        for (long y = 1; y <= NUM_Y; ++y)
        {
            const double box_front = Get_Layer_Front(y);
            const double box_depth = Get_Layer_Depth(y);

            for (long x = 1; x <= NUM_X; ++x)
            {
                const double box_left = Get_Layer_Left(x);
                const double box_width = Get_Layer_Width(x);

                for (long z = 1; z <= NUM_Z; ++z)
                {
                    const BoxIndex box_index = scoreboard->GetBoxIndex(x, y, z);
                    const double box_top = Get_Layer_Top(z);
                    const double box_height = Get_Layer_Height(z);

                    // MSA 10.11.11 Note that we start with uniform distribution of Nitrate within each Scoreboard Box,
                    // so we sum the total nitrate amount across restricted and unrestricted sections here
                    // and place packets pseudorandomly, ignoring VolumeObject barriers while initialising.

                    double nitrate_amount = Get_Concentration_Of_BoxIndex(Nitrate_Amount_Indices[0], box_index);
                    for (VolumeObjectList::const_iterator voliter = volumeobjectcoordinator->GetVolumeObjectList().begin();
                        voliter != volumeobjectcoordinator->GetVolumeObjectList().end();
                        ++voliter)
                    {
                        const size_t idx = (*voliter)->GetIndex();
                        if (scoreboard->GetCoincidentProportion(box_index, idx) > 0.0)
                        {
                            nitrate_amount += scoreboard->GetCharacteristicValue(Nitrate_Amount_Indices[idx], box_index);
                        }
                    }
                    const unsigned long number_nitrate_packets = static_cast<const unsigned long>(nitrate_amount / m_NITRATE_PACKET_SIZE);
                    const double left_over_amount = nitrate_amount - ((number_nitrate_packets * m_NITRATE_PACKET_SIZE));

                    /* Make the list of random coordinates for allocating to nitrate packets.
                    To generate the random numbers, Random1() from Utilities.h is used */
                    for (unsigned long i = 0; i <= number_nitrate_packets; ++i) // Note 1 extra point for the packet that represents the left over concentration
                    {
                        DoubleCoordinateNitrate dcn(Random1() < m_NITRATE_DRAW_PROPORTION);
                        if (i < number_nitrate_packets)
                        {
                            dcn.s = m_NITRATE_PACKET_SIZE; // This is just another full-size packet
                        }
                        else if (left_over_amount > 0.0)
                        {
                            dcn.s = left_over_amount; // Set partial packet size
                        }
                        else { continue; } // No partial packet to add. Skip out.

                        // dcn.f = false; //false for background N and true for fertiliser N. f is initialised to false in all DCN constructors.

                        // Generate random numbers for the x, y and z coordinates
                        // within the range of the box width/depth/height

                        do
                        {
                            // 09.10.26 MSA Generate a new pseudorandom nitrate position within the given box coordinates; handle for edge cases 
                            RandomiseNitratePosition(&dcn, box_left, box_front, box_top, box_width, box_depth, box_height);
                        } while (dcn.x <= 0.0 || dcn.y <= 0.0 || dcn.z <= 0.0); //don't want any point to be 0

                        // Finally, add it to the list
                        m_nitrateList.push_back(dcn);
                    } // for (long i=0; i< number_nitrate_packets; ++i)
                } // for (long z=1; z<=NUM_Z; ++z)
            } // for (long x=1; x<=NUM_X; ++x)
        } //for (long y=1; y<=NUM_Y; ++y)

        return (return_value);
    }

    /* ******************************************************* */

    void Nitrate::CalculateSoilTemp(ProcessActionDescriptor* action, const bool initialise)
    {
        Use_Time;
        Use_Scoreboard;
        Use_SharedAttributeManager;
        Use_VolumeObjectCoordinator;
        // MSA 11.04.28 Replaces Use_Boundaries;
        const BoundaryArray& Z_boundaries = m_boundarySet->Z_boundaries;

        const ProcessTime_t period = PROCESS_DAYS(1);

        //VMD At this stage time does not start from 0/0/0 0:0:0, but from a specified date, so need to subtract the start time to
        //get the time since the start of the simulation.
        const ProcessTime_t current_time = time - action->GetPostOffice()->GetStart();

        // MSA 09.10.27 Currently treating Plants as the same for the purposes of these SharedAttributes
        //        --> OK to just use first plant here. 
        const SharedAttributeCluster& cluster = Get_Cluster("Plant");
        SharedAttributeCluster::const_iterator cluster_iter = cluster.begin();
        const SharedAttributeList& sa_list = (*cluster_iter).second;

        SharedAttribute* Base_Temp_sa = sa_list[saBaseTempIndex];
        const double base_temp = Base_Temp_sa->GetValue(); //base temperature for degree days calculation

        SharedAttribute* germination_lag_sa = sa_list[saGerminationLagIndex]; //Germination lag in hours since the seed was sown
        const ProcessTime_t time_of_germination = static_cast<ProcessTime_t>(PROCESS_HOURS(germination_lag_sa->GetValue())); //converting the germination lag from hours to seconds
        SharedAttribute* seeding_time_sa = sa_list[saSeedingTimeIndex]; //Time from zero (start of simulation) in hours when the seeds are sown
        const ProcessTime_t time_of_seeding = static_cast<ProcessTime_t>(PROCESS_HOURS(seeding_time_sa->GetValue())); //converting the seeding time from hours to seconds

        const double average_air_temp = (saMaxTemperature->GetValue() + saMinTemperature->GetValue()) / 2;

        const double degree_days = saDegreeDays->GetValue();
        saDegreeDays->SetValue(degree_days + average_air_temp - base_temp); // degree days from the start of the simulation

        double grow_start_degree_days = saGrowStartDegreeDays->GetValue();
        if ((0.0 == grow_start_degree_days) && (current_time > (time_of_germination + time_of_seeding)))
        {
            saGrowStartDegreeDays->SetValue(degree_days + average_air_temp - base_temp); // degree days from start of simulation to when plant started to grow
        }

        for (long z = 1; z <= NUM_Z; ++z)
        {
            for (long x = 1; x <= NUM_X; ++x)
            {
                for (long y = 1; y <= NUM_Y; ++y)
                {
                    const BoxIndex box_index = scoreboard->GetBoxIndex(x, y, z);

                    SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                        const double thermal_diffusivity = scoreboard->GetCharacteristicValue(Thermal_Diffusivity_Indices[idx], box_index);
                    const double avg_soil_depth = (Get_Layer_Top(z) + Get_Layer_Bottom(z)) / 2;
                    double soil_temp = 0.0;
                    if (initialise) //Call from StartUp to initialise the soil temp profile.
                    {
                        soil_temp = average_air_temp * exp(-avg_soil_depth * ProcessUtility::guardedPow((PI / (thermal_diffusivity * period)), 0.5, _logger));
                    }
                    else //DoNormalWakeUp calls this routine every 24hrs to recalculate the soil temperature
                    {
                        // MSA 10.11.08 Radiation Extinction Coefficient should be independent of barrier modelling, right? (In fact I suspect it really should be a SA)
                        const double radiation_extinction_coeff = Get_Concentration_Of_BoxIndex(Radiation_Extinction_Coeff_Index, box_index);
                        const double crop_cover_factor = Utility::CSMin(0.99, saGreenCover->GetValue()); //0-0.99, total crop cover including any residues, from Water.cpp [[ MSA 09.10.22 Was Get_Concentration_Of_Box(Weather_Data_Index, 1,1,8); ]]
                        const double LAI = -log(1 - crop_cover_factor) / radiation_extinction_coeff; //natural log, converting from crop cover to LAI, Woodruff-Hammer 1999
                        soil_temp = scoreboard->GetCharacteristicValue(Temperature_Index, box_index);
                        soil_temp = soil_temp + (average_air_temp - soil_temp) * exp(-avg_soil_depth * ProcessUtility::guardedPow((PI / (thermal_diffusivity * period)), 0.5, _logger)) * exp(-radiation_extinction_coeff * LAI);
                    }
                    scoreboard->SetCharacteristicValue(Temperature_Index, soil_temp, box_index); //soil temp, degC

                    SUBSECTION_ITER_WITH_PROPORTIONS_END
                } //for y
            } //for x
        } //for z    
    }

    /* ******************************************************* */

    long int Nitrate::DoGeneralPurpose(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        // we want to know who sent the wake up, so we can decide how to act.
        Use_Source;
        Use_Code;

        long source_module_id = -1;
        if (source != __nullptr)
        {
            source_module_id = source->GetProcessID();
        }

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
    long int Nitrate::DoExternalWakeUp(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;

        // we want to know who sent the wake up, so we can decide how to act.
        Use_Source;

        long source_module_id = -1;
        if (source != __nullptr)
        {
            source_module_id = source->GetProcessID();
        }

        // now check if it is the water module
        if (source_module_id == itsWaterModuleID)
        {
            // new code for this is in DoGeneralPurpose(...)
#if defined NITRATEWATERPHOSPHORUS_SPECIALPROCESSDATA_EXAMPLE_CODE
            WaterNotification * water_notification = dynamic_cast<WaterNotification *>(data);
            if (__nullptr != data)
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
            if (data == __nullptr)
            {
                return_value = LeachN(action);  //A message has been send from PWater::Drain
            }
            if (data != __nullptr)
            {
                return_value = MassFlowN(action);  //A message has been sent from PWater::RedistributeWater
            }
#endif // #if defined NITRATEWATERPHOSPHORUS_OLD_NOTIFICATION_CODE
        }

        return (return_value);
    }

    /* ************************************************************************** */


    long int Nitrate::DoNormalWakeUp(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_Time;

        return_value = DiffuseN(action);
        return_value = UptakeN(action);
        return_value = RedistributeDots(action);

        if (!m_IS_NUTRIENT_SOLUTION) //roots are growing in soil
        {
            if ((time - m_time_prev_temp) >= PROCESS_DAYS(1)) //recalculate the soil temperature every 24 hrs
            {
                CalculateSoilTemp(action, false); // false --> not initialising
                m_time_prev_temp = time;
            }
        }

        return_value |= Process::DoNormalWakeUp(action);
        return (return_value);
    }

    /* ************************************************************************** */

    long int Nitrate::DoScoreboardPlantSummaryRegistered(ProcessActionDescriptor* action)
    {
        Use_PlantSummaryRegistration;

        nitrate_first_plant_summary_index = psr->GetStartIndex();
        nitrate_number_of_plants = psr->GetNumberOfPlants();
        nitrate_number_of_branch_orders = psr->GetNumberOfBranchOrders();

        return (Process::DoScoreboardPlantSummaryRegistered(action));
    }

    long int Nitrate::DoRaytracerOutput(RaytracerData* raytracerData)
    {
        if (0 == raytracerData) return (kNoError);

        const ConstNitrateIterator end = m_nitrateList.end();
        for (ConstNitrateIterator iter = m_nitrateList.begin(); iter != end; ++iter)
        {
            if (iter->s >= m_NITRATE_PACKET_SIZE)
            {
                raytracerData->AddNitrate(*iter, iter->s);
            }
        }

        return kNoError;
    }

    /* ************************************************************************** */
    /* VMD 12/11/98 This routine redraws all the nitrate dots on the screen everytime the window is resized etc.
       MSA 09.11.12 It is now the only place in which nitrates are drawn (any routine which moves dots calls this one). */
    void Nitrate::DrawScoreboard(const DoubleRect& /* area */, Scoreboard* /* scoreboard */)
    {
        BEGIN_DRAWING
            // First, dump all the old locations
            drawing.RemoveAllDots();

        // Then, draw NITRATE_DRAW_PROPORTION (0, 1]
        // of those nitrate packets having s>0  
        NitrateIterator iter = m_nitrateList.begin();
        while (iter != m_nitrateList.end())
        {
            if (iter->DoDraw(m_NITRATE_PACKET_SIZE)) // draw==true and s==NITRATE_PACKET_SIZE
            {
                // m_dotColour is the prevailing colour
                drawing.DrawDot(*iter, m_dotColour);
            }
            ++iter;
        }
        END_DRAWING
    }

    // MSA 10.11.11 Should this function count its own BOX_COUNT, rather than using the Nitrate class's count?
    // Probably - for better encapsulation - but it should be OK as it's a private member function...
    double Nitrate::CountTotalNitrates(const Scoreboard* scoreboard) const
    {
        double totalNitrateAmount = 0.0;
        for (BoxIndex box_index = 0; box_index < BOX_COUNT; ++box_index)
        {
            for (CharacteristicIndices::const_iterator iter = Nitrate_Amount_Indices.begin(); iter != Nitrate_Amount_Indices.end(); ++iter)
            {
                totalNitrateAmount += Get_Concentration_Of_BoxIndex(*iter, box_index);
            }
        }
        return totalNitrateAmount;
    }

    /* **************************************************************************
    */

    /* VMD 22/9/98 This routine is used to model the ongoing redistribution of nitrate by diffusion. Using Fick's law of
          diffusion, the amount of transfer between a box and all its surrounding boxes is calculated.  This
          routine is to be called often so that the transfer in any particular time step is small, reducing the
          error of not simultaneously redistributing to all surrounding boxes.  It also reduces the error of not
          using a pure random motion routine. Transfer of nitrate between boxes by diffusion is so small it could be omitted.

       VMD 09/07/02 Tidied up the code a little */

    long int Nitrate::DiffuseN(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_Time;
        Use_Scoreboard;

        const ProcessTime_t dT = time - m_time_of_previous_diffusion;
        m_time_of_previous_diffusion = time;
        if (dT == 0)
        {
            return kNoError;
        }
        RmAssert(dT > 0, "Time delta since previous diffusion is negative");

        const double totalNitrateAmount = CountTotalNitrates(scoreboard); //mass balance - make sure that no nitrate is 'lost' during the diffusion process

        //Look at each box in the scoreboard and "equilibrate" it with its surrounding boxes.
        //This approach is computationally efficient, and closely approximates the computationally expensive
        //Finite Element Method approach, see Dunbabin et al. 2005.
        for (long y = 1; y <= NUM_Y; ++y)
        {
            for (long x = 1; x <= NUM_X; ++x)
            {
                for (long z = 1; z <= NUM_Z; ++z)
                {
                    //Calculate the transfer that would occur to the LHS box
                    if (x != 1) // For now, do not transfer nitrate outside the scoreboard.
                    {
                        CalculateNitrateDiffusion(action, x, y, z, x - 1, y, z, dT, X, false);
                    }

                    //Calculate the transfer that would occur to the RHS box
                    if (x != NUM_X) // For now, do not transfer nitrate outside the scoreboard.
                    {
                        CalculateNitrateDiffusion(action, x, y, z, x + 1, y, z, dT, X, true);
                    }

                    //Calculate the transfer that would occur to the box in front
                    if (y != 1) // For now, do not transfer nitrate outside the scoreboard.
                    {
                        CalculateNitrateDiffusion(action, x, y, z, x, y - 1, z, dT, Y, false);
                    }

                    //Calculate the transfer that would occur to the box behind
                    if (y != NUM_Y) // For now, do not transfer nitrate outside the scoreboard.
                    {
                        CalculateNitrateDiffusion(action, x, y, z, x, y + 1, z, dT, Y, true);
                    }

                    //Calculate the transfer that would occur to the box on top
                    if (z != 1) // For now, do not transfer nitrate outside the scoreboard.
                    {
                        CalculateNitrateDiffusion(action, x, y, z, x, y, z - 1, dT, Z, false);
                    }

                    //Calculate the transfer that would occur to the box beneath
                    if (z != NUM_Z)// For now, do not transfer nitrate outside the scoreboard.
                    {
                        CalculateNitrateDiffusion(action, x, y, z, x, y, z + 1, dT, Z, true);
                    }
                } //for (long z=1; z<=NUM_Z; ++z)
            } //for (long x=1; x<=NUM_X; ++x)
        } //for (long y=1; y<=NUM_Y; ++y)

        RmAssert(fabs(totalNitrateAmount - CountTotalNitrates(scoreboard)) < 0.5, "Total Nitrate balance inequal");

        return (return_value);
    } //Nitrate::DiffuseN(ProcessActionDescriptor *action)


    /* **************************************************************************
    */
    // N.B. The totalNitrateAmount counter is passed by value, not by reference. It's a copy and does not affect the variable used to call this function.
    bool Nitrate::CalculateNitrateDiffusion(ProcessActionDescriptor* action, const long& this_x, const long& this_y, const long& this_z, const long& next_x, const long& next_y, const long& next_z, const ProcessTime_t& dT, Dimension transferDimension, bool transferInPositiveDir)
    {
        Use_Scoreboard;
        // MSA 11.04.28 Replaces Use_Boundaries;
        const BoundaryArray& X_boundaries = m_boundarySet->X_boundaries;
        const BoundaryArray& Y_boundaries = m_boundarySet->Y_boundaries;
        const BoundaryArray& Z_boundaries = m_boundarySet->Z_boundaries;

        // MSA 10.11.09 New barrier modelling transfer restriction code
        // MSA 10.11.19 Note: This code assumes no zero-volume boxes.
        // MSA 11.01.25 Major update: recoding this method to address subsections of boxes serially.

        const BoxIndex thisBox = scoreboard->GetBoxIndex(this_x, this_y, this_z);
        const BoxIndex nextBox = scoreboard->GetBoxIndex(next_x, next_y, next_z);

        RmAssert(thisBox != nextBox, "Next box == this box"); // debug
        if (thisBox == nextBox) return false; // release

        const double thisBoxVolume = Get_BoxIndex_Volume(thisBox);
        const double nextBoxVolume = Get_BoxIndex_Volume(nextBox);

        if (transferDimension == NoDimension)
        {
            if (this_x != next_x)
            {
                transferDimension = X;
                transferInPositiveDir = next_x > this_x;
            }
            if (this_y != next_y)
            {
                RmAssert(transferDimension == NoDimension, "Invalid Box Coordinates");
                transferDimension = Y;
                transferInPositiveDir = next_y > this_y;
            }
            if (this_z != next_z)
            {
                RmAssert(transferDimension == NoDimension, "Invalid Box Coordinates");
                transferDimension = Z;
                transferInPositiveDir = next_z > this_z;
            }
            RmAssert(transferDimension != NoDimension, "Invalid Box Coordinates");
        }

        const double fromDist = (transferDimension == X) ? (Get_Layer_Width(this_x)) : ((transferDimension == Y) ? (Get_Layer_Depth(this_y)) : (Get_Layer_Height(this_z)));
        const double toDist = (transferDimension == X) ? (Get_Layer_Width(next_x)) : ((transferDimension == Y) ? (Get_Layer_Depth(next_y)) : (Get_Layer_Height(next_z)));

        const TransferRestrictionData& trd = scoreboard->GetTransferRestrictionData(thisBox, nextBox, scoreboard->GetTransferRestrictedDimension(transferDimension, transferInPositiveDir));
        const size_t thisBackgroundVOIndex = trd.backgroundVolumeObjectFrom == __nullptr ? 0 : trd.backgroundVolumeObjectFrom->GetIndex();
        const size_t nextBackgroundVOIndex = trd.backgroundVolumeObjectTo == __nullptr ? 0 : trd.backgroundVolumeObjectTo->GetIndex();

        const bool thisRestricted = trd.unrestrictedProportionFrom < 1.0;

        // --------------------------------------------------------------------------------------------

        if (thisRestricted)
        {
            // We must first calculate the diffusion from the restricted section (the section inaccessible to the next box) to the unrestricted section.
            if (trd.voFromPermeability > 0.0)
            {
                const size_t RIndex = trd.unrestrictedTransferWithinVO ? thisBackgroundVOIndex : trd.volumeObjectFrom->GetIndex();
                const size_t UIndex = trd.unrestrictedTransferWithinVO ? trd.volumeObjectFrom->GetIndex() : thisBackgroundVOIndex;

                // Call the actual diffusion method twice, once for each direction. Diffusion will only occur if the from-section's Nitrate concentration
                // is higher than the to-section's.
                for (size_t i = 0; i < 2; ++i)
                {
                    const size_t thisFromIndex = i == 0 ? RIndex : UIndex;
                    const size_t thisToIndex = i == 0 ? UIndex : RIndex;
                    const double thisProportion = scoreboard->GetCoincidentProportion(thisBox, thisFromIndex);
                    const double nextProportion = scoreboard->GetCoincidentProportion(thisBox, thisToIndex);
                    const double thisVolume = thisProportion * thisBoxVolume;
                    const double nextVolume = nextProportion * thisBoxVolume;
                    double dist = 0.0;
                    // Note we're using this_ coordinates (i.e. fromDist) for both terms in the equation, because we are diffusing WITHIN this box.
                    // Important note: we also halve the effective distance, because we are within one box, not crossing from one to another.
                    if (thisVolume < 1e-3 && nextVolume > 1e-2)
                    {
                        dist = thisProportion * fromDist / 2.0;
                    }
                    else if (thisVolume > 1e-2 && nextVolume < 1e-3)
                    {
                        dist = nextProportion * fromDist / 2.0;
                    }
                    else
                    {
                        dist = (thisProportion * fromDist + nextProportion * fromDist) / (2.0 * 2.0);
                    }

                    // Attempt to transfer some Nitrate from:    (i==0)    the restricted section to the unrestricted, then
                    //                                            (i==1)    the unrestricted section to the restricted.
                    DoDiffusion(scoreboard, thisBox, thisBox, thisFromIndex, thisToIndex, thisProportion, nextProportion,
                        dist, dT, trd.voFromPermeability);
                }
            }
            // else: no transfer between restricted and unrestricted sections possible - surface is impermeable.
        }


        // --------------------------------------------------------------------------------------------

        const bool nextRestricted = trd.unrestrictedProportionTo < 1.0;

        if (nextRestricted)
        {
            // We must first calculate the diffusion from the restricted section (the section inaccessible to the previous box) to the unrestricted section.
            if (trd.voToPermeability > 0.0)
            {
                const size_t RIndex = trd.unrestrictedTransferWithinVO ? nextBackgroundVOIndex : trd.volumeObjectTo->GetIndex();
                const size_t UIndex = trd.unrestrictedTransferWithinVO ? trd.volumeObjectTo->GetIndex() : nextBackgroundVOIndex;

                // Call the actual diffusion method twice, once for each direction. Diffusion will only occur if the from-section's Nitrate concentration
                // is higher than the to-section's.
                for (size_t i = 0; i < 2; ++i)
                {
                    const size_t thisFromIndex = i == 0 ? RIndex : UIndex;
                    const size_t thisToIndex = i == 0 ? UIndex : RIndex;
                    const double thisProportion = scoreboard->GetCoincidentProportion(nextBox, thisFromIndex);
                    const double nextProportion = scoreboard->GetCoincidentProportion(nextBox, thisToIndex);
                    const double thisVolume = thisProportion * nextBoxVolume;
                    const double nextVolume = nextProportion * nextBoxVolume;
                    double dist = 0.0;
                    // Note we're using next_ coordinates (i.e. toDist) for both terms in the equation, because we are diffusing WITHIN the next box.
                    // Important note: we also halve the effective distance, because we are within one box, not crossing from one to another.
                    if (thisVolume < 1e-3 && nextVolume > 1e-2)
                    {
                        dist = thisProportion * toDist / 2.0;
                    }
                    else if (thisVolume > 1e-2 && nextVolume < 1e-3)
                    {
                        dist = nextProportion * toDist / 2.0;
                    }
                    else
                    {
                        dist = (thisProportion * toDist + nextProportion * toDist) / (2.0 * 2.0);
                    }
                    // Attempt to transfer some Nitrate from:    (i==0)    the restricted section to the unrestricted, then
                    //                                            (i==1)    the unrestricted section to the restricted.
                    DoDiffusion(scoreboard, nextBox, nextBox, thisFromIndex, thisToIndex, thisProportion, nextProportion,
                        dist, dT, trd.voToPermeability);
                }
            }
            // else: no transfer between restricted and unrestricted sections possible - surface is impermeable.
        }


        /*
            Any transfer within each box has now been completed.
            -------------------------------------------------------------------------------------------
        */

        // 1) If neither Box is restricted, we just transfer from Box to Box within each spatial subsection.

        // 2) If both Boxen are restricted, we transfer from the unrestricted spatial subsection of one Box to the unrestricted spatial subsection of the other Box.

        // 3) If only one Box is restricted, we transfer from the unrestricted spatial subsection of the restricted Box to the same spatial subsection
        //      (i.e. the one corresponding with the same VO / lack of VO) of the other Box. 

        // 1)
        if (!thisRestricted && !nextRestricted)
        {
            Use_VolumeObjectCoordinator;

            SUBSECTION_ITER_WITH_TO_AND_FROM_BOXES_AND_PROPORTIONS_BEGIN

                // If there is no volume available to transfer out of or into, naturally we can't do any diffusion.
                if (thisProportion == 0 || nextProportion == 0) continue;

            const double thisVolume = thisProportion * thisBoxVolume;
            const double nextVolume = nextProportion * nextBoxVolume;

            double dist = 0.0;
            if (thisVolume < 1e-3 && nextVolume > 1e-2)
            {
                dist = thisProportion * fromDist;
            }
            else if (thisVolume > 1e-2 && nextVolume < 1e-3)
            {
                dist = nextProportion * toDist;
            }
            else
            {
                dist = (thisProportion * fromDist + nextProportion * toDist) / 2.0;
            }
            DoDiffusion(scoreboard, thisBox, nextBox, idx, idx,
                thisProportion, nextProportion, dist, dT);

            SUBSECTION_ITER_WITH_TO_AND_FROM_BOXES_AND_PROPORTIONS_END
        }

        // 2)
        else if (thisRestricted && nextRestricted)
        {
            size_t unrestrictedIndex = trd.unrestrictedTransferWithinVO ? trd.volumeObjectFrom->GetIndex() : thisBackgroundVOIndex;
            const size_t originalUnrestrictedIndex = unrestrictedIndex;

            double thisProportion = scoreboard->GetCoincidentProportion(thisBox, unrestrictedIndex);

            double permeability = 1.0;

            // We may have to handle for the special case in which thisBox is 100% restricted and the surface separating it from nextBox is permeable.
            if (thisProportion <= 0.0)
            {
                unrestrictedIndex = trd.volumeObjectFrom->GetIndex();
                thisProportion = scoreboard->GetCoincidentProportion(thisBox, unrestrictedIndex);
                permeability = trd.voFromPermeability;
                if (permeability <= 0.0) return true;
            }

            const double nextProportion = scoreboard->GetCoincidentProportion(nextBox, originalUnrestrictedIndex);
            const double thisVolume = thisProportion * thisBoxVolume;
            const double nextVolume = nextProportion * nextBoxVolume;

            double dist = 0.0;
            if (thisVolume < 1e-3 && nextVolume > 1e-2)
            {
                dist = thisProportion * fromDist;
            }
            else if (thisVolume > 1e-2 && nextVolume < 1e-3)
            {
                dist = nextProportion * toDist;
            }
            else
            {
                dist = (thisProportion * fromDist + nextProportion * toDist) / 2.0;
            }
            DoDiffusion(scoreboard, thisBox, nextBox, unrestrictedIndex, originalUnrestrictedIndex,
                thisProportion, nextProportion, dist, dT);
        }

        // 3)
        else
        {
            size_t unrestrictedIndex = thisRestricted ? (trd.unrestrictedTransferWithinVO ? trd.volumeObjectFrom->GetIndex() : thisBackgroundVOIndex)
                : (trd.unrestrictedTransferWithinVO ? trd.volumeObjectTo->GetIndex() : nextBackgroundVOIndex);
            const size_t originalUnrestrictedIndex = unrestrictedIndex;

            double thisProportion = scoreboard->GetCoincidentProportion(thisBox, unrestrictedIndex);

            double permeability = 1.0;

            // We may have to handle for the special case in which thisBox is 100% restricted and the surface separating it from nextBox is permeable.
            if (thisProportion <= 0.0)
            {
                unrestrictedIndex = trd.volumeObjectFrom->GetIndex();
                thisProportion = scoreboard->GetCoincidentProportion(thisBox, unrestrictedIndex);
                permeability = trd.voFromPermeability;
                if (permeability <= 0.0) return true;
            }

            const double nextProportion = scoreboard->GetCoincidentProportion(nextBox, originalUnrestrictedIndex);
            const double thisVolume = thisProportion * thisBoxVolume;
            const double nextVolume = nextProportion * nextBoxVolume;

            double dist = 0.0;
            if (thisVolume < 1e-3 && nextVolume > 1e-2)
            {
                dist = thisProportion * fromDist;
            }
            else if (thisVolume > 1e-2 && nextVolume < 1e-3)
            {
                dist = nextProportion * toDist;
            }
            else
            {
                dist = (thisProportion * fromDist + nextProportion * toDist) / 2.0;
            }
            DoDiffusion(scoreboard, thisBox, nextBox, unrestrictedIndex, originalUnrestrictedIndex,
                thisProportion, nextProportion, dist, dT, permeability);
        }

        return (true);
    } //Nitrate::CalculateNitrateDiffusion


    // MSA 11.01.27 This method does the actual diffusion between two spatial-subsections of adjacent boxes.
    // It does not check for transfer restriction; callers of this method should first ensure that the requested diffusion is legal.
    // The parameter volumeObjectSurfacePermeability is only used in the case of diffusion between different spatial subsections of the same Box.
    bool Nitrate::DoDiffusion(Scoreboard* scoreboard, const BoxIndex& fromBox, const BoxIndex& toBox, const size_t& fromVOIndex, const size_t& toVOIndex, const double& fromProportion, const double& toProportion, const double& avgDistance, const ProcessTime_t& dT, const double& volumeObjectSurfacePermeability /* = 0 */)
    {
        if (fromBox == toBox && (fromVOIndex == toVOIndex || volumeObjectSurfacePermeability <= 0)) return false; // Shortcut exit

        if (fromProportion <= 0.0 || toProportion <= 0.0) return false;

        const double fromVolume = Get_BoxIndex_Volume(fromBox) * fromProportion;
        const double toVolume = Get_BoxIndex_Volume(toBox) * toProportion;

        double NAmountFrom = scoreboard->GetCharacteristicValue(Nitrate_Amount_Indices[fromVOIndex], fromBox);
        const double waterContentFrom = scoreboard->GetCharacteristicValue(Water_Content_Indices[fromVOIndex], fromBox);
        const double NConcFrom = waterContentFrom <= 0.0 ? 0.0 : (NAmountFrom / (waterContentFrom * fromVolume)); //ugN/cm3H2O

        double NAmountTo = scoreboard->GetCharacteristicValue(Nitrate_Amount_Indices[toVOIndex], toBox);
        const double waterContentTo = scoreboard->GetCharacteristicValue(Water_Content_Indices[toVOIndex], toBox);
        const double NConcTo = waterContentTo <= 0.0 ? 0.0 : (NAmountTo / (waterContentTo * toVolume)); //ugN/cm3H2O

        if (NConcFrom <= NConcTo) return false;

        const double deltaNRestrictionModifier = fromVOIndex != toVOIndex ? volumeObjectSurfacePermeability : 1.0;

        if (m_IS_NUTRIENT_SOLUTION) //roots are growing in nutrient solution
        {
            //Assume the nutrient solution is well mixed, so there are no zones of local N depletion
            const double deltaConcN = (NConcFrom - NConcTo) / 2.0; //ugN/cm3H2O
            const double deltaN = deltaConcN * waterContentFrom * fromProportion * Get_BoxIndex_Volume(fromBox); //ugN

            if ((deltaN * deltaNRestrictionModifier) > NAmountFrom)
            {
                RmAssert((deltaN*deltaNRestrictionModifier) < 1e-15, "Error: non-trivial deltaN exceeds N amount");
                return false; // (assuming the assertion above is correct:) deltaN considered too small to bother with
            }

            NAmountFrom -= deltaNRestrictionModifier * deltaN;
            RmAssert(NAmountFrom >= 0.0, "Negative N amount");

            NAmountTo += deltaNRestrictionModifier * deltaN;
            scoreboard->SetCharacteristicValue(Nitrate_Amount_Indices[fromVOIndex], NAmountFrom, fromBox);
            scoreboard->SetCharacteristicValue(Nitrate_Amount_Indices[toVOIndex], NAmountTo, toBox);
            scoreboard->SetCharacteristicValue(Nitrate_Concentration_Indices[fromVOIndex], (NAmountFrom / (fromVolume * waterContentFrom)), fromBox);
            scoreboard->SetCharacteristicValue(Nitrate_Concentration_Indices[toVOIndex], (NAmountTo / (toVolume * waterContentTo)), toBox);
        } // roots are growing in nutrient solution

        else //roots are growing in soil
        {
            // To be conservative, will choose the water content of the drier box to calculate the impedance factor.
            // This is not a bad approximation as the transfer into a box will be limited by the driest portion that it
            // will have to traverse.
            double lowest_water_content = waterContentFrom;
            double coeff_c = scoreboard->GetCharacteristicValue(Coeff_C_Indices[fromVOIndex], fromBox);
            double coeff_d = scoreboard->GetCharacteristicValue(Coeff_D_Indices[fromVOIndex], fromBox);
            if (waterContentTo < waterContentFrom)
            {
                lowest_water_content = waterContentTo;
                coeff_c = scoreboard->GetCharacteristicValue(Coeff_C_Indices[toVOIndex], toBox);
                coeff_d = scoreboard->GetCharacteristicValue(Coeff_D_Indices[toVOIndex], toBox);
            }

            //impedance factor takes the form of a 2nd order polynomial. The user enters the coefficients that fit their soil.
            //This can easily be changed at any time to a different type of equation
            const double impedance_factor = (coeff_c * ProcessUtility::guardedPow(lowest_water_content, 2.0, _logger)) + (coeff_d * lowest_water_content);
            RmAssert(impedance_factor >= 0, "Negative impedance factor");
            double deltaN = m_DIFFUSION_COEFF_SOLN * lowest_water_content * impedance_factor * (NConcFrom - NConcTo) * avgDistance * dT;

            // Mitigating floating-point issues
            if (deltaN < 1e-10) return false;

            // Checks and balances
            double newNConcFrom = waterContentFrom <= 0.0 ? 0.0 : ((NAmountFrom - deltaN * deltaNRestrictionModifier) / (waterContentFrom * fromVolume));
            double newNConcTo = waterContentTo <= 0.0 ? 0.0 : ((NAmountTo + deltaN * deltaNRestrictionModifier) / (waterContentTo * toVolume));
            double concentrationDifference = newNConcFrom - newNConcTo;
            if (concentrationDifference < 0.0)
            {
                // Clamp transfer so as not to exceed equalisation

                /*
                The algebra: starting from the required result, that newNConcFrom == newNConcTo.

                [[MSA 11.03.24 Vanessa, could you check this please? My basic maths can be shockingly deficient at times]]


                (NAmountFrom - deltaN*deltaNRestrictionModifier) / (waterContentFrom * fromVolume)
                =
                (NAmountTo + deltaN*deltaNRestrictionModifier) / (waterContentTo * toVolume)

                so

                (NAmountFrom - deltaN*deltaNRestrictionModifier) * waterContentTo * toVolume
                =
                (NAmountTo + deltaN*deltaNRestrictionModifier) * waterContentFrom * fromVolume

                and

                NAmountFrom * waterContentTo * toVolume
                =
                (NAmountTo + deltaN*deltaNRestrictionModifier) * waterContentFrom * fromVolume + (deltaN*deltaNRestrictionModifier) * waterContentTo * toVolume

                and

                NAmountFrom * waterContentTo * toVolume
                =
                NAmountTo * waterContentFrom * fromVolume + (waterContentTo * toVolume + waterContentFrom * fromVolume) * deltaN*deltaNRestrictionModifier

                thus

                NAmountFrom * waterContentTo * toVolume - NAmountTo * waterContentFrom * fromVolume
                =
                (waterContentTo * toVolume + waterContentFrom * fromVolume) * deltaN*deltaNRestrictionModifier

                so, finally,

                deltaN*deltaNRestrictionModifier

                =

                (NAmountFrom * waterContentTo * toVolume - NAmountTo * waterContentFrom * fromVolume) / (waterContentTo * toVolume + waterContentFrom * fromVolume)

                */
                deltaN = (NAmountFrom * waterContentTo * toVolume - NAmountTo * waterContentFrom * fromVolume) / (waterContentTo * toVolume + waterContentFrom * fromVolume * deltaNRestrictionModifier);

                // Do a sanity check by using our result to recalculate new concentrations
                newNConcFrom = waterContentFrom <= 0.0 ? 0.0 : ((NAmountFrom - deltaN * deltaNRestrictionModifier) / (waterContentFrom * fromVolume));
                newNConcTo = waterContentTo <= 0.0 ? 0.0 : ((NAmountTo + deltaN * deltaNRestrictionModifier) / (waterContentTo * toVolume));

                RmAssert((newNConcTo - newNConcFrom) < 1e-10, "If variance is small: floating point inaccuracy; increase size of allowable epsilon value. If variance is large: fatal algebra error; return programmer to remedial high school mathematics class");

                // Mitigating floating-point issues
                if (deltaN < 1e-10) return false;
                /*
                double newNConcFromCheck = waterContentFrom<=0.0 ? 0.0 : ((NAmountFrom - deltaN*deltaNRestrictionModifier) / (waterContentFrom * fromVolume));
                double concentrationDifferenceCheck = newNConcFromCheck-avgNConc;
                */
            }

            NAmountFrom -= deltaNRestrictionModifier * deltaN;
            RmAssert(NAmountFrom >= 0.0, "Negative N amount");
            NAmountTo += deltaNRestrictionModifier * deltaN;

            //because N has been moved, the ratio of fertiliser to background N in the box has changed.
            const double fertFracFrom = scoreboard->GetCharacteristicValue(FertFracN_Indices[fromVOIndex], fromBox); //tracks what fraction of N in that soil box came from a fertiliser source vs the background soil N pool
            double fertFracTo = scoreboard->GetCharacteristicValue(FertFracN_Indices[toVOIndex], toBox);
            if ((fertFracFrom > 0.0) || (fertFracTo > 0.0))
            {
                RmAssert(((fertFracFrom <= 1.0) && (fertFracFrom >= 0.0)), "Fertiliser Fraction out of 0-1 bounds");
                fertFracTo = scoreboard->GetCharacteristicValue(FertFracN_Indices[toVOIndex], toBox);
                fertFracTo = ((NAmountTo * fertFracTo) + (deltaN * fertFracFrom)) / (NAmountTo);
                RmAssert(((fertFracTo <= 1.0) && (fertFracTo >= 0.0)), "Fertiliser Fraction out of 0-1 bounds");
                scoreboard->SetCharacteristicValue(FertFracN_Indices[toVOIndex], fertFracTo, toBox);
            }

            scoreboard->SetCharacteristicValue(Nitrate_Amount_Indices[fromVOIndex], NAmountFrom, fromBox);
            scoreboard->SetCharacteristicValue(Nitrate_Amount_Indices[toVOIndex], NAmountTo, toBox);
            scoreboard->SetCharacteristicValue(Nitrate_Concentration_Indices[fromVOIndex], newNConcFrom, fromBox);
            scoreboard->SetCharacteristicValue(Nitrate_Concentration_Indices[toVOIndex], newNConcTo, toBox);
        } //roots are growing in soil

        return (true);
    }


    //************************************************************************************
    // VMD 23/9/98 This routine is based on the nitrate leaching routine but is to be called by the water redistribution routine rather
    // than the drain routine.  As plant roots dry out a cell, water moves into it from neighboring cells.  That water will also bring with it nitrate,
    // so that there is constantly nitrate being drawn to the rhizosphere with the water as the plant transpires.  This routine differs from the leach
    // routine in that it allows nitrate movement in 3 dimensions.
    //VMD 3/11/99 Made changes to the calculation of the truncated area
#pragma warning( push )
#pragma warning( disable : 4706)
    long int Nitrate::MassFlowN(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_Scoreboard;
        Use_ScoreboardCoordinator;
        // MSA 11.04.28 Replaces Use_Boundaries;
        const BoundaryArray& X_boundaries = m_boundarySet->X_boundaries;
        const BoundaryArray& Y_boundaries = m_boundarySet->Y_boundaries;
        const BoundaryArray& Z_boundaries = m_boundarySet->Z_boundaries;

        Use_VolumeObjectCoordinator;

        bool doRedistribute = false; // Will be set to true if redistribution takes place

        // Cumulative accounting variable
        double nitrateDotsBefore = 0.0;

        // MSA 09.10.29 Using new Nitrate packet iteration    
        // IMPORTANT: Do not use a for loop if you will be removing nitrate packets. 
        // If you will be removing packets, use a while(iter!=m_nitrateList.end()) loop and
        // iter = m_nitrateList.erase(iter);        when packets are to be removed, and 
        // ++iter;        otherwise. 
        const NitrateIterator theEnd = m_nitrateList.end(); // Don't preload a "saved end iterator" like this if you will be adding packets (as adding packets changes where the end is, naturally).
        for (NitrateIterator iter = m_nitrateList.begin(); iter != theEnd; ++iter)
        {
            nitrateDotsBefore += iter->s; //Accounting for all the dots, make sure none get 'lost' during the massflow process

            if (Random1() > 0.20)
            {
                // MSA 09.10.29 All nitrates iterated are now valid. Nitrates that were previously set to (0,0,0) are now just discarded.
                // This 80% (on average) random skip condition is from pre-existing code.
                // Is it simulation logic, or was it just something necessitated by the old method of storing Nitrate packets?
                continue;
            }

            // MSA We don't want to have to re-determine which box this Nitrate is in every time, only when it's moved.
            BoxIndex box_index;
            size_t idx;
            BoxCoordinate box_coord;
            if (iter->containingBoxSubsectionKnown)
            {
                box_index = iter->containingBox;
                idx = iter->containingSubsection;
                scoreboard->GetXYZ(box_index, box_coord);
            }
            else
            {
                scoreboardcoordinator->FindBoxBySoilCoordinate(&box_coord, scoreboard, &(*iter));
                box_index = scoreboard->GetBoxIndex(box_coord.x, box_coord.y, box_coord.z);
                // MSA 10.11.11 Must determine whether this Nitrate packet is constrained by a VO.
                idx = volumeobjectcoordinator->GetContainingVolumeObjectIndex(*iter);
                iter->SetContainingBoxSubsection(box_index, idx);
            }

            double boxWidth = Get_Layer_Width(box_coord.x);
            double boxDepth = Get_Layer_Depth(box_coord.y);
            double boxHeight = Get_Layer_Height(box_coord.z);


            double waterMoved = scoreboard->GetCharacteristicValue(Water_Moved_Indices[idx], box_index);
            if (waterMoved < 0.01)
            {
                // MSA 09.10.15 Do nothing - not enough water movement to move nitrate.
                continue;
            }

            const int directionMoved = scoreboard->GetCharacteristicValue(Direction_Moved_Indices[idx], box_index);
            double fieldCapacity = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], box_index);


            // Otherwise, move some nitrate.

            doRedistribute = true;
            double meanDisplacement = 0.0;
            double wettingFront = 0.0;

            // MSA 11.02.03 Again, this code assumes VolumeObjects to be impermeable to water/nutrients.
            // MSA TODO Update to support semipermeable VOs.
            // MSA 11.07.01 Furthermore: this method is entirely dependent on Water Moved, which is subject to barrier modelling in Water.
            // Hence there is no need to take barrier modelling into accout here.
            double coincidentProportion = scoreboard->GetCoincidentProportion(box_index, idx);

            // calculate mean displacement.

            switch (directionMoved)
            {
            case -1:
            {
                if (1 == box_coord.x) break; // If water and hence the nitrate moved through the left hand face of the Scoreboard (-x direction), bail out
                double boxLeft = Get_Layer_Left(box_coord.x);
                int i = 0;
                while (boxLeft > Get_Layer_Left(1) // Ensure water and hence nitrate have not moved through the left hand face of the Scoreboard (-x direction)
                    && i < 2 // Go through a maximum of two boxen
                    && waterMoved >(fieldCapacity * boxWidth * coincidentProportion) // Ensure there's enough water to fill this box and more
                    )
                {
                    waterMoved -= fieldCapacity * boxWidth * coincidentProportion;
                    ++i;
                    boxLeft = Get_Layer_Left(box_coord.x - i);
                    boxWidth = Get_Layer_Width(box_coord.x - i);
                    const BoxIndex nextBoxIndex = scoreboard->GetBoxIndex(box_coord.x - i, box_coord.y, box_coord.z);
                    fieldCapacity = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], nextBoxIndex);
                    coincidentProportion = scoreboard->GetCoincidentProportion(nextBoxIndex, idx);

                    meanDisplacement += boxWidth;
                }
                boxWidth = Get_Layer_Width(box_coord.x - i);
                meanDisplacement += (waterMoved / fieldCapacity);
                wettingFront = Get_Layer_Left(box_coord.x - 1);

                break;
            }
            case 1:
            {
                if (NUM_X == box_coord.x) break; // If water and hence the nitrate moved through the right hand face of the Scoreboard (+x direction), bail out
                double boxRight = Get_Layer_Right(box_coord.x);
                int i = 0;
                while (boxRight > Get_Layer_Right(NUM_X) // Ensure water and hence nitrate have not moved through the right hand face of the Scoreboard (+x direction)
                    && i < 2 // Go through a maximum of two boxen
                    && waterMoved >(fieldCapacity * boxWidth * coincidentProportion) // Ensure there's enough water to fill this box and more
                    )
                {
                    waterMoved -= fieldCapacity * boxWidth * coincidentProportion;
                    ++i;
                    boxRight = Get_Layer_Right(box_coord.x + i);
                    boxWidth = Get_Layer_Width(box_coord.x + i);
                    const BoxIndex nextBoxIndex = scoreboard->GetBoxIndex(box_coord.x + i, box_coord.y, box_coord.z);
                    fieldCapacity = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], nextBoxIndex);
                    coincidentProportion = scoreboard->GetCoincidentProportion(nextBoxIndex, idx);

                    meanDisplacement += boxWidth;
                }
                boxWidth = Get_Layer_Width(box_coord.x + i);
                meanDisplacement += (waterMoved / fieldCapacity);
                wettingFront = Get_Layer_Right(box_coord.x + 1);

                break;
            }
            case -2:
            {
                if (1 == box_coord.y) break; // If water and hence the nitrate moved through the front face of the Scoreboard (-y direction), bail out
                double boxFront = Get_Layer_Front(box_coord.y);
                int i = 0;
                while (boxFront > Get_Layer_Front(1) // Ensure water and hence nitrate have not moved through the front face of the Scoreboard (-y direction)
                    && i < 2 // Go through a maximum of two boxen
                    && waterMoved >(fieldCapacity * boxDepth * coincidentProportion) // Ensure there's enough water to fill this box and more
                    )
                {
                    waterMoved -= fieldCapacity * boxWidth * coincidentProportion;
                    ++i;
                    boxFront = Get_Layer_Front(box_coord.y - i);
                    boxDepth = Get_Layer_Depth(box_coord.y - i);
                    const BoxIndex nextBoxIndex = scoreboard->GetBoxIndex(box_coord.x, box_coord.y - i, box_coord.z);
                    fieldCapacity = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], nextBoxIndex);
                    coincidentProportion = scoreboard->GetCoincidentProportion(nextBoxIndex, idx);

                    meanDisplacement += boxWidth;
                }
                boxDepth = Get_Layer_Depth(box_coord.y - i);
                meanDisplacement += (waterMoved / fieldCapacity);
                wettingFront = Get_Layer_Front(box_coord.y - 1);

                break;
            }
            case 2:
            {
                if (NUM_Y == box_coord.y) break; // If water and hence the nitrate moved through the back face of the Scoreboard (+y direction), bail out
                double boxBack = Get_Layer_Back(box_coord.y);
                int i = 0;
                while (boxBack > Get_Layer_Back(NUM_Y) // Ensure water and hence nitrate have not moved through the back face of the Scoreboard (+y direction)
                    && i < 2 // Go through a maximum of two boxen
                    && waterMoved >(fieldCapacity * boxDepth * coincidentProportion) // Ensure there's enough water to fill this box and more
                    )
                {
                    waterMoved -= fieldCapacity * boxWidth * coincidentProportion;
                    ++i;
                    boxBack = Get_Layer_Back(box_coord.y + i);
                    boxDepth = Get_Layer_Depth(box_coord.y + i);
                    const BoxIndex nextBoxIndex = scoreboard->GetBoxIndex(box_coord.x, box_coord.y + i, box_coord.z);
                    fieldCapacity = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], nextBoxIndex);
                    coincidentProportion = scoreboard->GetCoincidentProportion(nextBoxIndex, idx);

                    meanDisplacement += boxWidth;
                }
                boxDepth = Get_Layer_Depth(box_coord.y + i);
                meanDisplacement += (waterMoved / fieldCapacity);
                wettingFront = Get_Layer_Back(box_coord.y + 1);

                break;
            }
            case -3:
            {
                if (1 == box_coord.z) break; // If water and hence the nitrate moved through the top face of the box (-z direction), bail out
                double boxTop = Get_Layer_Top(box_coord.z);
                int i = 0;
                while (boxTop > Get_Layer_Top(1) // Ensure water and hence nitrate have not moved through the top face of the box (-z direction)
                    && i < 2 // Go through a maximum of two boxen
                    && waterMoved >(fieldCapacity * boxHeight * coincidentProportion) // Ensure there's enough water to fill this box and more
                    )
                {
                    waterMoved -= fieldCapacity * boxHeight * coincidentProportion;
                    ++i;
                    boxTop = Get_Layer_Top(box_coord.z - i);
                    boxHeight = Get_Layer_Height(box_coord.z - i);
                    const BoxIndex nextBoxIndex = scoreboard->GetBoxIndex(box_coord.x, box_coord.y, box_coord.z - i);
                    fieldCapacity = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], nextBoxIndex);
                    coincidentProportion = scoreboard->GetCoincidentProportion(nextBoxIndex, idx);

                    meanDisplacement += boxHeight;
                }
                boxHeight = Get_Layer_Height(box_coord.z - i);
                meanDisplacement += (waterMoved / fieldCapacity);
                wettingFront = Get_Layer_Top(box_coord.z - 1);

                break;
            }
            case 3:
            {
                if (NUM_Z == box_coord.z) break; // If water and hence the nitrate moved through the bottom face of the box (+z direction), bail out
                double boxBottom = Get_Layer_Bottom(box_coord.z);
                int i = 0;
                while (boxBottom > Get_Layer_Bottom(NUM_Z) // Ensure water and hence nitrate have not moved through the bottom face of the box (+z direction)
                    && i < 2 // Go through a maximum of two boxen
                    && waterMoved >(fieldCapacity * boxHeight * coincidentProportion) // Ensure there's enough water to fill this box and more
                    )
                {
                    waterMoved -= fieldCapacity * boxHeight * coincidentProportion;
                    ++i;
                    boxBottom = Get_Layer_Bottom(box_coord.z + i);
                    boxHeight = Get_Layer_Height(box_coord.z + i);
                    const BoxIndex nextBoxIndex = scoreboard->GetBoxIndex(box_coord.x, box_coord.y, box_coord.z + i);
                    fieldCapacity = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], nextBoxIndex);
                    coincidentProportion = scoreboard->GetCoincidentProportion(nextBoxIndex, idx);

                    meanDisplacement += boxHeight;
                }
                boxHeight = Get_Layer_Height(box_coord.z + i);
                meanDisplacement += (waterMoved / fieldCapacity);
                wettingFront = Get_Layer_Bottom(box_coord.z + 1);

                break;
            }
            default:
                RmAssert(false, "Invalid movement direction");
                break;
            }

            if (meanDisplacement == 0.0)
            {
                continue; // If there is no displacement, this nitrate will not move. Go to the next one.
            }


            const double STEP_SIZE_SMALL = meanDisplacement / 5.0; //this makes the division of the frequency distribution relative to the size of the mean displacement

            //Now that we know what the mean displacement is we can move some nitrate.

            const double dispersivity = scoreboard->GetCharacteristicValue(Dispersivity_Indices[idx], box_index); //user defined displacement dependent dispersivity cm
            const double variance = 2.0 * dispersivity * (meanDisplacement);
            double position = meanDisplacement;

            const double frequency_at_mean = (1 / (sqrt(2 * PI * variance))) * (exp(-((ProcessUtility::guardedPow((position - (meanDisplacement)), 2.0, _logger)) / (2.0 * variance))));
            double frequency = frequency_at_mean;

            while (frequency > (FREQ_CUTOFF * frequency_at_mean))
            {
                position -= STEP_SIZE_SMALL;
                frequency = (1.0 / (sqrt(2.0 * PI * variance))) * (exp(-((ProcessUtility::guardedPow((position - (meanDisplacement)), 2.0, _logger)) / (2.0 * variance))));
            }

            // We are going to store the frequency distribution into an array so we need to know how many points there are going to be.
            // MSA 11.02.03 Converting this to use a vector, but retaining the number_of_points for efficient allocation
            position += STEP_SIZE_SMALL;
            const long number_of_points = static_cast<const long>((meanDisplacement - position) / STEP_SIZE_SMALL + 1.0);

            std::vector<double> frequency_vector(number_of_points);

            // Firstly just calculate the entire distribution regardless of any wetting front positions or scoreboard boundaries.
            // Will start at the mean and calulate the distribution for one - ie keep calculating in increments or decrements of z
            // until the frequency value is less than the cut off frequency.  There will be too many points if we go any further along each tail.
            // since the distribution is symmetrical, there is no need to waste time calculating both sides - they are exactly the same!

            // Calculate one side of the distribution starting at the mean.  The other side is a mirror image of the first.

            // MSA 09.10.21 Compacted this conditional / loop structure

            position = meanDisplacement;
            double truncated_area = 0.0;
            double non_truncated_area = 0.0;
            bool truncate = false;
            double dot_coordinate = 0.0;

            for (long index = 0; index < number_of_points; ++index)
            {
                const double thisFreq = (1.0 / (sqrt(2.0 * PI * variance))) * (exp(-((ProcessUtility::guardedPow((position - (meanDisplacement)), 2.0, _logger)) / (2.0 * variance))));

                frequency_vector[index] = thisFreq;

                // A suitable increment in z value is determined by calculating the area under the frequency distribution with smaller and smaller
                // delta z values until the value for the area does not change significantly.  We want to calculate the truncated area as we go so
                // the position of the truncation point will determine if we do the "RHS" or "LHS" of the distribution.

                switch (directionMoved)
                {
                case -1:
                case 1:
                    dot_coordinate = iter->x;
                    break;
                case -2:
                case 2:
                    dot_coordinate = iter->y;
                    break;
                case -3:
                case 3:
                    dot_coordinate = iter->z;
                    break;
                default:
                    RmAssert(false, "Invalid directionMoved encountered");
                }

                bool decrementPosition = false;
                if ((decrementPosition = (directionMoved < 0 && (dot_coordinate - meanDisplacement) < wettingFront)
                    && (dot_coordinate - position) < wettingFront) // moved -ve
                    || // OR...
                    (decrementPosition = (directionMoved > 0 && (dot_coordinate + meanDisplacement) > wettingFront)
                        && (dot_coordinate + position) > wettingFront)) // moved +ve
                {
                    truncated_area += frequency * STEP_SIZE_SMALL;
                    truncate = true;
                }
                else if ((directionMoved < 0 && (dot_coordinate - position) >= wettingFront) // moved -ve
                    || // OR...
                    (directionMoved > 0 && (dot_coordinate + position) <= wettingFront)) // moved +ve
                {
                    non_truncated_area += frequency * STEP_SIZE_SMALL;
                    truncate = true;
                }

                if (decrementPosition)
                {
                    position -= STEP_SIZE_SMALL;
                }
                else
                {
                    position += STEP_SIZE_SMALL;
                }
            } // for(long index=0; index<number_of_points; ++index)


            // Adjust truncated area
            if (dot_coordinate + meanDisplacement > wettingFront)
            {
                truncated_area += 0.5; //All of the RHS of the distribution is truncated as well.
            }
            if (dot_coordinate + meanDisplacement < wettingFront)
            {
                truncated_area = 0.5 - non_truncated_area;
            }


            // A nitrate cannot move past the wetting front, so the area under the frequency distribution that is truncated must
            // be redistributed over the remaining area.  This is done using the adjustment factor.

            bool no_displacement = true;
            if (truncated_area >= 1)
            {
                truncate = false;
                no_displacement = false;
            }

            if (truncate)
            {
                double adjustment_factor = truncated_area / (1.0 - truncated_area);
                for (long index = 0; index < number_of_points; ++index)
                {
                    double freq = frequency_vector[index];
                    frequency_vector[index] = freq + (freq * adjustment_factor);
                }
            }

            double random_number;
            do
            {
                random_number = Random1();
            } while (random_number > 0.98);


            //Keep adding the area under the frequency distribution until the area is greater than or equal to the random number.

            if (!no_displacement)
            {
                meanDisplacement = 0.0;
                continue;
            }

            double theArea = 0.0;
            bool area_finished = false;
            bool points_finished = false;
            //Start at the very LHS of the distribution and step right.
            position = meanDisplacement - number_of_points * STEP_SIZE_SMALL;
            long index = number_of_points - 1;
            double theFrequency;

            while (!points_finished && !area_finished)
            {
                theFrequency = frequency_vector[index];
                theArea += (STEP_SIZE_SMALL * theFrequency);

                if (theArea >= random_number)
                {
                    meanDisplacement = position + (0.5 * STEP_SIZE_SMALL);
                    area_finished = true;
                }

                if (index == 0)
                {
                    points_finished = true;
                }
                --index;
                position += STEP_SIZE_SMALL;
            } //while (!points_finished && !area_finished)

            index = 0;

            while (points_finished && !area_finished)
                // Will leach past the mean
            {
                theFrequency = frequency_vector[index];
                theArea += (STEP_SIZE_SMALL * theFrequency);

                if (theArea >= random_number)
                {
                    meanDisplacement = position + (0.5 * STEP_SIZE_SMALL);
                    area_finished = true;
                }

                LOG_ERROR_IF((index == number_of_points) && !area_finished);
                if ((index == number_of_points) && !area_finished)
                {
                    points_finished = false;
                }

                ++index;
                position += STEP_SIZE_SMALL;
            } //while (points_finished && !area_finished)

            position -= STEP_SIZE_SMALL;
            LOG_ERROR_IF((iter->z == 0.0) && (iter->x == 0.0));

            if (meanDisplacement <= 0.0)
            {
                // No displacement - nitrate has not moved. Go to next nitrate.
                continue;
            }

            switch (directionMoved)
            {
            case -1:
            {
                const double x = Utility::CSMax(iter->x - meanDisplacement / 2.0, wettingFront); //Nitrate is moved to the new x position.
                iter->x = x;
                break;
            }
            case -2:
            {
                const double y = Utility::CSMax(iter->y - meanDisplacement / 2.0, wettingFront); //Nitrate is moved to the new y position.
                iter->y = y;
                break;
            }
            case -3:
            {
                const double z = Utility::CSMax(iter->z - meanDisplacement / 2.0, wettingFront); //Nitrate is moved to the new z position.
                iter->z = z;
                break;
            }
            case 1:
            {
                const double x = Utility::CSMin(iter->x + meanDisplacement / 2.0, wettingFront); //Nitrate is moved to the new x position.
                iter->x = x;
                LOG_ERROR_IF((iter->x > (Get_Layer_Right(NUM_X))) || (iter->x < (Get_Layer_Left(1))));
                break;
            }
            case 2:
            {
                const double y = Utility::CSMin(iter->y + meanDisplacement / 2.0, wettingFront); //Nitrate is moved to the new y position.
                iter->y = y;
                LOG_ERROR_IF((iter->y > (Get_Layer_Back(NUM_Y))) || (iter->y < (Get_Layer_Front(1))));
                break;
            }
            case 3:
            {
                const double z = Utility::CSMin(iter->z + meanDisplacement / 2.0, wettingFront); //Nitrate is moved to the new z position.
                iter->z = z < 0.0 ? 0.01 : z;
                LOG_ERROR_IF((iter->z > (Get_Layer_Bottom(NUM_Z))) || (iter->z < (Get_Layer_Top(1))));
                break;
            }
            default:
                RmAssert(false, "Invalid directionMoved");
            }
            // MSA 11.04.28 DoubleCoordinateNitrate has moved.
            iter->containingBoxSubsectionKnown = false;
        } //for(NitrateIterator iter = m_nitrateList.begin(); iter!=theEnd; ++iter)


        // Go through and work out how many nitrates are now in each scoreboard box and hence recalculate
        // the total amount in each box.
        if (doRedistribute)
        {
            //massflow has caused nitrate dots to move, so now need to recalculate the nitrate amount in each box
            RecalculateNitrateAmounts(scoreboard, scoreboardcoordinator, volumeobjectcoordinator);
        }//if (doRedistribute)

        // Redraw the scoreboard to reflect the changes made by this routine.
        DrawScoreboard(DoubleRect(), __nullptr);

        return (return_value);
    } //long int Nitrate::MassFlowN(ProcessActionDescriptor *action)
#pragma warning( pop ) 


    /* ****************************************************************************************************** */
    // VMD 3/11/99 made changes the calculation of the mean displacement and the calculation of the truncated area.
    // MSA 10.07.22 This method leaches N in response to water movement.
    //                We thus allow the water movement code to handle barrier modelling
    //                and assume that, as no water movement blocked by VolumeObjects will occur,
    //                no nitrate leaching blocked by VolumeObjects will occur.

    long int Nitrate::LeachN(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_ScoreboardCoordinator;
        Use_Scoreboard;
        // MSA 11.04.28 Replaces Use_Boundaries;
        const BoundaryArray& Z_boundaries = m_boundarySet->Z_boundaries;
        Use_VolumeObjectCoordinator;

        // MSA 09.10.29 Using new Nitrate packet iteration
        NitrateIterator iter = m_nitrateList.begin();
        while (iter != m_nitrateList.end())
        {
            double mean_displacement = 0.0;

            // MSA We don't want to have to re-determine which box this Nitrate is in every time, only when it's moved.
            BoxIndex box_index;
            size_t idx;
            BoxCoordinate bc;
            if (iter->containingBoxSubsectionKnown)
            {
                box_index = iter->containingBox;
                idx = iter->containingSubsection;
                scoreboard->GetXYZ(box_index, bc);
            }
            else
            {
                scoreboardcoordinator->FindBoxBySoilCoordinate(&bc, scoreboard, &(*iter));
                box_index = scoreboard->GetBoxIndex(bc.x, bc.y, bc.z);
                // MSA 10.11.11 Must determine whether this Nitrate packet is constrained by a VO.
                idx = volumeobjectcoordinator->GetContainingVolumeObjectIndex(*iter);
                iter->SetContainingBoxSubsection(box_index, idx);
            }

            double box_top = Get_Layer_Top(bc.z);
            double box_height = Get_Layer_Height(bc.z);
            const double dist_fraction = ((iter->z) - box_top) / box_height;

            double water_in = scoreboard->GetCharacteristicValue(Rained_Amount_Indices[idx], box_index); //cm of water

            bool endsboard = false;
            if (water_in == 0.0)
            {
                ++iter;
                continue;
            } // No water has swept past and no nitrate needs to leach. Go to next nitrate.

            double water_out = (bc.z <= (NUM_Z - 1)) ? scoreboard->GetCharacteristicValue(Rained_Amount_Indices[idx], scoreboard->GetBoxIndex(bc.x, bc.y, bc.z + 1))
                : scoreboard->GetCharacteristicValue(Rain_Out_Indices[idx], box_index);
            double field_capacity = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], box_index);

            //How much water flows past the nitrate?
            const double delta_water = water_in - water_out;
            double wetting_front = 0.0;

            if (water_out == water_in) //This box was already filled to field capacity so all the water has moved to the next box
            {
                int i = 0;
                while (water_out == water_in && (bc.z + i + 1) <= NUM_Z)
                {
                    ++i;
                    water_out = scoreboard->GetCharacteristicValue(Rained_Amount_Indices[idx], scoreboard->GetBoxIndex(bc.x, bc.y, (bc.z) + i + 1));
                    water_in = scoreboard->GetCharacteristicValue(Rained_Amount_Indices[idx], scoreboard->GetBoxIndex(bc.x, bc.y, (bc.z) + i));
                }
                if ((bc.z + i + 1) > NUM_Z)
                {
                    endsboard = true; // There has been so much water that the little nitrate has been swept out of the bottom of the scoreboard.
                    water_out = scoreboard->GetCharacteristicValue(Rain_Out_Indices[idx], scoreboard->GetBoxIndex(bc.x, bc.y, (bc.z) + i));
                }
                field_capacity = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], scoreboard->GetBoxIndex(bc.x, bc.y, (bc.z) + i));

                box_height = Get_Layer_Height((bc.z) + i);
                double coincidentProportion = scoreboard->GetCoincidentProportion(box_index, idx);
                while (water_in > (field_capacity * box_height * coincidentProportion) && (bc.z + i) <= NUM_Z)
                {
                    ++i;
                    water_in -= (field_capacity * box_height * coincidentProportion); //maximum potential displacement in dry soil
                    if (((bc.z) + i) <= NUM_Z)
                    {
                        const BoxIndex nextBoxIndex = scoreboard->GetBoxIndex(bc.x, bc.y, (bc.z) + i);
                        field_capacity = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], nextBoxIndex);
                        box_height = Get_Layer_Height((bc.z) + i);
                        coincidentProportion = scoreboard->GetCoincidentProportion(nextBoxIndex, idx);
                    }
                } //while (water_in>(field_capacity*box_height) && (bc.z+i)<=NUM_Z)

                if (((bc.z) + i) > NUM_Z)
                {
                    endsboard = true;
                    box_top = Get_Layer_Bottom((bc.z));
                }
                else
                {
                    box_top = Get_Layer_Top((bc.z) + i);
                }
                mean_displacement = (water_in / field_capacity) + box_top - (iter->z);
            } //if (water_out==water_in)
            else if (water_out != 0.0) //The nitrate ions don't get swept along for a few boxes.
            {
                water_in -= (delta_water * dist_fraction); //this is how much water would get to that nitrate ion

                if (water_in <= field_capacity * (box_height * (1.0 - dist_fraction)))
                {
                    mean_displacement = (water_in / field_capacity);
                } // if (water_in <= field_capacity*(box_height*(1-dist_fraction)))
                else
                {
                    int i = 1;
                    water_in -= field_capacity * (box_height * (1.0 - dist_fraction)); // How much water goes into the next box
                    if (((bc.z) + i) > NUM_Z)
                    {
                        field_capacity = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], box_index);
                    }
                    else
                    {
                        field_capacity = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], scoreboard->GetBoxIndex(bc.x, bc.y, (bc.z) + i));
                        box_height = Get_Layer_Height((bc.z) + i);
                    }

                    double coincidentProportion = scoreboard->GetCoincidentProportion(box_index, idx);
                    while ((water_in > field_capacity * box_height * coincidentProportion) && (((bc.z) + i) <= NUM_Z))
                    {
                        ++i;
                        water_in -= (field_capacity * box_height);
                        if (((bc.z) + i) <= NUM_Z)
                        {
                            const BoxIndex nextBoxIndex = scoreboard->GetBoxIndex(bc.x, bc.y, bc.z + i);
                            field_capacity = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], nextBoxIndex);
                            box_height = Get_Layer_Height((bc.z) + i);
                            coincidentProportion = scoreboard->GetCoincidentProportion(nextBoxIndex, idx);
                        }
                    } //while
                    if ((bc.z + i) > NUM_Z)
                    {
                        box_top = Get_Layer_End(Z, (bc.z));
                        endsboard = true;
                    }
                    else
                    {
                        box_top = Get_Layer_Start(Z, (bc.z) + i);
                    }
                    mean_displacement = (water_in / field_capacity) + box_top - (iter->z);
                } // else
            } // else if (water_out != 0.0)
            else // water_out==0.0
            {
                water_in -= (delta_water * dist_fraction);
                mean_displacement = (water_in / field_capacity);
            }

            const double STEP_SIZE = mean_displacement / 10.0; //this makes the frequency distribution division relative to the size of the mean displacement
            double position = 0.0;

            if (!endsboard) //the mean displacement of the nitrate ion is not out the bottom of the board so need to calculate its actual displacement
            {
                water_out = scoreboard->GetCharacteristicValue(Rain_Out_Indices[idx], scoreboard->GetBoxIndex(bc.x, bc.y, (bc.z) + 1));

                if (water_out != 0) // Mean nitrate displacement will be below this box.
                {
                    wetting_front = scoreboard->GetCharacteristicValue(Wetting_Front_Indices[idx], scoreboard->GetBoxIndex(bc.x, bc.y, (bc.z) + 1));

                    int i = 1;
                    while (wetting_front == 0.0 && (i + bc.z) <= NUM_Z)
                    {
                        ++i;
                        wetting_front = scoreboard->GetCharacteristicValue(Wetting_Front_Indices[idx], scoreboard->GetBoxIndex(bc.x, bc.y, (bc.z) + i));
                    } //while (wetting_front == 0)

                    if ((i + bc.z) > NUM_Z)
                    {
                        wetting_front = 0.0;
                    }
                } //if (water_out !=0)

                else // Water has not moved out of the box, and may or may not have gone past the nitrate.
                {
                    wetting_front = scoreboard->GetCharacteristicValue(Wetting_Front_Indices[idx], box_index);
                    if (iter->z > wetting_front)
                    {
                        mean_displacement = 0.0; // the wetting front is above the nitrate so it wont have moved any where.
                    }
                } //if (water_out == 0.0)

                if (mean_displacement <= 0.5)
                {
                    mean_displacement = 0.0; // do nothing as no water has gone past thus the nitrate is going nowhere.
                }

                //Now that we know what the mean displacement is we can do some leaching!!

                if (mean_displacement != 0.0) //Water has gone past the nitrate and it will leach.
                {
                    const double dispersivity = scoreboard->GetCharacteristicValue(Dispersivity_Indices[idx], box_index);

                    const double variance = 2 * dispersivity * mean_displacement;
                    position = mean_displacement;

                    const double frequency_at_mean = (1.0 / (sqrt(2.0 * PI * variance))) * (exp(-((ProcessUtility::guardedPow((position - mean_displacement), 2.0, _logger)) / (2.0 * variance))));
                    double frequency = frequency_at_mean;

                    while (frequency > (FREQ_CUTOFF * frequency_at_mean))
                    {
                        position -= STEP_SIZE;
                        frequency = (1 / (sqrt(2.0 * PI * variance))) * (exp(-((ProcessUtility::guardedPow((position - mean_displacement), 2.0, _logger)) / (2.0 * variance))));
                    }

                    // We are going to store the frequency distribution into an array so we need to know how many points there are going to be.
                    position += STEP_SIZE;
                    const long number_of_points = static_cast<const long>((mean_displacement - position) / STEP_SIZE + 1);
                    std::vector<double> frequency_vector(number_of_points);

                    // Firstly just calculate the entire distribution regardless of any wetting front positions or scoreboard boundaries.
                    // Will start at the mean and calulate the distribution for one - ie keep calculating in increments or decrements of z
                    // until the frequency value is less than a the cut off frequency.  There will be too many points if we go any further along each tail.
                    // since the distribution is symmetrical, ther is no need to waste time calculating both sides - they are exactly the same!

                    // Calculate one of the distribution starting at the mean.  The other side is a mirror image of the first.

                    position = mean_displacement;
                    double truncated_area = 0.0;
                    double non_truncated_area = 0.0;
                    bool truncate = false;

                    for (int index = 0; index < number_of_points; ++index)
                    {
                        frequency = (1.0 / (sqrt(2.0 * PI * variance))) * (exp(-((ProcessUtility::guardedPow((position - mean_displacement), 2.0, _logger)) / (2.0 * variance))));

                        frequency_vector[index] = frequency;


                        // A suitable increment in z value is determined by calculating the area under the frequency distribution with smaller and smaller
                        // delta z values until the value for the area does not change significantly.  We want to calculate the truncated area as we go so
                        // the position of the truncation point will determine if we do the "RHS" or "LHS" of the distribution.

                        //The wetting front is in the LHS of the distribution, and any position greater than the wetting front will be truncated.
                        //Only need to calculate to the LHS and then add 0.5 to get the total truncated area.
                        if ((mean_displacement + iter->z > wetting_front) && (wetting_front > 0.0))
                        {
                            if (((position + iter->z) > wetting_front) && (wetting_front > 0.0))
                            {
                                truncated_area += frequency * STEP_SIZE;
                                truncate = true;
                            }
                            position -= STEP_SIZE;
                        }

                        //Else the wetting front is at the mean or in the RHS of the distribution.  Any point greater than the wetting front
                        //must be truncated.  Only need to calculate the RHS.  Calculate the area upto the wetting front and add 0.5 gives the total non-truncated
                        //area, then subtract this from 1 to get the truncated area.
                        else
                        {
                            if (((position + iter->z) <= wetting_front) && (wetting_front > 0.0))
                            {
                                non_truncated_area += frequency * STEP_SIZE;
                                truncate = true;
                            }
                            position += STEP_SIZE;
                        }
                    } // for(int index=0; index<number_of_points; ++index)

                    //If the wetting front is in the LHS of the distribution, all of the RHS is truncated so need to add this on.
                    if ((mean_displacement + iter->z > wetting_front) && (wetting_front > 0.0))
                    {
                        truncated_area += 0.5; //All of the RHS of the distribution is truncated aswell.
                    }

                    //If the wetting front is in the RHS of the distribution, only the portion greater than the wetting front is truncated.
                    //This is calculated by subtracting the non-truncated area from the total area 1 (or 0.5 as only need to look at half the distribution).
                    if ((mean_displacement + iter->z < wetting_front) && (wetting_front > 0.0))
                    {
                        truncated_area = 0.5 - non_truncated_area;
                    }

                    // A nitrate cannot move passed the wetting front, so the area under the frequency distribution that is truncated must
                    // be redistributed over the remaining area, so that the probability of being in the included area is 1.
                    // This is done using the adjustment factor - a multiplication factor that makes the area under the truncated curve the same
                    // as that under the original curve.

                    bool no_displacement = true;
                    if (truncated_area >= 1.0)
                    {
                        truncate = false;
                        no_displacement = false;
                    }

                    if (truncate)
                    {
                        const double adjustment_factor = truncated_area / (1.0 - truncated_area);
                        for (long index = 0; index < number_of_points; ++index)
                        {
                            frequency = frequency_vector[index];
                            frequency_vector[index] = frequency + (frequency * adjustment_factor);
                        }
                    }

                    double random_number;
                    do
                    {
                        random_number = Random1();
                    } while (random_number > 0.95);

                    //Keep adding the area under the frequency distribution until the area is greater than or equal to the random number.

                    if (!no_displacement)
                    {
                        mean_displacement = 0.0;
                    }
                    else
                    {
                        double area = 0.0;
                        bool points_finished = false;
                        bool area_finished = false;
                        position = mean_displacement - ((number_of_points)* STEP_SIZE);
                        int index = (number_of_points - 1);

                        while (!points_finished && !area_finished)
                        {
                            area += (STEP_SIZE * frequency_vector[index]);

                            if (area >= random_number)
                            {
                                mean_displacement = position + (0.5 * STEP_SIZE);
                                area_finished = true;
                            }

                            if (index == 0)
                            {
                                points_finished = true;
                            }
                            --index;
                            position += STEP_SIZE;
                        } // while (!points_finished && !area_finished)

                        index = 0;
                        while (points_finished && !area_finished)
                            // Will leach past the mean
                        {
                            area += (STEP_SIZE * frequency_vector[index]);

                            if (area >= random_number)
                            {
                                mean_displacement = position + (0.5 * STEP_SIZE);
                                area_finished = true;
                            }

                            if (index == number_of_points && !area_finished)
                            {
                                points_finished = false;
                                // MSA 10.09.20 Huh? This is the exact same conditional as above...
                                LOG_ERROR_IF((index == number_of_points) && !area_finished);
                            }

                            ++index;
                            position += STEP_SIZE;
                        } //while (points_finished && !area_finished)
                    } //if (no_displacement)
                }//if mean_displacement!=0
            } //if (!endsboard)

            position -= STEP_SIZE;

            LOG_ERROR_IF((iter->z == 0.0) && (iter->x == 0.0));

            iter->z = (iter->z) + mean_displacement; //Nitrate is leached to the new z position.

            if ((iter->z > wetting_front) && (wetting_front > 0.0) && (mean_displacement > 0.0))
            {
                iter->z = wetting_front;
                // MSA 11.04.28 DoubleCoordinateNitrate has moved.
                iter->containingBoxSubsectionKnown = false;
            }

            if (iter->z <= 0.0)
            {
                iter->z = 0.1;
                // MSA 11.04.28 DoubleCoordinateNitrate has moved.
                iter->containingBoxSubsectionKnown = false;
            }

            if (endsboard || iter->z > Get_Layer_Bottom(NUM_Z))
            {
                //The water and the nitrate has flowed out the bottom of the scoreboard hence make the nitrate
                //coords 0,0,0 so that it disappears off the screen.

                scoreboard->AddCharacteristicValue(Nitrate_Leached_Indices[idx], m_NITRATE_PACKET_SIZE, scoreboard->GetBoxIndex(bc.x, bc.y, 1));

                // Erase this nitrate packet and set the iterator to the next valid packet
                iter = m_nitrateList.erase(iter);
            }
            else
            {
                ++iter; // Set the iterator to the next packet.
            }
        } // while(iter!=m_nitrateList.end())        END OF BIG WHILE LOOP!


        //leaching has caused nitrate dots to move, so now need to recalculate the nitrate amount in each box

        // MSA TODO 10.11.12 Should convert this to a call to Nitrate::RecalculateNitrateAmounts()

        // MSA 09.10.26 Converted to rangewise setting
        for (CharacteristicIndices::const_iterator citer = Nitrate_Amount_Indices.begin(); citer != Nitrate_Amount_Indices.end(); ++citer)
        {
            scoreboard->SetCharacteristicValueRange(*citer, 0.0, 0, BOX_COUNT);
        }
        for (CharacteristicIndices::const_iterator citer = Rained_Amount_Indices.begin(); citer != Rained_Amount_Indices.end(); ++citer)
        {
            scoreboard->SetCharacteristicValueRange(*citer, 0.0, 0, BOX_COUNT);
        }

        NitrateIterator theEnd = m_nitrateList.end();
        for (NitrateIterator nIter = m_nitrateList.begin(); nIter != theEnd; ++nIter)
        {
            // MSA We don't want to have to re-determine which box this Nitrate is in every time, only when it's moved.
            BoxIndex box_index;
            size_t idx;
            BoxCoordinate bc;
            if (nIter->containingBoxSubsectionKnown)
            {
                box_index = nIter->containingBox;
                idx = nIter->containingSubsection;
                scoreboard->GetXYZ(box_index, bc);
            }
            else
            {
                scoreboardcoordinator->FindBoxBySoilCoordinate(&bc, scoreboard, &(*nIter));
                box_index = scoreboard->GetBoxIndex(bc.x, bc.y, bc.z);
                // MSA 10.11.11 Must determine whether this Nitrate packet is constrained by a VO.
                idx = volumeobjectcoordinator->GetContainingVolumeObjectIndex(*nIter);
                nIter->SetContainingBoxSubsection(box_index, idx);
            }

            double nitrate_amount = scoreboard->GetCharacteristicValue(Nitrate_Amount_Indices[idx], box_index);
            nitrate_amount += nIter->s;
            scoreboard->SetCharacteristicValue(Nitrate_Amount_Indices[idx], nitrate_amount, box_index);

            const double water_content = scoreboard->GetCharacteristicValue(Water_Content_Indices[idx], box_index);

            const double thisVolume = scoreboard->GetCoincidentProportion(box_index, idx) * Get_Box_Volume(bc.x, bc.y, bc.z);

            scoreboard->SetCharacteristicValue(Nitrate_Concentration_Indices[idx], (nitrate_amount / (thisVolume * water_content)), box_index);
        } // for(NitrateIterator nIter = m_nitrateList.begin(); nIter!=theEnd; ++nIter)

        // Redraw the scoreboard to reflect the changes made by this routine.
        DrawScoreboard(DoubleRect(), __nullptr);

        return (return_value);
    } // long int Nitrate::LeachN(ProcessActionDescriptor *action)

    /* ********************************************************************************* */

    long int Nitrate::UptakeN(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_Time;
        Use_Scoreboard;
        Use_SharedAttributeManager;
        Use_VolumeObjectCoordinator;
        const ProcessTime_t dT = time - m_time_prev_waking;
        m_time_prev_waking = time;

        if (dT == 0)
        {
            return kNoError;
        }
        RmAssert(dT > 0, "Time delta since previous waking is negative");


        const double buffer_power = 1.0; //for non-adsorbed nitrate
        // MSA 10.11.15 Replaced with class-level m_DIFFUSION_COEFF_SOLN
        //const double diffusion_coeff = 1.9e-5; //cm2/sec nitrate/chloride diffusion coeff in free solution

        if (!m_IS_NUTRIENT_SOLUTION && m_CALCULATE_NITRATE_MINERALISATION) //roots are growing in soil
        {
            //This routine calculates nitrate mineralisation rates from field measurements of mineralisable N
            CalculateNitrateMineralisation(action, dT);
        }

        //
        // Believe it or not, this is effectively a "for each Plant do..." loop
        const SharedAttributeCluster& cluster = Get_Cluster("Plant");
        for (SharedAttributeCluster::const_iterator cluster_iter = cluster.begin();
            cluster_iter != cluster.end();
            ++cluster_iter
            )
        {
            //                                                                          RvH
            // This is "use the next Plant X attribute list".
            // Note that the name of the Plant/Cluster is in (*cluster_iter).first
            const SharedAttributeList& sa_list = (*cluster_iter).second;

            //
            // the variable saNitrateUptakePerPlantIndex is a data member that was
            // found during ::PrepareSharedAttributes
            //SharedAttribute * root_length_sa = sa_list[saRootLengthPerPlantIndex];    // MSA 09.10.22 Unused

            SharedAttribute* root_length_ro_sa = sa_list[saRootLengthPerPlantPerRootOrderIndex];
            SharedAttribute* root_radius_ro_sa = sa_list[saRootRadiusPerPlantPerRootOrderIndex];
            SharedAttribute* water_flux_sa = sa_list[saWaterFluxPerPlantPerRootOrderIndex];
            SharedAttribute* absorption_power_sa = sa_list[saAbsorptionPowerPerPlantIndex];
            SharedAttribute* nitrate_uptake_sa = sa_list[saNitrateUptakePerPlantIndex]; //Total nitrate taken up by each plant, in each soil volume (subsection), in each time-step (spatial)
            SharedAttribute* cumul_nitrate_uptake_sa = sa_list[saCumulNitrateUptakePerPlantIndex]; //Cumulative total nitrate taken up by each plant (non-spatial)
            SharedAttribute* cumul_fertN_uptake_sa = sa_list[saCumulFertNUptakePerPlantIndex]; //Cumulative total fertiliser N taken up by each plant (non-spatial)
            SharedAttribute* nitrate_uptake_ro_sa = sa_list[saNitrateUptakePerPlantPerRootOrderIndex]; //Cumulative total nitrate taken up by each root order of each plant (non-spatial)
            SharedAttribute* local_nitrate_uptake_sa = sa_list[saLocalNitrateUptakePerPlantIndex]; //Cumulative nitrate uptake from each soil volume (subsection) by each plant (spatial)
            SharedAttribute* local_fertN_uptake_sa = sa_list[saLocalFertNUptakePerPlantIndex]; //Cumulative fertiliser N uptake from each soil volume (subsection) by each plant (spatial)
            SharedAttribute* nutr_soln_renew_sa = sa_list[saNutrientSolnRenewIndex];
            SharedAttribute* nutr_renew_sa = sa_list[saNutrientRenewIndex];
            const double nutr_soln_rnw = PROCESS_DAYS(nutr_soln_renew_sa->GetValue()); //time in seconds between solution changes // MSA 11.01.20 I feel pretty confident leaving this one ignorant of barrier modelling
            const double nutr_rnw = PROCESS_DAYS(nutr_renew_sa->GetValue()); //time in seconds between additions of nutrient solution to free draining sandy soil in pots

            for (BoxIndex box_index = 0; box_index < BOX_COUNT; ++box_index)
            {
                SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                    const double thisVolume = thisProportion * Get_BoxIndex_Volume(box_index);
                if (thisVolume <= 0) continue;

                // For root lengths, [0] = tap root, [1] = first root order, etc
                const double root_lengths[ROOT_ORDER_COUNT] = { root_length_ro_sa->GetValue(box_index, 0, idx), root_length_ro_sa->GetValue(box_index, 1, idx), root_length_ro_sa->GetValue(box_index, 2, idx), root_length_ro_sa->GetValue(box_index, 3, idx) };
                const double root_radii[ROOT_ORDER_COUNT] = { root_radius_ro_sa->GetValue(box_index, 0, idx), root_radius_ro_sa->GetValue(box_index, 1, idx), root_radius_ro_sa->GetValue(box_index, 2, idx), root_radius_ro_sa->GetValue(box_index, 3, idx) };
                const double water_fluxes[ROOT_ORDER_COUNT] = { water_flux_sa->GetValue(box_index, 0, idx), water_flux_sa->GetValue(box_index, 1, idx), water_flux_sa->GetValue(box_index, 2, idx), water_flux_sa->GetValue(box_index, 3, idx) };
                double delta_amounts[ROOT_ORDER_COUNT] = { 0.0,0.0,0.0,0.0 };
                double factors[ROOT_ORDER_COUNT * ROOT_ORDER_COUNT] = { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 };

                LOG_ERROR_IF((root_lengths[0] < 0.0) || (root_lengths[1] < 0.0) || (root_lengths[2] < 0.0) || (root_lengths[3] < 0.0));

                double water_content = scoreboard->GetCharacteristicValue(Water_Content_Indices[idx], box_index);

                const double wilting_point = scoreboard->GetCharacteristicValue(Wilting_Point_Indices[idx], box_index);
                // MSA 11.01.20 Is absorption_power independent of barrier modelling?
                // MSA 11.03.15 Yes. I think so.
                const double absorption_power = absorption_power_sa->GetValue(); //cm3H2O/cmroot.s    

                //impedance factor takes the form of a 2nd order polynomial. The user enters the coefficients that fit their soil.
                //This can easily be changed at any time to a different type of equation
                // MSA 10.11.15 Coefficients are independent of barrier modelling, right? 10.12.22 NO.
                const double coeff_c = Get_Concentration_Of_BoxIndex(Coeff_C_Indices[0], box_index);
                const double coeff_d = Get_Concentration_Of_BoxIndex(Coeff_D_Indices[0], box_index);
                double impedance_factor = (coeff_c * ProcessUtility::guardedPow(water_content, 2, _logger)) + (coeff_d * water_content);

                if (m_IS_NUTRIENT_SOLUTION)
                {
                    impedance_factor = 1.0; //roots growing in nutrient solution

                    ProcessTime_t dT_soln = time - m_time_of_prev_solution_change;

                    if (dT_soln > nutr_soln_rnw)
                    {
                        // MSA 10.11.15 Renewing solution.
                        const double renew_labile_N_conc = Get_Concentration_Of_BoxIndex(Nitrate_Renew_Indices[0], box_index); // µg/mL = µg/cm^3 water (initial total amount of labile N. Uniform, so we just grab the idx==0 case.)

                        scoreboard->SetCharacteristicValue(Nitrate_Concentration_Indices[idx], renew_labile_N_conc, box_index);

                        const double renew_labile_N = renew_labile_N_conc * thisVolume; //µgN in solution
                        scoreboard->SetCharacteristicValue(Nitrate_Amount_Indices[idx], renew_labile_N, box_index);

                        if (box_index == (BOX_COUNT - 1))
                        {
                            m_time_of_prev_solution_change = time;
                        }
                    } //if(dT_soln > nutr_soln_rnw)
                } //(m_IS_NUTRIENT_SOLUTION)

                if (nutr_rnw > 1e-6) //roots growing in a free draining sandy soil with nutrient solution added frequently to maintain water and nutrient content
                {
                    ProcessTime_t dT_nutr_rnw = time - m_time_of_prev_nutr_rnw;
                    if (dT_nutr_rnw > nutr_rnw)
                    {
                        const double renew_labile_N_conc = Get_Concentration_Of_BoxIndex(Nitrate_Renew_Indices[0], box_index); // µgN/g (initial total amount of labile N. Uniform, so we just grab the idx==0 case.)
                        const double bulkDensity = scoreboard->GetCharacteristicValue(Bulk_Density_Indices[idx], box_index);

                        const double renew_labile_N = renew_labile_N_conc * thisVolume * bulkDensity; //µgN
                        scoreboard->SetCharacteristicValue(Nitrate_Concentration_Indices[idx], (renew_labile_N / (thisVolume * water_content)), box_index); //UgN / cm3 / cm3/cm3 = ugN/cm3

                        scoreboard->SetCharacteristicValue(Nitrate_Amount_Indices[idx], renew_labile_N, box_index);

                        if (box_index == (BOX_COUNT - 1))
                        {
                            m_time_of_prev_nutr_rnw = time;
                        }
                    }
                } //if (nutr_rnw > 1e-6)


                const double nitrate_amount = scoreboard->GetCharacteristicValue(Nitrate_Amount_Indices[idx], box_index); //µgN
                const double nitrate_concentration = nitrate_amount / (water_content * thisVolume); //µgN/cm^3 water

                LOG_ERROR_IF(impedance_factor < 0.0); // a check against negative impedance factor
                RmAssert(impedance_factor >= 0.0, "Negative impedance factor");

                if ((root_lengths[0] + root_lengths[1] + root_lengths[2] + root_lengths[3] <= 0.00000001)
                    || (nitrate_concentration <= 0.0)
                    || (water_content <= (wilting_point * 1.0001)))
                {
                    continue; // No further calculations to be done on this section of this box. Go to next one.
                }

                for (int i = 0; i < ROOT_ORDER_COUNT; ++i)
                {
                    if (root_lengths[i] > 0 && water_fluxes[i] > 0.0)
                    {
                        factors[i * ROOT_ORDER_COUNT] = (-2.0 * PI * root_radii[i] * (absorption_power / (2.0 * PI * root_radii[i])) * (root_lengths[i])) / buffer_power;
                        factors[i * ROOT_ORDER_COUNT + 1] = ((absorption_power / (2.0 * PI * root_radii[i])) / water_fluxes[i]);
                        factors[i * ROOT_ORDER_COUNT + 2] = (root_radii[i] * water_fluxes[i]) / (m_DIFFUSION_COEFF_SOLN * water_content * impedance_factor * buffer_power);
                        factors[i * ROOT_ORDER_COUNT + 3] = (1.0 / (ProcessUtility::guardedPow((PI * (root_lengths[i] / thisVolume)), 0.5, _logger))) / root_radii[i];
                        delta_amounts[i] = (factors[i * ROOT_ORDER_COUNT] * ((nitrate_concentration) / (factors[i * ROOT_ORDER_COUNT + 1] + (1.0 - factors[i * ROOT_ORDER_COUNT + 1]) * (2.0 / (2.0 - factors[i * ROOT_ORDER_COUNT + 2])) * (((ProcessUtility::guardedPow(factors[i * ROOT_ORDER_COUNT + 3], (2.0 - factors[i * ROOT_ORDER_COUNT + 2]), _logger)) - 1.0) / (ProcessUtility::guardedPow(factors[i * ROOT_ORDER_COUNT + 3], 2.0, _logger) - 1.0)))));
                        if (!((delta_amounts[i] < 0.0) && (delta_amounts[i] > -1000.0))) { delta_amounts[i] = 0.0; }
                    }
                }

                double deltaN = ((delta_amounts[0] + delta_amounts[1] + delta_amounts[2] + delta_amounts[3]) * dT);
                double new_amount = nitrate_amount + deltaN; //µgN

                LOG_ERROR_IF((delta_amounts[0] > 0.0) || (delta_amounts[1] > 0.0) || (delta_amounts[2] > 0.0) || (delta_amounts[3] > 0.0));

                long int il = 0;
                bool finished = false;
                while ((new_amount < 0.01) && !finished)
                {
                    deltaN /= 2.0;
                    delta_amounts[0] /= 2.0; //µgN/s
                    delta_amounts[1] /= 2.0;
                    delta_amounts[2] /= 2.0;
                    delta_amounts[3] /= 2.0;
                    new_amount = nitrate_amount + deltaN; //µgN
                    ++il;
                    LOG_ERROR_IF(il > 1000);
                    if (nitrate_amount < 0.01)
                    {
                        new_amount = nitrate_amount;
                        deltaN = 0;
                        delta_amounts[0] = 0.0; //µgN/s
                        delta_amounts[1] = 0.0;
                        delta_amounts[2] = 0.0;
                        delta_amounts[3] = 0.0;
                        finished = true;
                    }
                }

                if ((delta_amounts[0] <= 0.0) && (delta_amounts[1] <= 0.0) && (delta_amounts[2] <= 0.0) && (delta_amounts[3] <= 0.0))
                {
                    //Total nitrate taken up by each plant at each growth time step and each soil volume (spatial), used in PlantDynamicResourceAllocationToRoots
                    double new_uptake = -(((delta_amounts[0] + delta_amounts[1] + delta_amounts[2] + delta_amounts[3]) * dT) / 14.0067); //µmolN

                    // MSA 09.11.04 This error conditional (nitrate_uptake==0) evaluates to true very often.
                    // What action should be taken in this case, rather than logging thousands of errors?
                    // For now, I've commented it out to reduce log spam.
                    //LOG_ERROR_IF(nitrate_uptake == 0);

                    // In addition, I've changed the order to reduce the number of function calls required.

                    if (new_uptake != 0.0)
                    {
                        nitrate_uptake_sa->SetValue((nitrate_uptake_sa->GetValue(box_index, idx) + new_uptake), box_index, idx); //µmolN, Total nitrate taken up by each plant at each growth time step and each soil volume (spatial), used in PlantDynamicResourceAllocationToRoots
                    }

                    scoreboard->SetCharacteristicValue(Nitrate_Amount_Indices[idx], new_amount, box_index); //µgN, new soil nitrate content
                    scoreboard->SetCharacteristicValue(Nitrate_Concentration_Indices[idx], new_amount / (thisVolume * water_content), box_index);

                    new_uptake = -(delta_amounts[0] + delta_amounts[1] + delta_amounts[2] + delta_amounts[3]) * dT; //µgN taken up this timestep

                    double cumulative_nitrate_uptake = cumul_nitrate_uptake_sa->GetValue(); //total cumulative uptake by the plant, non_spatial
                    cumulative_nitrate_uptake += new_uptake;//µgN
                    cumul_nitrate_uptake_sa->SetValue(cumulative_nitrate_uptake); // MSA 11.01.20 should this SA be VO-wise? It's not otherwise spatial...

                    //Cumulative total nitrate taken up by each root order of each plant (non-spatial)
                    for (int i = 0; i < ROOT_ORDER_COUNT; ++i)
                    {
                        cumulative_nitrate_uptake = nitrate_uptake_ro_sa->GetValue(0, i, idx);
                        cumulative_nitrate_uptake += (-delta_amounts[i] * dT); //µgN
                        nitrate_uptake_ro_sa->SetValue(cumulative_nitrate_uptake, 0, i, idx);
                    }

                    cumulative_nitrate_uptake = local_nitrate_uptake_sa->GetValue(box_index, idx); //Cumulative nitrate uptake from each soil volume by each plant (spatial)
                    cumulative_nitrate_uptake += new_uptake; //µgN
                    local_nitrate_uptake_sa->SetValue(cumulative_nitrate_uptake, box_index, idx);

                    //what fraction of the N taken up was from a fertiliser source?
                    const double fertfracN = scoreboard->GetCharacteristicValue(FertFracN_Indices[idx], box_index);
                    LOG_ERROR_IF(!((fertfracN <= 1) && (fertfracN >= 0)));

                    cumulative_nitrate_uptake = cumul_fertN_uptake_sa->GetValue(); //µgN Cumulative total fertiliser N taken up by each plant (non-spatial)
                    cumulative_nitrate_uptake += new_uptake * fertfracN;
                    cumul_fertN_uptake_sa->SetValue(cumulative_nitrate_uptake);

                    cumulative_nitrate_uptake = local_fertN_uptake_sa->GetValue(box_index, idx); //µgN Cumulative fertiliser N uptake from each soil volume by each plant (spatial)
                    cumulative_nitrate_uptake += new_uptake * fertfracN;
                    local_fertN_uptake_sa->SetValue(cumulative_nitrate_uptake, box_index, idx);
                } //if ((delta_amounts[0] <= 0) && (delta_amounts[1] <= 0) && (delta_amounts[2] <= 0) && (delta_amounts[3] <= 0))
                else
                {
                    LOG_ERROR << HERE << "Not all delta_amountX were <=0. 0:" << delta_amounts[0] << ", 1:" << delta_amounts[1] << ", 2:" << delta_amounts[2] << ", 3:" << delta_amounts[3];
                }

                SUBSECTION_ITER_WITH_PROPORTIONS_END // for (spatial subsections of box)
            } // for(BoxIndex box_index = 0; box_index < BOX_COUNT; ++box_index)
        }//for "each Plant"

        return (return_value);
    } //Nitrate::UptakeN(ProcessActionDescriptor *action)

    /*****************************************************************************************************/

    void Nitrate::CalculateNitrateMineralisation(ProcessActionDescriptor* action, const ProcessTime_t& dT)
    {
        //Mineralisation of soil N is calculated using the potentially mineralisable N approach (Campbell et al. 1981, 1984, 1988)
        Use_Scoreboard;
        Use_VolumeObjectCoordinator;
        // MSA 11.04.28 Replaces Use_Boundaries;
        const BoundaryArray& Z_boundaries = m_boundarySet->Z_boundaries;

        // MSA 09.10.28 Converted to SharedAttribute
        const double mineralisation_depth = saMineralisationDepth->GetValue(); //non-spatial soil parameter
        //mineralisation_depth = scoreboard->GetCharacteristicValue(Mineralisation_Depth_Indices[idx],0); 

        for (long y = 1; y <= NUM_Y; ++y)
        {
            for (long x = 1; x <= NUM_X; ++x)
            {
                for (long z = 1; z <= NUM_Z; ++z)
                {
                    const double box_bottom = Get_Layer_Bottom(z);

                    if (box_bottom <= mineralisation_depth) //the user defines to what depth mineralisation will be calculated. Default is 10cm.
                    {
                        const BoxIndex box_index = scoreboard->GetBoxIndex(x, y, z);

                        SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                            if (thisProportion <= 0.0) continue;

                        const double thisVolume = thisProportion * Get_BoxIndex_Volume(box_index);

                        const double water_content = scoreboard->GetCharacteristicValue(Water_Content_Indices[idx], box_index);
                        const double drained_upper_limit = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], box_index);
                        const double wilting_point = scoreboard->GetCharacteristicValue(Wilting_Point_Indices[idx], box_index);
                        double rate_constant = scoreboard->GetCharacteristicValue(Rate_Constant_Indices[idx], box_index);
                        const double rate_const_temp = scoreboard->GetCharacteristicValue(Rate_Constant_Temperature_Indices[idx], box_index);
                        //The mineralisation rate constant is modified by a soil moisture factor, Campbell et al. 1988
                        const double soil_moisture_factor = (water_content - wilting_point) / (drained_upper_limit - wilting_point);
                        rate_constant *= soil_moisture_factor;

                        //The Q10 method is used for determining the effect of temperature on the rate constant, Campbell et al. 1981, 1984
                        const double Q10 = scoreboard->GetCharacteristicValue(Q10_Indices[idx], box_index);
                        const double soil_temp = scoreboard->GetCharacteristicValue(Temperature_Index, box_index); //soil temp, degC
                        const double temperature_coeff = 10 / (rate_const_temp - soil_temp);

                        // bits and pieces
                        const double one = ProcessUtility::guardedPow(rate_constant, temperature_coeff, _logger);
                        const double two = one * Q10;
                        const double three = 1.0 / temperature_coeff;

                        rate_constant = ProcessUtility::guardedPow(two, three, _logger);


                        //rate_constant = ProcessUtility::guardedPow(ProcessUtility::guardedPow(rate_constant,temperature_coeff)*Q10, (1.0/temperature_coeff));

                        double mineralisable_N = scoreboard->GetCharacteristicValue(Mineralisable_N_Indices[idx], box_index); //µg/g
                        double nitrate_amount = scoreboard->GetCharacteristicValue(Nitrate_Amount_Indices[idx], box_index);

                        const double mineralised_amount = mineralisable_N * (1.0 - exp(-rate_constant * dT)); //µgN/g
                        mineralisable_N -= mineralised_amount; //µg/g
                        scoreboard->SetCharacteristicValue(Mineralisable_N_Indices[idx], mineralisable_N, box_index);//µg/g
                        const double bulk_density = scoreboard->GetCharacteristicValue(Bulk_Density_Indices[idx], box_index);
                        scoreboard->SetCharacteristicValue(Mineralised_N_Indices[idx], (mineralised_amount * bulk_density * thisVolume), box_index); //µgN
                        m_cumulativeMineralisation += (mineralised_amount * bulk_density * thisVolume); //µgN    

                        double fertfracN = scoreboard->GetCharacteristicValue(FertFracN_Indices[idx], box_index);
                        if (fertfracN > 0.0)
                        {
                            fertfracN = (fertfracN * nitrate_amount) / (nitrate_amount + (mineralised_amount * bulk_density * thisVolume));
                            scoreboard->SetCharacteristicValue(FertFracN_Indices[idx], fertfracN, box_index);
                        }

                        nitrate_amount += (mineralised_amount * bulk_density * thisVolume); //µgN                
                        scoreboard->SetCharacteristicValue(Nitrate_Amount_Indices[idx], nitrate_amount, box_index);

                        SUBSECTION_ITER_WITH_PROPORTIONS_END
                    }//if(box_bottom <= mineralisation_depth)
                } //for (z=1; z<=NUM_Z; ++z)
            } //for (x=1; x<=NUM_X; ++x)
        } //for (y=1; y<=NUM_Y; ++y)    
    } //Nitrate::CalculateMineralisation


    /* ******************************************************************************* */

    //VMD 4/10/98 Separated this dot redrawing routine from nitrate uptake.  Both N diffusion and N uptake result in a change in the
    // nitrate concentration in boxes hence requiring the dots to be redrawn on the screen.  Separating this routine out allows any
    // routine to call a dot redraw.
    //VMD 2/11/99 This routine as it stands can only be used for those routines that change the nitrate concentration of a box and hence
    // the number of dots needs to be adjusted - ie N diffusion and N uptake.  This routine cannot be used for MassFlow and Leach which change the distribution
    // of the dots and need a resultant recalculation of the nitrate concentration in each scoreboard box.  I have added some more comments to this
    // routine.
    //VMD 12/07/02, 19/07/02 Have done some recoding to account for the new iter structure of (s,x,y,z).  Now track not only nitrates of whole packet size,
    // but also nitrates of a lesser size that were the left over.
    //MSA 10.11.15 Updated this method to work on a per-Box-section basis. Each Scoreboard Box is made up of one or more sections which are coincident in space
    // with a VolumeObject or no VolumeObject. In the minimal case, all boxes are made up of one section each which is not coincident with any VO (because no VOs are present).

    long int Nitrate::RedistributeDots(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_Time;
        Use_Scoreboard;
        Use_ScoreboardCoordinator;
        Use_VolumeObjectCoordinator;
        Use_SharedAttributeManager;
        // MSA 11.04.28 Replaces Use_Boundaries;
        const BoundaryArray& X_boundaries = m_boundarySet->X_boundaries;
        const BoundaryArray& Y_boundaries = m_boundarySet->Y_boundaries;
        const BoundaryArray& Z_boundaries = m_boundarySet->Z_boundaries;

        // MSA 11.02.03 Converted this method to use vectors instead of arrays

        const long totalSize = NUM_X * NUM_Y * NUM_Z * (VO_COUNT + 1);

        std::vector<BoxIndex> wholePacketCountVector(totalSize);
        std::vector<BoxIndex> partPacketCountVector(totalSize);

        long totalWholePacketCount = 0;
        long totalPartPacketCount = 0;

        // MSA 11.05.02 Currently unused.
        //double total_nitrate_amount = 0;
        size_t i = 0;
        for (CharacteristicIndices::const_iterator iter = Nitrate_Amount_Indices.begin(); iter != Nitrate_Amount_Indices.end(); ++iter)
        {
            for (BoxIndex bi = 0; bi < BOX_COUNT; ++bi)
            {
                const double nitrate_amount = Get_Concentration_Of_BoxIndex((*iter), bi);
                scoreboard->SetCharacteristicValue(Nitrate_AmountCheck_Indices[i], nitrate_amount, bi);
                //total_nitrate_amount += nitrate_amount;
            }
            ++i;
        }

        //  This routine counts up how many nitrate packets are in each box section
        const NitrateIterator theEnd = m_nitrateList.end();
        for (NitrateIterator iter = m_nitrateList.begin(); iter != theEnd; ++iter)
        {
            // Need to count how many nitrate packets are in each box section.  To do this every box section in the scoreboard is given a number
            // which corresponds to the index in an array. The number of the first box section is zero and the order of numbering is ZXY.
            // Within each box, the order of numbering is first the null VO (i.e. the section of the box coincident with no VOs), 
            // followed by the order of VolumeObjects in the VO Coordinator.

            // MSA We don't want to have to re-determine which box this Nitrate is in every time, only when it's moved.
            BoxIndex box_index;
            size_t idx;
            if (iter->containingBoxSubsectionKnown)
            {
                box_index = iter->containingBox;
                idx = iter->containingSubsection;
            }
            else
            {
                BoxCoordinate bc;
                BoxCoordinate* box_coords = &bc;
                scoreboardcoordinator->FindBoxBySoilCoordinate(box_coords, scoreboard, &(*iter));
                box_index = scoreboard->GetBoxIndex(box_coords->x, box_coords->y, box_coords->z);
                // MSA 10.11.11 Must determine whether this Nitrate packet is constrained by a VO.
                idx = volumeobjectcoordinator->GetContainingVolumeObjectIndex(*iter);
                iter->SetContainingBoxSubsection(box_index, idx);
            }

            const size_t boxSubsectionIndex = (VO_COUNT + 1) * box_index + idx;

            if (iter->s == m_NITRATE_PACKET_SIZE)
            {
                wholePacketCountVector[boxSubsectionIndex]++;
                ++totalWholePacketCount;
                //new_number_whole_nitrate_packets = wholePacketCountArray[index];
            }
            else //if (iter -> s != m_NITRATE_PACKET_SIZE)
            {
                partPacketCountVector[boxSubsectionIndex]++;
                ++totalPartPacketCount;
                //new_number_whole_nitrate_packets = partPacketCountArray[index];
            }
        } // for(NitrateIterator iter = m_nitrateList.begin(); iter!=theEnd; ++iter)


        //If the number of nitrate packets in any particular scoreboard box section has changed then dots need to be removed from or added to the screen.
        // IMPORTANT! Using a while loop here, because we may be removing packets.
        // Using std::list::erase invalidates iterators to positions at and after the erased iterator(s),
        // so a straightforward ++iter is not always legal.
        NitrateIterator removalIterator = m_nitrateList.begin();
        while (removalIterator != m_nitrateList.end())
        {
            // MSA We don't want to have to re-determine which box this Nitrate is in every time, only when it's moved.
            BoxIndex box_index;
            size_t idx;
            if (removalIterator->containingBoxSubsectionKnown)
            {
                box_index = removalIterator->containingBox;
                idx = removalIterator->containingSubsection;
            }
            else
            {
                BoxCoordinate bc;
                BoxCoordinate* box_coords = &bc;
                scoreboardcoordinator->FindBoxBySoilCoordinate(box_coords, scoreboard, &(*removalIterator));
                box_index = scoreboard->GetBoxIndex(box_coords->x, box_coords->y, box_coords->z);
                // MSA 10.11.11 Must determine whether this Nitrate packet is constrained by a VO.
                idx = volumeobjectcoordinator->GetContainingVolumeObjectIndex(*removalIterator);
                removalIterator->SetContainingBoxSubsection(box_index, idx);
            }

            const size_t boxSubsectionIndex = (VO_COUNT + 1) * box_index + idx;

            double nitrate_amountcheck = scoreboard->GetCharacteristicValue(Nitrate_AmountCheck_Indices[idx], box_index);
            const double nitrate_amount = scoreboard->GetCharacteristicValue(Nitrate_Amount_Indices[idx], box_index);
            const long thisBoxSectionWholePacketCount = nitrate_amount / m_NITRATE_PACKET_SIZE; //integer based on the current amount of nitrate

            // Check if nitrate count that used to be in this box exceeds the current count
            if (wholePacketCountVector[boxSubsectionIndex] > thisBoxSectionWholePacketCount && removalIterator->s == m_NITRATE_PACKET_SIZE)//too many whole packets. delete one
            {
                wholePacketCountVector[boxSubsectionIndex]--;
                removalIterator = m_nitrateList.erase(removalIterator); // Erase this nitrate packet. erase() returns an iterator to the next valid packet.
                continue; // Finished here; go to next nitrate packet.
            }

            const long thisBoxSectionPartPacketCount = partPacketCountVector[boxSubsectionIndex]; //part nitrate dots in this box
            if (thisBoxSectionPartPacketCount > 1 && removalIterator->s < m_NITRATE_PACKET_SIZE)//too many part packets delete one
            {
                LOG_ERROR_IF(removalIterator->s == 0.0);
                partPacketCountVector[boxSubsectionIndex]--;
                removalIterator = m_nitrateList.erase(removalIterator); // Erase this nitrate packet. erase() returns an iterator to the next valid packet.
                continue; // Finished here; go to next nitrate.
            }

            if (removalIterator->s == m_NITRATE_PACKET_SIZE)
            {
                if (nitrate_amountcheck < m_NITRATE_PACKET_SIZE)
                {
                    removalIterator->s = nitrate_amountcheck;
                    nitrate_amountcheck = 0.0;
                    if (removalIterator->s == 0.0) // If this nitrate has s==0, wipe it
                    {
                        removalIterator = m_nitrateList.erase(removalIterator);
                        continue; // Finished here; go to next nitrate.
                    }
                }
                else
                {
                    nitrate_amountcheck -= m_NITRATE_PACKET_SIZE;
                }
                scoreboard->SetCharacteristicValue(Nitrate_AmountCheck_Indices[idx], nitrate_amountcheck, box_index);
                ++removalIterator; // Finished here; go to next nitrate.
                continue;
            } //if(removalIterator->s == m_NITRATE_PACKET_SIZE)

            /* "else" */
            if (nitrate_amountcheck == 0.0)
            {
                removalIterator = m_nitrateList.erase(removalIterator); // Erase this nitrate packet. erase() returns an iterator to the next valid packet.
                continue; // Finished here; go to next nitrate.
            }
            // else
            const double thisBoxSectionRemainder = (nitrate_amount)-(thisBoxSectionWholePacketCount * m_NITRATE_PACKET_SIZE); //amount left over to go into a part packet
            nitrate_amountcheck -= thisBoxSectionRemainder;
            scoreboard->SetCharacteristicValue(Nitrate_AmountCheck_Indices[idx], nitrate_amountcheck, box_index);
            removalIterator->s = thisBoxSectionRemainder;
            if (removalIterator->s >= m_NITRATE_PACKET_SIZE)
            {
                Debugger();
            }

            LOG_ERROR_IF(nitrate_amount < 0.0);

            // MSA 09.10.30 If we get to here in the loop, it's safe to simply increment the iterator (nothing has been erased) to go to the next packet.
            ++removalIterator;
        } // while(removalIterator!=m_nitrateList.end())

        //now add new packets as necessary

        for (long y = 1; y <= NUM_Y; ++y)
        {
            for (long x = 1; x <= NUM_X; ++x)
            {
                for (long z = 1; z <= NUM_Z; ++z)
                {
                    const BoxIndex box_index = scoreboard->GetBoxIndex(x, y, z);

                    // While we're here, zero out the background and fertiliser N proportions, for later recalculation.
                    for (CharacteristicIndices::const_iterator iter = BackgroundN_Indices.begin(); iter != BackgroundN_Indices.end(); ++iter)
                    {
                        scoreboard->SetCharacteristicValue(*iter, 0.0, box_index);
                    }
                    for (CharacteristicIndices::const_iterator iter = FertiliserN_Indices.begin(); iter != FertiliserN_Indices.end(); ++iter)
                    {
                        scoreboard->SetCharacteristicValue(*iter, 0.0, box_index);
                    }

                    SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                        double nitrate_amountcheck = scoreboard->GetCharacteristicValue(Nitrate_AmountCheck_Indices[idx], box_index);

                    // MSA 11.02.14 Modifying this conditional to require a more-than-trivial nitrate amount.
                    // This is because, otherwise, we sometimes get stuck repeatedly attempting to assign nitrate packets to 
                    // an almost-zero volume (and the algorithm used just tries to randomly place the packet within its Box,
                    // so repeated iterations of the do loop don't necessarily get closer to an acceptable result)

                    while (nitrate_amountcheck > 0.01) // Add 1 or more new nitrate packets to use up nitrate_amountcheck
                    {
                        RmAssert(thisProportion > 0.0, "Error: logically zero volume seems to contain nitrates");

                        const double box_left = Get_Layer_Left(x);
                        const double box_front = Get_Layer_Front(y);
                        const double box_top = Get_Layer_Top(z);
                        const double box_width = Get_Layer_Width(x);
                        const double box_depth = Get_Layer_Depth(y);
                        const double box_height = Get_Layer_Height(z);

                        const DoubleBox doubleBox(box_left, box_front, box_top, box_left + box_width, box_front + box_depth, box_top + box_height);

                        DoubleCoordinate dc;
                        // 09.10.28 MSA Generate a new pseudorandom nitrate position within the given box coordinates; handle for edge cases 
                        // 10.11.15 MSA Ensure the new Nitrate is in the correct section of the box
                        // MSA 11.02.15 Using a new VolumeObject function to improve the efficiency of assigning the Nitrate a position.
                        bool positionOK = false;

                        while (!positionOK)
                        {
                            positionOK = true;
                            if (vo == __nullptr)
                            {
                                bool first = true;
                                for (VolumeObjectList::const_iterator vi = volumeobjectcoordinator->GetVolumeObjectList().begin(); vi != volumeobjectcoordinator->GetVolumeObjectList().end(); ++vi)
                                {
                                    if (first)
                                    {
                                        // Create the point
                                        dc = (*vi)->GenerateRandomPointInBox(doubleBox, false);
                                        first = false;
                                    }
                                    else
                                    {
                                        // Ensure it's not inside any other VO
                                        positionOK = !(*vi)->Contains(dc);
                                    }
                                }
                            }
                            else
                            {
                                dc = vo->GenerateRandomPointInBox(doubleBox, true);
                            }
                        }

                        // Finally, create the DCN from the DC
                        DoubleCoordinateNitrate dcn(dc, Random1() < m_NITRATE_DRAW_PROPORTION);

                        if (nitrate_amountcheck >= m_NITRATE_PACKET_SIZE)
                        {
                            dcn.s = m_NITRATE_PACKET_SIZE; //we are making a new whole packet
                            nitrate_amountcheck -= m_NITRATE_PACKET_SIZE;
                        } //if(nitrate_amountcheck >= m_NITRATE_PACKET_SIZE)
                        else //Will add a part nitrate to the box.    
                        {
                            dcn.s = nitrate_amountcheck; //we are making a new part packet
                            nitrate_amountcheck = 0.0;
                        }//else                                

                        // set the array value
                        m_nitrateList.push_back(dcn);
                    } //while(nitrate_amountcheck > 0.0)        

                    scoreboard->SetCharacteristicValue(Nitrate_AmountCheck_Indices[idx], nitrate_amountcheck, box_index);

                    SUBSECTION_ITER_WITH_PROPORTIONS_END
                } //for z
            } //for x
        } //for y

        //Check that we have the right proportion of fertiliser to background N dots in each box and adjust if necessary
        const NitrateIterator theNewEnd = m_nitrateList.end();
        for (NitrateIterator iter = m_nitrateList.begin(); iter != theNewEnd; ++iter)
        {
            // MSA We don't want to have to re-determine which box this Nitrate is in every time, only when it's moved.
            BoxIndex box_index;
            size_t idx;
            if (iter->containingBoxSubsectionKnown)
            {
                box_index = iter->containingBox;
                idx = iter->containingSubsection;
            }
            else
            {
                BoxCoordinate bc;
                BoxCoordinate* box_coords = &bc;
                scoreboardcoordinator->FindBoxBySoilCoordinate(box_coords, scoreboard, &(*iter));
                box_index = scoreboard->GetBoxIndex(box_coords->x, box_coords->y, box_coords->z);
                // MSA 10.11.11 Must determine whether this Nitrate packet is constrained by a VO.
                idx = volumeobjectcoordinator->GetContainingVolumeObjectIndex(*iter);
                iter->SetContainingBoxSubsection(box_index, idx);
            }

            // AddCharacteristicValue is essentially "+= for ScoreboardBoxes"
            if (iter->f)
            {
                scoreboard->AddCharacteristicValue(FertiliserN_Indices[idx], iter->s, box_index);
            }
            else
            {
                scoreboard->AddCharacteristicValue(BackgroundN_Indices[idx], iter->s, box_index);
            }
        } // for(NitrateIterator iter = m_nitrateList.begin(); iter!=theNewEnd; ++iter)

        // Get the SharedAttributeList corresponding to the first Plant
        // MSA 09.10.27 Currently treating Plants as the same for the purposes of these SharedAttributes
        //        --> OK to just use first plant here. 
        const SharedAttributeCluster& cluster = Get_Cluster("Plant");
        SharedAttributeCluster::const_iterator cluster_iter = cluster.begin();
        const SharedAttributeList& sa_list = (*cluster_iter).second;

        SharedAttribute* seeding_time_sa = sa_list[saSeedingTimeIndex]; //Time from zero (start of simulation) in hours when the seeds are sown
        const ProcessTime_t time_of_seeding = static_cast<ProcessTime_t>(PROCESS_HOURS(seeding_time_sa->GetValue())); //converting the seeding time from hours to seconds
        SharedAttribute* Add_FertN_sa = sa_list[saAddFertNIndex];

        const ProcessTime_t nextFertiliserAddTime = static_cast<ProcessTime_t>(PROCESS_HOURS(Add_FertN_sa->GetValue())); //time of second addition of N fertiliser

        //VMD At this stage time does not start from 0/0/0 0:0:0, but from a specified date, so need to subtract the start time to
        //get the time since the start of the simulation.
        const ProcessTime_t current_time = time - action->GetPostOffice()->GetStart();


        //can have a split application of N, 1 at sowing and 1 later in the season
        const bool doApplication = (current_time > time_of_seeding) || (current_time > nextFertiliserAddTime && nextFertiliserAddTime > 0);
        if (!doApplication) { return return_value; }

        for (long y = 1; y <= NUM_Y; ++y) //now add any new amounts to the zeros
        {
            for (long x = 1; x <= NUM_X; ++x)
            {
                for (long z = 1; z <= NUM_Z; ++z)
                {
                    const BoxIndex box_index = scoreboard->GetBoxIndex(x, y, z);
                    const double box_front = Get_Layer_Front(y);
                    const double box_depth = Get_Layer_Depth(y);
                    const double box_left = Get_Layer_Left(x);
                    const double box_width = Get_Layer_Width(x);
                    const double box_height = Get_Layer_Height(z);
                    const double box_top = Get_Layer_Top(z);

                    const DoubleBox doubleBox(box_left, box_front, box_top, box_left + box_width, box_front + box_depth, box_top + box_height);

                    SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                        double backgroundN = scoreboard->GetCharacteristicValue(BackgroundN_Indices[idx], box_index);//µgN
                    double fertiliserN = scoreboard->GetCharacteristicValue(FertiliserN_Indices[idx], box_index);//µgN
                    double fertFracN = scoreboard->GetCharacteristicValue(FertFracN_Indices[idx], box_index);

                    // MSA 11.02.11 "Added N" is specified as being initially entered into NitrateAmount.XML in µgN/g.
                    // This is the only place where Added_N_Indices are referenced.
                    // Thus, we clearly need to convert added_N and added_N2 from Nitrate Content to Nitrate Amounts here, right?

                    double added_N = scoreboard->GetCharacteristicValue(Added_N_Indices[idx], box_index); //µgN/g, fertiliser added at crop sowing
                    double added_N2 = scoreboard->GetCharacteristicValue(Added_N2_Indices[idx], box_index); //µgN/g, fertilser added later in the season
                    const double conversionToAmountTerm = thisProportion * Get_BoxIndex_Volume(box_index) * scoreboard->GetCharacteristicValue(Bulk_Density_Indices[idx], box_index);
                    added_N *= conversionToAmountTerm; //µgN
                    added_N2 *= conversionToAmountTerm; //µgN

                    //Crop has just been sown and fertiliser added
                    while (added_N > 0.0)
                    {
                        fertiliserN = added_N;

                        const double nitrate_amount = scoreboard->GetCharacteristicValue(Nitrate_Amount_Indices[idx], box_index);
                        fertFracN = added_N / (nitrate_amount + added_N); //fraction of N in the box that is from fertiliser
                        scoreboard->SetCharacteristicValue(Nitrate_Amount_Indices[idx], (nitrate_amount + added_N), box_index);
                        scoreboard->SetCharacteristicValue(FertFracN_Indices[idx], fertFracN, box_index);
                        DoubleCoordinate dc;
                        // 09.10.28 MSA Generate a new pseudorandom nitrate position within the given box coordinates; handle for edge cases 
                        // 10.11.15 MSA Ensure the new Nitrate is in the correct section of the box
                        // MSA 11.02.15 Using a new VolumeObject function to improve the efficiency of assigning the Nitrate a position.
                        bool positionOK = false;

                        while (!positionOK)
                        {
                            positionOK = true;
                            if (vo == __nullptr)
                            {
                                bool first = true;
                                for (VolumeObjectList::const_iterator vi = volumeobjectcoordinator->GetVolumeObjectList().begin(); vi != volumeobjectcoordinator->GetVolumeObjectList().end(); ++vi)
                                {
                                    if (first)
                                    {
                                        // Create the point
                                        dc = (*vi)->GenerateRandomPointInBox(doubleBox, false);
                                        first = false;
                                    }
                                    else
                                    {
                                        // Ensure it's not inside any other VO
                                        positionOK = !(*vi)->Contains(dc);
                                    }
                                }
                            }
                            else
                            {
                                dc = vo->GenerateRandomPointInBox(doubleBox, true);
                            }
                        }

                        // Finally, create the DCN from the DC
                        DoubleCoordinateNitrate dcn(dc, Random1() < m_NITRATE_DRAW_PROPORTION);

                        if (added_N >= m_NITRATE_PACKET_SIZE)
                        {
                            dcn.s = m_NITRATE_PACKET_SIZE; //we are making a new whole packet
                            dcn.f = true; // true --> it is from fertiliser
                            added_N -= m_NITRATE_PACKET_SIZE;
                        } //if(added_N >= m_NITRATE_PACKET_SIZE)
                        else //Will add a part nitrate to the box.    
                        {
                            dcn.s = added_N; //we are making a new part packet
                            dcn.f = true;
                            added_N = 0.0;
                        }//else                                

                        scoreboard->SetCharacteristicValue(Added_N_Indices[idx], added_N, box_index);

                        // set the array value
                        m_nitrateList.push_back(dcn);
                    } //while(added_N > 0.0)

                    //A second application of N fertiliser has been made
                    while ((added_N2 > 0.0) && (current_time > nextFertiliserAddTime) && (nextFertiliserAddTime > 0))
                    {
                        fertiliserN += added_N2;

                        const double nitrate_amount = scoreboard->GetCharacteristicValue(Nitrate_Amount_Indices[idx], box_index);
                        fertFracN = (fertiliserN) / (backgroundN + fertiliserN); //fraction of N in the box that is from fertiliser
                        scoreboard->SetCharacteristicValue(Nitrate_Amount_Indices[idx], (nitrate_amount + added_N2), box_index);
                        scoreboard->SetCharacteristicValue(FertFracN_Indices[idx], fertFracN, box_index);

                        DoubleCoordinate dc;
                        // 09.10.28 MSA Generate a new pseudorandom nitrate position within the given box coordinates; handle for edge cases 
                        // 10.11.15 MSA Ensure the new Nitrate is in the correct section of the box
                        // MSA 11.02.15 Using a new VolumeObject function to improve the efficiency of assigning the Nitrate a position.
                        bool positionOK = false;

                        while (!positionOK)
                        {
                            positionOK = true;
                            if (vo == __nullptr)
                            {
                                bool first = true;
                                for (VolumeObjectList::const_iterator vi = volumeobjectcoordinator->GetVolumeObjectList().begin(); vi != volumeobjectcoordinator->GetVolumeObjectList().end(); ++vi)
                                {
                                    if (first)
                                    {
                                        // Create the point
                                        dc = (*vi)->GenerateRandomPointInBox(doubleBox, false);
                                        first = false;
                                    }
                                    else
                                    {
                                        // Ensure it's not inside any other VO
                                        positionOK = !(*vi)->Contains(dc);
                                    }
                                }
                            }
                            else
                            {
                                dc = vo->GenerateRandomPointInBox(doubleBox, true);
                            }
                        }

                        // Finally, create the DCN from the DC
                        DoubleCoordinateNitrate dcn(dc, Random1() < m_NITRATE_DRAW_PROPORTION);

                        if (added_N2 >= m_NITRATE_PACKET_SIZE)
                        {
                            dcn.s = m_NITRATE_PACKET_SIZE; //we are making a new whole packet
                            dcn.f = true; // true --> it is from fertiliser
                            added_N2 -= m_NITRATE_PACKET_SIZE;
                        } //if(added_N >= m_NITRATE_PACKET_SIZE)
                        else //Will add a part nitrate to the box.    
                        {
                            dcn.s = added_N2; //we are making a new part packet
                            dcn.f = true;
                            added_N2 = 0.0;
                        }//else                                

                        scoreboard->SetCharacteristicValue(Added_N2_Indices[idx], added_N2, box_index);

                        // set the array value
                        m_nitrateList.push_back(dcn);
                    } //while(added_N2 > 0)

                    double dotsFertFrac;
                    if ((0.0 == fertiliserN && 0.0 == backgroundN) || (fertiliserN + backgroundN == 0.0)) { dotsFertFrac = 0.0; }
                    else { dotsFertFrac = fertiliserN / (fertiliserN + backgroundN); }

                    double deltaFert;
                    if (dotsFertFrac > fertFracN) { deltaFert = dotsFertFrac - fertFracN; }
                    else { deltaFert = fertFracN - dotsFertFrac; }

                    if (deltaFert <= 0.01)
                    {
                        continue; // Not enough deltaFert. Go to next box section.
                    }

                    // MSA 11.04.28 Use this BoxIndex for a simpler comparison operation.
                    const BoxIndex boxIndex = scoreboard->GetBoxIndex(x, y, z);

                    // (else) we need to change some dots from fertiliser type to background type or vice versa
                    for (NitrateIterator nIter = m_nitrateList.begin(); nIter != m_nitrateList.end(); ++nIter)
                    {
                        // Check if this Nitrate is confirmed to be in this Box.
                        if (nIter->containingBoxSubsectionKnown)
                        {
                            if (nIter->containingBox != boxIndex) continue; // Box indices do not match. Go to next nitrate packet.
                        }
                        else
                        {
                            BoxCoordinate bc;
                            BoxCoordinate* box_coords = &bc;
                            scoreboardcoordinator->FindBoxBySoilCoordinate(box_coords, scoreboard, &(*nIter));
                            const BoxIndex containingBox = scoreboard->GetBoxIndex(box_coords->x, box_coords->y, box_coords->z);
                            // While we know which Box contains this packet, we might as well tell the packet itself
                            const size_t voIndex = volumeobjectcoordinator->GetContainingVolumeObjectIndex(*nIter);
                            nIter->SetContainingBoxSubsection(containingBox, voIndex);
                            if (containingBox != boxIndex) continue; // Box indices do not match. Go to next nitrate packet.
                        }

                        // (finally) found a packet, is it the right one to change?
                        bool fromBackgroundToFertiliser = dotsFertFrac <= fertFracN; //need to change dots from background to fertiliser?

                        if (nIter->f != fromBackgroundToFertiliser // is this packet OK to change? 
                            && (deltaFert * (fertiliserN + backgroundN) > m_NITRATE_PACKET_SIZE) && (nIter->s == m_NITRATE_PACKET_SIZE)) // ...and is it full-sized?
                        {
                            nIter->f = fromBackgroundToFertiliser;
                            fertiliserN -= m_NITRATE_PACKET_SIZE;
                            backgroundN += m_NITRATE_PACKET_SIZE;
                            dotsFertFrac = (fertiliserN) / (fertiliserN + backgroundN);
                            //if(dotsFertFrac == 1.0){Debugger();}
                            //if(!((dotsFertFrac <=1)&&(dotsFertFrac >=0.0))){Debugger();}
                            if (dotsFertFrac > fertFracN) { deltaFert = dotsFertFrac - fertFracN; }
                            else { deltaFert = fertFracN - dotsFertFrac; }
                        }
                        if (nIter->f != fromBackgroundToFertiliser // is this packet OK to change? 
                            && (deltaFert * (fertiliserN + backgroundN) < m_NITRATE_PACKET_SIZE) && (nIter->s < m_NITRATE_PACKET_SIZE)) // ...and is it partial?
                        {
                            nIter->f = fromBackgroundToFertiliser;
                            // We're done changing.
                            break;
                        }
                    } // for(NitrateIterator nIter = m_nitrateList.begin(); nIter!=m_nitrateList.end(); ++nIter)

                    SUBSECTION_ITER_WITH_PROPORTIONS_END
                } //for z
            } //for x
        } //for y

        // Redraw the scoreboard to reflect the changes made by this routine.
        DrawScoreboard(DoubleRect(), __nullptr);

        return (return_value);
    } // void Nitrate::RedistributeDots(ProcessActionDescriptor * action)


    bool Nitrate::DoesOverride() const
    {
        return (true);
    }

    bool Nitrate::DoesDrawing() const
    {
        return (true);
    }

    bool Nitrate::DoesRaytracerOutput() const
    {
        return (true);
    }

    /**
     *  09.10.20 MSA Private helper function for recalculating Nitrate amount in each ScoreboardBox
     */
    void Nitrate::RecalculateNitrateAmounts(Scoreboard* scoreboard, ScoreboardCoordinator* scoreboardcoordinator, VolumeObjectCoordinator* volumeobjectcoordinator)
    {
        const size_t VO_COUNT = volumeobjectcoordinator->GetVolumeObjectList().size();

        // MSA 09.10.20 Updated this to use rangewise resetting of all Nitrate_Amount and FertiliserN values to 0.0
        for (CharacteristicIndices::const_iterator citer = Nitrate_Amount_Indices.begin(); citer != Nitrate_Amount_Indices.end(); ++citer)
        {
            scoreboard->SetCharacteristicValueRange(*citer, 0.0, 0, BOX_COUNT);
        }
        for (CharacteristicIndices::const_iterator citer = FertiliserN_Indices.begin(); citer != FertiliserN_Indices.end(); ++citer)
        {
            scoreboard->SetCharacteristicValueRange(*citer, 0.0, 0, BOX_COUNT);
        }

        for (NitrateIterator iter = m_nitrateList.begin(); iter != m_nitrateList.end(); ++iter)
        {
            // MSA We don't want to have to re-determine which box this Nitrate is in every time, only when it's moved.
            BoxIndex box_index;
            size_t idx;
            if (iter->containingBoxSubsectionKnown)
            {
                box_index = iter->containingBox;
                idx = iter->containingSubsection;
            }
            else
            {
                BoxCoordinate bc;
                BoxCoordinate* box_coords = &bc;
                scoreboardcoordinator->FindBoxBySoilCoordinate(box_coords, scoreboard, &(*iter));
                box_index = scoreboard->GetBoxIndex(box_coords->x, box_coords->y, box_coords->z);
                // MSA 10.11.11 Must determine whether this Nitrate packet is constrained by a VO.
                idx = volumeobjectcoordinator->GetContainingVolumeObjectIndex(*iter);
                iter->SetContainingBoxSubsection(box_index, idx);
            }

            double nitrate_amount = scoreboard->GetCharacteristicValue(Nitrate_Amount_Indices[idx], box_index);
            nitrate_amount += iter->s;
            scoreboard->SetCharacteristicValue(Nitrate_Amount_Indices[idx], nitrate_amount, box_index);

            const double water_content = scoreboard->GetCharacteristicValue(Water_Content_Indices[idx], box_index);

            const double thisVolume = Get_BoxIndex_Volume(box_index) * scoreboard->GetCoincidentProportion(box_index, idx);

            scoreboard->SetCharacteristicValue(Nitrate_Concentration_Indices[idx], (nitrate_amount / (thisVolume * water_content)), box_index);

            if (iter->f) // Is this nitrate from fertiliser?
            {
                // AddCharacteristicValue is essentially "+= for ScoreboardBoxes"
                scoreboard->AddCharacteristicValue(FertiliserN_Indices[idx], iter->s, box_index);
            }
        }// for(NitrateIterator iter = m_nitrateList.begin(); iter!=m_nitrateList.end(); ++iter)

        for (BoxIndex box_index = 0; !m_IS_NUTRIENT_SOLUTION && box_index < BOX_COUNT; ++box_index)
        {
            SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                const double nitrate_amount = scoreboard->GetCharacteristicValue(Nitrate_Amount_Indices[idx], box_index);
            if (nitrate_amount <= 0.0)
            {
                scoreboard->SetCharacteristicValue(FertFracN_Indices[idx], 0.0, box_index);
            }
            else
            {
                const double newFFN = scoreboard->GetCharacteristicValue(FertiliserN_Indices[idx], box_index) / nitrate_amount;
                scoreboard->SetCharacteristicValue(FertFracN_Indices[idx], newFFN, box_index);
            }

            SUBSECTION_ITER_WITH_PROPORTIONS_END
        }//for(BoxIndex bi=0; bi<numBoxes; ++bi)
    }
} /* namespace rootmap */

