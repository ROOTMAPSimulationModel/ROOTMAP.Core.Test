/////////////////////////////////////////////////////////////////////////////
// Name:        ClassTemplate.cpp
// Purpose:     Implementation of the ClassTemplate class
// Created:     DD/06/1997
// Substantially modified September 2002 by: Vanessa Dunbabin
// Author:      Vanessa Dunbabin
// $Date: August 2010 VMD$
// $Revision: 43 $
// Copyright:   ©2006 University of Tasmania, Dr Vanessa Dunbabin, Centre for Legumes in Meditteranean Agriculture, Grains Research and Development Corporation, Department of Agriculture and Food Western Australia
//
// This routine parallels Nitrate, considering some phosphorus transformations: uptake, leaching
// P addition, P balance between the labile and in-solution pools, P fixation.
//
//
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/common/Process.h"
#include "simulation/process/modules/Phosphorus.h"
#include "simulation/process/modules/WaterNotification.h"
#include "simulation/process/plant/PlantSummaryRegistration.h"
#include "simulation/process/plant/PlantSummaryHelper.h"
#include "simulation/process/modules/VolumeObject.h"
#include "simulation/process/modules/VolumeObjectCoordinator.h"

#include "simulation/process/shared_attributes/ProcessSharedAttributeOwner.h"
#include "simulation/process/shared_attributes/ProcessAttributeSupplier.h"
#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"
#include "simulation/process/shared_attributes/SharedAttributeManager.h"
#include "simulation/process/shared_attributes/SharedAttributeSearchHelper.h"
#include "simulation/process/shared_attributes/SharedAttribute.h"
#include "simulation/process/common/ProcessUtility.h"
#include "simulation/data_access/tinyxmlconfig/TXCharacteristicDA.h"
#include "simulation/data_access/interface/ProcessDAI.h"
#include "simulation/data_access/tinyxmlconfig/TXProcessDA.h"

#include "core/scoreboard/BoundaryArray.h"
#include "core/utility/Utility.h"
#include "core/common/RmAssert.h"


#define DEBUGGER(s) if ( (s) ) { RootMapLogError(LOG_LINE << "Encountered error in logic :" << #s); }
#define LOG_ERROR_IF(s) if ( (s) ) { RootMapLogError(LOG_LINE "Encountered error in logic : " #s); }

namespace rootmap
{
    RootMapLoggerDefinition(Phosphorus);
    IMPLEMENT_DYNAMIC_CLASS(Phosphorus, Process)

        /* ***************************************************************************
        */
        Phosphorus::Phosphorus()
        : m_time_between_wakings(PROCESS_HOURS(4))
        , m_time_prev_waking(-1)
        , phosphorus_first_plant_summary_index(-1)
        , m_phosphorus_number_of_plants(0)
        , m_phosphorus_number_of_branch_orders(0)
        , m_time_of_previous_diffusion(-1)
        , m_time_of_prev_solution_change(-1)
        , m_diffusion_rotation_toggle(false)
        , mySharedAttributeOwner(0)
        , m_DIFFUSION_COEFF_SOLN(0.0000089) //diffusion coefficient in free solution = 0.89e-5 cm2/sec for H2PO4- in free solution
        , m_IS_NUTRIENT_SOLUTION(false)
        , m_MODEL_ROOT_HAIRS(false)
        , m_volumeObjectCoordinator(__nullptr)
        , m_boundarySet(__nullptr)
    {
        RootMapLoggerInitialisation("rootmap.Phosphorus");
        mySharedAttributeOwner = new ProcessSharedAttributeOwner("Phosphorus", this);
        SharedAttributeRegistrar::RegisterOwner(mySharedAttributeOwner);

        ProcessAttributeSupplier* supplier = new ProcessAttributeSupplier(this, "Phosphorus");
        SharedAttributeRegistrar::RegisterSupplier(supplier);
    }

    Phosphorus::~Phosphorus()
    {
        delete m_boundarySet;
    }

    const double Phosphorus::Mr = 30.974;

    bool Phosphorus::DoesOverride() const { return (true); }

    void Phosphorus::SetVolumeObjectCoordinator(const VolumeObjectCoordinator& voc)
    {
        m_volumeObjectCoordinator = &voc;
    }

    /* ***************************************************************************
    */
    long int Phosphorus::Register(ProcessActionDescriptor* action)
    {
        Use_ProcessCoordinator;
        PlantSummaryHelper helper(processcoordinator, 0);
        PlantSummaryRegistration* psr = helper.MakePlantSummaryRegistration(action);

        psr->RegisterByPerBox(true);
        psr->RegisterAllPlantCombos();
        psr->RegisterAllBranchOrderCombos();
        psr->RegisterSummaryType(srRootLength);
        psr->RegisterModule((Process*)this);
        helper.SendPlantSummaryRegistration(psr, action, this);

        return kNoError;
    }

    void Phosphorus::Initialise(const ProcessDAI& data)
    {
        if (m_volumeObjectCoordinator != __nullptr)
        {
            // MSA 10.12.22 List the Characteristics NOT affected by barrier modelling here; exclude from the makeVariants process.
            std::vector<std::string> invariantCharacteristicNames;
            invariantCharacteristicNames.push_back("Direction Moved");
            invariantCharacteristicNames.push_back("Plus X");
            invariantCharacteristicNames.push_back("Minus X");
            invariantCharacteristicNames.push_back("Plus Y");
            invariantCharacteristicNames.push_back("Minus Y");
            invariantCharacteristicNames.push_back("Plus Z");
            invariantCharacteristicNames.push_back("Minus Z");
            ProcessDAI* ptr = m_volumeObjectCoordinator->AccomodateVolumeObjects(data, invariantCharacteristicNames);
            Process::Initialise(*ptr);
            delete ptr;
        }
        else
        {
            Process::Initialise(data);
        }
    }

    /* ***************************************************************************
    Initialise
    Called before the user sees or does anything. We will use this moment
    to ask the PlantCoordinator to create and set aside some per-box, per-plant,
    per-branch-order plant summaries. We also need to set some regular
    actions in motion */
    long int Phosphorus::Initialise(ProcessActionDescriptor* action)
    {
        Use_Time;
        Use_ProcessCoordinator;
        Use_SharedAttributeManager;
        Use_PostOffice;
        Use_Scoreboard;
        Use_VolumeObjectCoordinator;

        Use_ScoreboardCoordinator;
        Use_Boundaries;
        m_boundarySet = new BoundarySet(X_boundaries, Y_boundaries, Z_boundaries);

        // Set the "alarm clock"
        SetPeriodicDelay(kNormalMessage, m_time_between_wakings);
        Send_GeneralPurpose(time + m_time_between_wakings, this, __nullptr, kNormalMessage);

        // MSA 09.11.09 Determine the Scoreboard dimensions here, save doing it repeatedly
        Get_Number_Of_Layers(NUM_X, NUM_Y, NUM_Z); // number of layers in each dimension
        BOX_COUNT = scoreboard->GetNumberOfBoxes(); // How many boxes do we have?

        //Indices for local variables

        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Liquid_Phase_P_Conc_Indices, "Liquid Phase P Concentration");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Liquid_Phase_P_Indices, "Liquid Phase P");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Labile_Solid_Phase_P_Indices, "Labile Solid Phase P");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Total_Labile_P_Indices, "Total Labile P");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Renew_Labile_P_Indices, "Renew Labile P");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Buffer_Capacity_Indices, "Buffer Capacity");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Buffer_Index_Indices, "Buffer Index");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Added_P_Indices, "Added P");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Freundlich_n_Indices, "Freundlich Param n");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Recalcitrant_P_Indices, "Recalcitrant P Fraction");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, OrganicP_Indices, "Organic P");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, LocalPUptake_Indices, "Local P Uptake");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, FertFrac_Solid_Indices, "Fraction Fertiliser Solid");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, FertFrac_Liquid_Indices, "Fraction Fertiliser Liquid");

        //Indices for characteristics from other processes
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Bulk_Density_Indices, "Bulk Density");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Water_Content_Indices, "Water Content");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Rained_Amount_Indices, "Rained Amount");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Wetting_Front_Indices, "Wetting Front");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Drained_Upper_Limit_Indices, "Drained Upper Limit");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Water_Moved_Indices, "Water Moved");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Wilting_Point_Indices, "Wilting Point");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Rain_Out_Bottom_Indices, "Rain Out Bottom");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Coeff_C_Indices, "Coefficient c");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Coeff_D_Indices, "Coefficient d");
        SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Direction_Moved_Indices, "Direction Moved");

        // These Characteristics are logically boolean.
        Plus_X_Index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Plus X");
        Minus_X_Index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Minus X");
        Plus_Y_Index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Plus Y");
        Minus_Y_Index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Minus Y");
        Plus_Z_Index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Plus Z");
        Minus_Z_Index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Minus Z");


        Process* theWaterModule = Find_Process_By_Process_Name("Water");
        m_itsWaterModuleID = theWaterModule->GetProcessID();

        PrepareSharedAttributes(action);

        const SharedAttributeCluster& cluster = Get_Cluster("Plant");
        SharedAttributeCluster::const_iterator cluster_iter = cluster.begin();

        // This is "use the next Plant X attribute list".
        // Note that the name of the Plant/Cluster is in (*cluster_iter).first
        const SharedAttributeList& sa_list = (*cluster_iter).second;
        SharedAttribute* nutrient_solution_sa = sa_list[saNutrientSolutionIndex];
        m_IS_NUTRIENT_SOLUTION = nutrient_solution_sa->GetValue() == 1.0; //0 = soil experiment, 1 = nutrient solution experiment
        SharedAttribute* model_root_hairs_sa = sa_list[saModelRootHairsIndex];
        m_MODEL_ROOT_HAIRS = model_root_hairs_sa->GetValue() == 1.0; // 1 = model root hairs, 0 = do not model root hairs

        //26/09/2002 VMD For cases where the P content of the soil is described as total available P need to work out how much is likely
        //to be in solution and how much in the reversible solid phase.  A differencing scheme is used which is similar to that in
        //UptakeP, which solves for the P concentration in solution and labile solid phase that sum to give the total reversible P amount.

        //The Mendham et al freundlich parameters are based upon a labile solid phase P condentration of µg/g soil, and a
        //liquid phase P concentration of µg/cm^3 soln, so need to convert amounts to concentrations in this calculation

        // Check variables; help with increasing the efficiency of this loop
        double previous_total_labile_P = 0;
        double liquid_phase_P = 0;
        double liquid_phase_P_conc = 0;
        double labile_solid_phase_P = 0;

        for (BoxIndex box_index = 0; box_index < BOX_COUNT; ++box_index)
        {
            const double box_volume = Get_BoxIndex_Volume(box_index);

            //SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

            VolumeObjectList::const_iterator iter = volumeobjectcoordinator->GetVolumeObjectList().begin();
            double noncoincidentProportion = 1;
            for (size_t i = 0; i <= VO_COUNT; ++i)
            {
                double thisProportion;
                /*Get pointer to the VolumeObject, or a NULL pointer if it's the last iteration of the loop.*/
                VolumeObject* vo = __nullptr;
                if (i < VO_COUNT)
                {
                    vo = *iter;
                    ++iter;
                    thisProportion = scoreboard->GetCoincidentProportion(box_index, vo->GetIndex());
                    noncoincidentProportion -= thisProportion;
                }
                else
                {
                    /* MSA 11.06.23 Updated to correctly handle for VOs located inside one another.  */
                    thisProportion = Utility::CSMax(0.0, noncoincidentProportion);
                }
                const size_t idx = vo == __nullptr ? 0 : vo->GetIndex();

                // MSA 10.12.07 Gotta do all sections regardless, because we are initialising here
                //if(thisProportion==0)	continue;
                const double thisVolume = thisProportion * box_volume;

                if (!m_IS_NUTRIENT_SOLUTION) //roots are growing in soil
                {
                    const double freundlich_param_a = scoreboard->GetCharacteristicValue(Buffer_Index_Indices[idx], box_index); //User defined parameters for each soil type, describes the Freundlich isotherm
                    const double freundlich_param_n = scoreboard->GetCharacteristicValue(Freundlich_n_Indices[idx], box_index);

                    const double soil_density = scoreboard->GetCharacteristicValue(Bulk_Density_Indices[idx], box_index); //g/cm^3
                    const double water_content = scoreboard->GetCharacteristicValue(Water_Content_Indices[idx], box_index);
                    double total_labile_P = (scoreboard->GetCharacteristicValue(Total_Labile_P_Indices[idx], box_index)) * soil_density * thisVolume; // µg/g * g = µg (initial total amount of labile P)
                    const double added_P = (scoreboard->GetCharacteristicValue(Added_P_Indices[idx], box_index)) * soil_density * thisVolume; //µg/g * g = µg
                    double fertiliser_fraction = 0;
                    if (added_P > 0)
                    {
                        total_labile_P += added_P; //initially all the added P is considered labile, but within 48h a percentage of it will have moved to the fixed P pool
                        fertiliser_fraction = (total_labile_P == 0) ? 0 : (added_P / total_labile_P); //fraction of N in the box that is from fertiliser
                    }
                    scoreboard->SetCharacteristicValue(FertFrac_Liquid_Indices[idx], fertiliser_fraction, box_index);
                    scoreboard->SetCharacteristicValue(FertFrac_Solid_Indices[idx], fertiliser_fraction, box_index);
                    const double tlp = (thisVolume == 0) ? 0 : (total_labile_P / (soil_density * thisVolume)); // µg/g (total amount of labile P in soil)
                    scoreboard->SetCharacteristicValue(Total_Labile_P_Indices[idx], tlp, box_index);

                    if (thisVolume == 0 || total_labile_P == previous_total_labile_P || total_labile_P == 0) //don't waste time recalculating what we already know - or what must logically be 0 - use the previous values
                    {
                        scoreboard->SetCharacteristicValue(Liquid_Phase_P_Indices[idx], liquid_phase_P, box_index); //  µg (amount of P in solution)
                        scoreboard->SetCharacteristicValue(Liquid_Phase_P_Conc_Indices[idx], liquid_phase_P_conc, box_index); //  µmolP/cm^3 solution
                        scoreboard->SetCharacteristicValue(Labile_Solid_Phase_P_Indices[idx], labile_solid_phase_P, box_index);// µg (amount of labile P in soil)
                    }
                    else
                    {
                        double search_interval = (total_labile_P / 2);
                        double new_liquid_phase_P = (search_interval); //µg (in solution)
                        double new_labile_solid_phase_P = (freundlich_param_a * ProcessUtility::guardedPow((new_liquid_phase_P / (water_content * thisVolume)), freundlich_param_n, _logger)) * (soil_density * thisVolume); //(µg/g * g/cm3 * cm3) = ug P            

                        while ((((new_liquid_phase_P + new_labile_solid_phase_P) - (total_labile_P)) > (total_labile_P / 1000))
                            || (((new_liquid_phase_P + new_labile_solid_phase_P) - (total_labile_P)) < (-(total_labile_P) / 1000)))
                        {
                            if ((new_liquid_phase_P + new_labile_solid_phase_P) > (total_labile_P))
                            { //The solution is in the LHS of the distribution
                                search_interval -= search_interval / 2;
                                new_liquid_phase_P -= search_interval; //µg (in solution)
                                new_labile_solid_phase_P = (freundlich_param_a * ProcessUtility::guardedPow((new_liquid_phase_P / (water_content * thisVolume)), freundlich_param_n, _logger)) * (soil_density * thisVolume); //(µg/g * g/cm3 * cm3) = ug P                
                            }
                            else
                            { //The solution is in the RHS of the distribution
                                search_interval = search_interval / 2;
                                new_liquid_phase_P += search_interval; //µg (in solution)
                                new_labile_solid_phase_P = (freundlich_param_a * ProcessUtility::guardedPow((new_liquid_phase_P / (water_content * thisVolume)), freundlich_param_n, _logger)) * (soil_density * thisVolume); //(µg/g * g/cm3 * cm3) = ug P
                            }
                        } //while ((|(new_liquid_phase_P + new_labile_solid_phase_P) - (v)|) > ((total_labile_P)/1000)

                        labile_solid_phase_P = new_labile_solid_phase_P; //µgP labile P in soil
                        liquid_phase_P = new_liquid_phase_P; //µgP in soil solution
                        liquid_phase_P_conc = liquid_phase_P / (water_content * thisVolume * Mr); //µmolP/cm^3 solution
                        previous_total_labile_P = total_labile_P;
                        scoreboard->SetCharacteristicValue(Liquid_Phase_P_Indices[idx], liquid_phase_P, box_index); // µg (amount of P in solution)
                        scoreboard->SetCharacteristicValue(Liquid_Phase_P_Conc_Indices[idx], liquid_phase_P_conc, box_index); // µmolP/cm^3 water (concentration of P in solution)
                        scoreboard->SetCharacteristicValue(Labile_Solid_Phase_P_Indices[idx], labile_solid_phase_P, box_index);// µg (amount of labile P in soil)
                    }
                }// if(!m_IS_NUTRIENT_SOLUTION)

                else // is nutrient solution
                {
                    labile_solid_phase_P = 0; //µgP labile P in soil
                    scoreboard->SetCharacteristicValue(Labile_Solid_Phase_P_Indices[idx], labile_solid_phase_P, box_index);// µg (amount of labile P in soil)

                    double total_labile_P = scoreboard->GetCharacteristicValue(Total_Labile_P_Indices[idx], box_index); // µg/mL = µg/cm^3 water (initial total amount of labile P)
                    liquid_phase_P_conc = total_labile_P / Mr; // µmolP/cm^3 water (concentration of P in solution)
                    scoreboard->SetCharacteristicValue(Liquid_Phase_P_Conc_Indices[idx], liquid_phase_P_conc, box_index); // µmolP/cm^3 water (concentration of P in solution)

                    const double water_content = scoreboard->GetCharacteristicValue(Water_Content_Indices[idx], box_index);
                    liquid_phase_P = total_labile_P * thisVolume * water_content; //µgP in soil solution
                    scoreboard->SetCharacteristicValue(Liquid_Phase_P_Indices[idx], liquid_phase_P, box_index); // µg (amount of P in solution)    
                }

                SUBSECTION_ITER_WITH_PROPORTIONS_END
            } // for(BoxIndex box_index = 0; box_index < BOX_COUNT; ++box_index)

            // Do the other, default stuff
            return (Process::Initialise(action));
        }

        void Phosphorus::PrepareSharedAttributes(ProcessActionDescriptor* action)
        {
            SharedAttributeSearchHelper searchHelper(action->GetSharedAttributeManager(), mySharedAttributeOwner);

            // Load up our expected SharedAttributes, or at least their indices within
            // each Cluster's SharedAttributeList [vector/array]
            //
            // Further comments in "PWater.cp"
            std::vector<std::string> variation_names;
            variation_names.push_back("Plant");
            variation_names.push_back("RootOrder");

            saPhosphorusUptakePerPlantIndex = searchHelper.SearchForClusterIndex("Phosphorus Uptake", "Plant", __nullptr);//+µmol Total phosphorus taken up by each plant at each growth time step and each soil volume (spatial), used in PlantDynamicResourceAllocationToRoots
            saWaterFluxPerPlantPerRootOrderIndex = searchHelper.SearchForClusterIndex("Water Flux", "Plant", variation_names);
            saRootLengthPerPlantIndex = searchHelper.SearchForClusterIndex("Root Length Wrap None", "Plant", __nullptr);
            saRootLengthPerPlantPerRootOrderIndex = searchHelper.SearchForClusterIndex("Root Length Wrap None", "Plant", variation_names);
            saRootRadiusPerPlantPerRootOrderIndex = searchHelper.SearchForClusterIndex("Root Radius", "Plant", variation_names);
            saAbsorptionPowerPerPlantIndex = searchHelper.SearchForClusterIndex("P Absorption Power", "Plant", __nullptr);
            saPUtilisationEfficiencyPerPlantIndex = searchHelper.SearchForClusterIndex("P Utilisation Efficiency", "Plant", __nullptr);
            saLocalCumulPUptakePerPlantIndex = searchHelper.SearchForClusterIndex("Cumul Local Phosphorus Uptake", "Plant", __nullptr);//-µg Cumulative local uptake from each box, spatial
            saTotalCumulPUptakePerPlantIndex = searchHelper.SearchForClusterIndex("Cumulative Phosphorus Uptake", "Plant", __nullptr);//-µg Cumulative P uptake by the whole plant, non-spatial
            saCumulFertPUptakePerPlantIndex = searchHelper.SearchForClusterIndex("Cumulative Fertiliser P Uptake", "Plant", __nullptr);//-µg Cumulative total fertiliser P uptake by each plant, non-spatial
            saSeedingTimeIndex = searchHelper.SearchForClusterIndex("Seeding Time", "Plant", __nullptr);
            saOrgPMineralisationIndex = searchHelper.SearchForClusterIndex("Organic P Mineralisation", "Plant", __nullptr);
            saOrgPMinFractionIndex = searchHelper.SearchForClusterIndex("Org P Min Fraction", "Plant", __nullptr);
            saOrgPMinTimeIndex = searchHelper.SearchForClusterIndex("Org P Min Time", "Plant", __nullptr);
            saNutrientSolutionIndex = searchHelper.SearchForClusterIndex("Nutrient Solution", "Plant", __nullptr);
            saNutrientSolnRenewIndex = searchHelper.SearchForClusterIndex("Nutrient Solution Renew", "Plant", __nullptr);
            saNutrientRenewIndex = searchHelper.SearchForClusterIndex("Nutrient Renew", "Plant", __nullptr);
            saRootHairRadiusIndex = searchHelper.SearchForClusterIndex("Root Hair Radius", "Plant", __nullptr); //cm
            saRootHairLengthIndex = searchHelper.SearchForClusterIndex("Root Hair Length", "Plant", __nullptr); //cm
            saRootHairDensityIndex = searchHelper.SearchForClusterIndex("Root Hair Density", "Plant", __nullptr); //number/cm of root length
            saModelRootHairsIndex = searchHelper.SearchForClusterIndex("Model Root Hairs", "Plant", __nullptr); //model root hairs 1=yes, 0=no
            saOrganicExudatePerPlantIndex = searchHelper.SearchForClusterIndex("Organic Exudate", "Plant", __nullptr); //are we doing root exudation and what type?

            saApaseMultiplier = searchHelper.SearchForAttribute("Apase Multiplier");
        }


        /* ***************************************************************************************************************** */

        long int Phosphorus::StartUp(ProcessActionDescriptor* action)
        {
            Use_Time;
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
            if (m_time_of_prev_solution_change < 0)
            {
                m_time_of_prev_solution_change = time;
            }
            if (m_time_of_prev_nutr_rnw < 0)
            {
                m_time_of_prev_nutr_rnw = time;
            }

            return (Process::StartUp(action));
        }

        long int Phosphorus::Ending(ProcessActionDescriptor* action)
        {
            return (Process::Ending(action));
        }

        /* ***************************************************************************************************************** */
        long int Phosphorus::DoGeneralPurpose(ProcessActionDescriptor* action)
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
            if (source_module_id == m_itsWaterModuleID)
            {
                switch (code)
                {
                case kWaterDrained:
                    return_value = LeachP(action);
                    break;
                case kWaterRedistributed:
                    return_value = DiffuseP(action, true);
                    break;
                default:
                    // log error or whatever
                    break;
                } // switch (code)
            } // if (source_module_id == m_itsWaterModuleID)

            return (return_value);
        }


        /* DoExternalWakeUp
        Called when a 'XWak' message is received. Sent by another process module,
        to wake "this" one up to do some processing based on the other process'
        action[s]. This default method does nothing, you'll need to override it
        for your process to accept wake up calls from other process modules.

        98.10.14 RvH  Fixed the whole lot up. It nearly looks the same, but isn't at all*/
        // MSA 10.12.09 All functionality in this method is #IFDEF'd out. Is the method ever called? Should it be removed?
        long int Phosphorus::DoExternalWakeUp(ProcessActionDescriptor* action)
        {
            Use_ReturnValue;

            // we want to know who sent the wake up, so we can decide how to act.
            Use_Source;

            long source_module_id = -1;
            if (source != __nullptr)
                source_module_id = source->GetProcessID();

            // now check if it is the water module
            if (source_module_id == m_itsWaterModuleID)
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
                    return_value = LeachP(action);  //A message has been send from PWater::Drain
                }

                if (data != __nullptr)
                {
                    return_value = DiffuseP(action, true);  //A message has been send from PWater::Drain
                }
#endif // #if defined NITRATEWATERPHOSPHORUS_OLD_NOTIFICATION_CODE
            }

            return (return_value);
        }

        /* ************************************************************************** */

        long int Phosphorus::DoNormalWakeUp(ProcessActionDescriptor* action)
        {
            Use_ReturnValue;

            return_value |= UptakeP(action);
            return_value |= DiffuseP(action, false);

            return_value |= Process::DoNormalWakeUp(action);

            return (return_value);
        }

        /* ***************************************************************************************************************** */

        long int Phosphorus::DoScoreboardPlantSummaryRegistered(ProcessActionDescriptor* action)
        {
            Use_PlantSummaryRegistration;

            phosphorus_first_plant_summary_index = psr->GetStartIndex();
            m_phosphorus_number_of_plants = psr->GetNumberOfPlants();
            m_phosphorus_number_of_branch_orders = psr->GetNumberOfBranchOrders();

            return (Process::DoScoreboardPlantSummaryRegistered(action));
        }

        /* ***************************************************************************************************************** */

        long int Phosphorus::DiffuseP(ProcessActionDescriptor* action, const bool& drain)
        {
            Use_ReturnValue;
            Use_Time;
            Use_Scoreboard;

            ProcessTime_t dT = time - m_time_of_previous_diffusion;
            m_time_of_previous_diffusion = time;

            if (dT == 0)
            {
                return (return_value);
            }

            //VMD 25/5/03 Diffusion is calculated by stepping through each of the boxes and equilibrating the mobile P content with
            //the surrounding boxes.  This is done every time-step.  The order of cycling through the boxes alternates to eliminate any affect
            //of the order on the pattern of diffusion.  This is a computationally efficient approach that provides similar accuracy to
            //computationally expensive Finite Element Method approaches (see Dunbabin et al. 2005).  In the case of large water movement (drain == true),
            //P diffusion is calculated in two steps to allow for the more accurate calculation of large P displacements.  Works well against
            //measured P leaching and P diffusion (see Dunbabin et al. unpublished).

            int diffusionsRemaining = 1;
            if (drain)
            {
                ++diffusionsRemaining; // Diffuse twice
                if (dT < 3600) { dT = 3600; }
            }

            for (; diffusionsRemaining > 0; --diffusionsRemaining)
            {
                // If !m_diffusion_rotation_toggle, it is time to diffuse in the yxz direction.
                //else it is time to diffuse in the opposite direction
                const long ystart = m_diffusion_rotation_toggle ? NUM_Y : 1;
                const long xstart = m_diffusion_rotation_toggle ? NUM_X : 1;
                const long crement = m_diffusion_rotation_toggle ? -1 : 1;

                //Look at each box in the scoreboard and "equilibrate" it with its surrounding boxes.
                for (long y = ystart; m_diffusion_rotation_toggle ? (y >= 1) : (y <= NUM_Y); y += crement)
                {
                    for (long x = xstart; m_diffusion_rotation_toggle ? (x >= 1) : (x <= NUM_X); x += crement)
                    {
                        for (long z = 1; z <= NUM_Z; ++z)
                        {
                            const BoxIndex box_index = scoreboard->GetBoxIndex(x, y, z);

                            if (!m_diffusion_rotation_toggle)
                            {
                                Set_Concentration_Of_BoxIndex(Plus_X_Index, 0, box_index); //new round of diffusionsRemaining
                                Set_Concentration_Of_BoxIndex(Minus_X_Index, 0, box_index);
                                Set_Concentration_Of_BoxIndex(Plus_Y_Index, 0, box_index);
                                Set_Concentration_Of_BoxIndex(Minus_Y_Index, 0, box_index);
                                Set_Concentration_Of_BoxIndex(Plus_Z_Index, 0, box_index);
                                Set_Concentration_Of_BoxIndex(Minus_Z_Index, 0, box_index);
                            }

                            CalculateDiffusionOrder(action, x, y, z, dT, drain);
                        } //for (long z=1; z<=NUM_Z; ++z)
                    } //for (long x=xstart; x<=xend; x+=crement)
                } //for (long y=ystart; y<=yend; y+=crement)

                m_diffusion_rotation_toggle = !m_diffusion_rotation_toggle; // Toggle from true to false or vice versa

                if (!m_IS_NUTRIENT_SOLUTION) //roots growing in soil
                {
                    Use_VolumeObjectCoordinator;
                    // MSA 09.10.27 Converted from xyz loop to BoxIndex loop for simplicity

                    for (BoxIndex box_index = 0; box_index < BOX_COUNT; ++box_index)
                    {
                        SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                            const double liquid_phase_P = scoreboard->GetCharacteristicValue(Liquid_Phase_P_Indices[idx], box_index); // µg (amount of P in solution)
                        const double labile_solid_phase_P = scoreboard->GetCharacteristicValue(Labile_Solid_Phase_P_Indices[idx], box_index); // µg (amount of labile solid-phase P in soil)
                        const double total_labile_P = liquid_phase_P + labile_solid_phase_P;
                        CalculateSolidLiquidPartition(action, labile_solid_phase_P, total_labile_P, box_index, vo, thisProportion); //now that P has diffused, need to recalculate the solid/liquid partition

                        SUBSECTION_ITER_WITH_PROPORTIONS_END
                    } //for(BoxIndex bi=0; bi<BOX_COUNT; ++bi)
                } //roots growing in soil
            } //for(;diffusionsRemaining > 0; --diffusionsRemaining)

            return (return_value);
        } //Phosphorus::DiffuseP(ProcessActionDescriptor *action)

        /* **************************************************************************************************************** */
        long int Phosphorus::CalculateDiffusionOrder(ProcessActionDescriptor* action, const long& x, const long& y, const long& z, const ProcessTime_t& dT, const bool& drain)
        {
            Use_ReturnValue;
            Use_Scoreboard;

            const BoxIndex box_index = scoreboard->GetBoxIndex(x, y, z);

            //Calculate the transfer that would occur to the LHS box
            if (x != 1) // For now, do not transfer Phosphorus outside the scoreboard.
            {
                const bool doDiffusion = scoreboard->GetCharacteristicValue(Minus_X_Index, box_index) == 0; // Check if P has already diffused through this box wall
                if (doDiffusion)
                {
                    const bool diffused = CalculatePhosphorusDiffusion(action, x, y, z, x - 1, y, z, dT, drain, X, false);
                    if (diffused)
                    {
                        Set_Concentration_Of_Box(Plus_X_Index, 1, (x - 1), y, z); //dont diffuse back from the next box
                    }
                }
            } //if (x != 1)

            //Calculate the transfer that would occur to the RHS box
            if (x != NUM_X) // For now, do not transfer Phosphorus outside the scoreboard.
            {
                const bool doDiffusion = scoreboard->GetCharacteristicValue(Plus_X_Index, box_index) == 0; // Check if P has already diffused through this box wall
                if (doDiffusion)
                {
                    const bool diffused = CalculatePhosphorusDiffusion(action, x, y, z, x + 1, y, z, dT, drain, X, true);
                    if (diffused)
                    {
                        Set_Concentration_Of_Box(Minus_X_Index, 1, (x + 1), y, z); //dont diffuse back from the next box
                    }
                }
            } //if (x != NUM_X)

            //Calculate the transfer that would occur to the box in front
            if (y != 1) // For now, do not transfer nitrate outside the scoreboard.
            {
                // Check if P has already diffused through this box wall
                const bool doDiffusion = scoreboard->GetCharacteristicValue(Minus_Y_Index, box_index) == 0;
                if (doDiffusion)
                {
                    // If not, diffuse it now.
                    const bool diffused = CalculatePhosphorusDiffusion(action, x, y, z, x, y - 1, z, dT, drain, Y, false);
                    if (diffused)
                    {
                        Set_Concentration_Of_Box(Plus_Y_Index, 1, x, (y - 1), z); //dont diffuse back from the next box
                    }
                }
            } //if (y != 1)

            //Calculate the transfer that would occur to the box behind
            if (y != NUM_Y) // For now, do not transfer nitrate outside the scoreboard.
            {
                const bool doDiffusion = scoreboard->GetCharacteristicValue(Plus_Y_Index, box_index) == 0;
                // Check if P has already diffused through this box wall
                if (doDiffusion)
                {
                    // If not, diffuse it now.
                    const bool diffused = CalculatePhosphorusDiffusion(action, x, y, z, x, y + 1, z, dT, drain, Y, true);
                    if (diffused)
                    {
                        Set_Concentration_Of_Box(Minus_Y_Index, 1, x, (y + 1), z); //dont diffuse back from the next box
                    }
                }
            } //if (y != NUM_Y)

            //Calculate the transfer that would occur to the box on top
            if (z != 1) // For now, do not transfer nitrate outside the scoreboard.
            {
                const bool doDiffusion = scoreboard->GetCharacteristicValue(Minus_Z_Index, box_index) == 0;
                // Check if P has already diffused through this box wall
                if (doDiffusion)
                {
                    // If not, diffuse it now.
                    const bool diffused = CalculatePhosphorusDiffusion(action, x, y, z, x, y, z - 1, dT, drain, Z, false);
                    if (diffused)
                    {
                        Set_Concentration_Of_Box(Plus_Z_Index, 1, x, y, (z - 1)); //dont diffuse back from the next box
                    }
                }
            } //if (z != 1)

            //Calculate the transfer that would occur to the box beneath
            if (z != NUM_Z)// For now, do not transfer nitrate outside the scoreboard.
            {
                const bool doDiffusion = scoreboard->GetCharacteristicValue(Plus_Z_Index, box_index) == 0;
                // Check if P has already diffused through this box wall
                if (doDiffusion)
                {
                    // If not, diffuse it now.
                    const bool diffused = CalculatePhosphorusDiffusion(action, x, y, z, x, y, z + 1, dT, drain, Z, true);
                    if (diffused)
                    {
                        Set_Concentration_Of_Box(Minus_Z_Index, 1, x, y, (z + 1)); //dont diffuse back from the next box
                    }
                }
            } //if (z != NUM_Z)

            return (return_value);
        } //Phosphorus::CalculateDiffusionOrder

        /* **************************************************************************************************************** */

        // MSA 11.01.27 This method does the actual diffusion between two spatial-subsections of adjacent boxes.
        // It does not check for transfer restriction; callers of this method should first ensure that the requested diffusion is legal.
        // The parameter volumeObjectSurfacePermeability is only used in the case of diffusion between different spatial subsections of the same Box.
        bool Phosphorus::DoDiffusion(Scoreboard* scoreboard, const BoxIndex& fromBox, const BoxIndex& toBox, const size_t& fromVOIndex, const size_t& toVOIndex, const double& fromProportion, const double& toProportion, const double& avgDistance, const ProcessTime_t& dT, const bool& drain, const double& volumeObjectSurfacePermeability /* = 0 */)
        {
            if (fromBox == toBox && (fromVOIndex == toVOIndex || volumeObjectSurfacePermeability <= 0.0)) return false; // Shortcut exit

            if (fromProportion <= 0.0 || toProportion <= 0.0) return false;

            const double fromVolume = Get_BoxIndex_Volume(fromBox) * fromProportion;
            const double toVolume = Get_BoxIndex_Volume(toBox) * toProportion;

            double liquidPhasePFrom = scoreboard->GetCharacteristicValue(Liquid_Phase_P_Indices[fromVOIndex], fromBox);
            const double waterContentFrom = scoreboard->GetCharacteristicValue(Water_Content_Indices[fromVOIndex], fromBox);
            double fertFracLiquidFrom = scoreboard->GetCharacteristicValue(FertFrac_Liquid_Indices[fromVOIndex], fromBox);
            double PConcFrom = waterContentFrom <= 0.0 ? 0.0 : (liquidPhasePFrom / (waterContentFrom * fromVolume));

            double liquidPhasePTo = scoreboard->GetCharacteristicValue(Liquid_Phase_P_Indices[toVOIndex], toBox);
            double labileSolidPhasePTo = scoreboard->GetCharacteristicValue(Labile_Solid_Phase_P_Indices[toVOIndex], toBox);
            const double waterContentTo = scoreboard->GetCharacteristicValue(Water_Content_Indices[toVOIndex], toBox);
            const double fertFracLiquidTo = scoreboard->GetCharacteristicValue(FertFrac_Liquid_Indices[toVOIndex], toBox);
            double fertFracSolidTo = scoreboard->GetCharacteristicValue(FertFrac_Solid_Indices[toVOIndex], toBox);
            double PConcTo = waterContentTo <= 0.0 ? 0.0 : (liquidPhasePTo / (waterContentTo * toVolume));


            if (PConcFrom <= PConcTo) return false;

            const double deltaPRestrictionModifier = fromVOIndex != toVOIndex ? volumeObjectSurfacePermeability : 1.0;

            if (m_IS_NUTRIENT_SOLUTION) //roots are growing in nutrient solution
            {
                //Assume the nutrient solution is well mixed, so there are no zones of local P depletion
                const double deltaP = (liquidPhasePFrom - liquidPhasePTo) / 2.0; //µgP/cm3H20*cm2/sec*cm2/cm*sec = µgP

                if ((deltaP * deltaPRestrictionModifier) > liquidPhasePFrom)
                {
                    RmAssert((deltaP*deltaPRestrictionModifier) < 1e-15, "Error: non-trivial deltaP exceeds liquid phase P");
                    return false; // (assuming the assertion above is correct:) deltaP considered too small to bother with
                }

                liquidPhasePFrom -= deltaPRestrictionModifier * deltaP;
                liquidPhasePTo += deltaPRestrictionModifier * deltaP;

                PConcFrom = liquidPhasePFrom / fromVolume; //µgP/cm3
                PConcTo = liquidPhasePTo / toVolume; //µgP/cm3

                scoreboard->SetCharacteristicValue(Liquid_Phase_P_Indices[fromVOIndex], liquidPhasePFrom, fromBox);
                scoreboard->SetCharacteristicValue(Liquid_Phase_P_Indices[toVOIndex], liquidPhasePTo, toBox);
                scoreboard->SetCharacteristicValue(Liquid_Phase_P_Conc_Indices[fromVOIndex], PConcFrom, fromBox);
                scoreboard->SetCharacteristicValue(Liquid_Phase_P_Conc_Indices[toVOIndex], PConcTo, toBox);
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

                const double drainage_dispersivity_factor = drain ? 2.0 : 1.0;

                // Because deltaP is repeatedly used in calculations, we'll apply the deltaPRestrictionModifier here.
                double deltaP = deltaPRestrictionModifier * drainage_dispersivity_factor * m_DIFFUSION_COEFF_SOLN * lowest_water_content * impedance_factor * (PConcFrom - PConcTo) * avgDistance * dT; //µgP

                    // Mitigating floating-point issues
                if (deltaP < 1e-10) return false;

                // Checks and balances
                double newPConcFrom = waterContentFrom <= 0.0 ? 0.0 : ((liquidPhasePFrom - deltaP) / (waterContentFrom * fromVolume * Mr));
                double newPConcTo = waterContentTo <= 0.0 ? 0.0 : ((liquidPhasePTo + deltaP) / (waterContentTo * toVolume * Mr));
                double concentrationDifference = newPConcFrom - newPConcTo;
                if (concentrationDifference < 0.0)
                {
                    // Clamp transfer so as not to exceed equalisation

                    /*
                    The algebra: starting from the required result, that newPConcFrom == newPConcTo.

                    [[MSA 11.03.24 Vanessa, could you check this please? My basic maths can be shockingly deficient at times]]


                    (liquidPhasePFrom - deltaP) / (waterContentFrom * fromVolume*Mr)
                    =
                    (liquidPhasePTo + deltaP) / (waterContentTo * toVolume*Mr)

                    so

                    (liquidPhasePFrom - deltaP) * waterContentTo * toVolume*Mr
                    =
                    (liquidPhasePTo + deltaP) * waterContentFrom * fromVolume*Mr

                    and

                    liquidPhasePFrom * waterContentTo * toVolume*Mr
                    =
                    (liquidPhasePTo + deltaP) * waterContentFrom * fromVolume*Mr + (deltaP) * waterContentTo * toVolume*Mr

                    and

                    liquidPhasePFrom * waterContentTo * toVolume*Mr
                    =
                    liquidPhasePTo * waterContentFrom * fromVolume*Mr + (waterContentTo * toVolume*Mr + waterContentFrom * fromVolume*Mr) * deltaP

                    thus

                    liquidPhasePFrom * waterContentTo * toVolume*Mr - liquidPhasePTo * waterContentFrom * fromVolume*Mr
                    =
                    (waterContentTo * toVolume*Mr + waterContentFrom * fromVolume*Mr) * deltaP

                    so, finally,

                    deltaP

                    =

                    (liquidPhasePFrom * waterContentTo * toVolume*Mr - liquidPhasePTo * waterContentFrom * fromVolume*Mr) / (waterContentTo * toVolume*Mr + waterContentFrom * fromVolume*Mr)

                    */
                    deltaP = (liquidPhasePFrom * waterContentTo * toVolume * Mr - liquidPhasePTo * waterContentFrom * fromVolume * Mr) / (waterContentTo * toVolume * Mr + waterContentFrom * fromVolume * Mr);

                    // Do a sanity check by using our result to recalculate new concentrations
                    newPConcFrom = waterContentFrom <= 0.0 ? 0.0 : ((liquidPhasePFrom - deltaP) / (waterContentFrom * fromVolume * Mr));
                    newPConcTo = waterContentTo <= 0.0 ? 0.0 : ((liquidPhasePTo + deltaP) / (waterContentTo * toVolume * Mr));

                    RmAssert((newPConcTo - newPConcFrom) < 1e-10, "If variance is small: floating point inaccuracy; increase size of allowable epsilon value. If variance is large: fatal algebra error; return programmer to remedial high school mathematics class");

                    // Mitigating floating-point issues
                    if (deltaP < 1e-10) return false;
                    /*
                    double newPConcFromCheck = waterContentFrom<=0 ? 0 : ((liquidPhasePFrom - deltaP) / (waterContentFrom * fromVolume));
                    double concentrationDifferenceCheck = newPConcFromCheck-avgPConc;
                    */
                }

                fertFracLiquidFrom = ((fertFracLiquidFrom * liquidPhasePFrom) - (fertFracLiquidFrom * deltaP)) / (liquidPhasePFrom - deltaP);

                liquidPhasePFrom -= deltaP; //µgP
                RmAssert(liquidPhasePFrom >= 0.0, "liquid phase P <0!"); //a check against negative P amounts

                //because P has been moved, the ratio of fertiliser to background P in the box has changed.
                fertFracLiquidFrom = ((fertFracLiquidTo * liquidPhasePTo) + (fertFracLiquidFrom * 0.3 * deltaP)) / (liquidPhasePTo + (0.3 * deltaP));
                fertFracSolidTo = ((fertFracSolidTo * labileSolidPhasePTo) + (fertFracLiquidFrom * 0.7 * deltaP)) / (labileSolidPhasePTo + (0.7 * deltaP));

                liquidPhasePTo += 0.3 * deltaP; //µgP, initial solid/liquid split assumption. Actual split solved in the partitioning routine.  This initial split reduces time to solution.
                labileSolidPhasePTo += 0.7 * deltaP; //µgP

                LOG_ERROR_IF(!((liquidPhasePFrom < 10000) && (liquidPhasePFrom > 0.0)));
                LOG_ERROR_IF(!((liquidPhasePTo < 10000) && (liquidPhasePTo > 0.0)));

                scoreboard->SetCharacteristicValue(Labile_Solid_Phase_P_Indices[toVOIndex], labileSolidPhasePTo, toBox);
                scoreboard->SetCharacteristicValue(FertFrac_Solid_Indices[toVOIndex], fertFracSolidTo, toBox);

                scoreboard->SetCharacteristicValue(Liquid_Phase_P_Indices[fromVOIndex], liquidPhasePFrom, fromBox);
                scoreboard->SetCharacteristicValue(Liquid_Phase_P_Indices[toVOIndex], liquidPhasePTo, toBox);
                scoreboard->SetCharacteristicValue(Liquid_Phase_P_Conc_Indices[fromVOIndex], (liquidPhasePFrom / (fromVolume * waterContentFrom * Mr)), fromBox);
                scoreboard->SetCharacteristicValue(Liquid_Phase_P_Conc_Indices[toVOIndex], (liquidPhasePTo / (toVolume * waterContentTo * Mr)), toBox);
                scoreboard->SetCharacteristicValue(FertFrac_Liquid_Indices[fromVOIndex], fertFracLiquidFrom, fromBox);
                scoreboard->SetCharacteristicValue(FertFrac_Liquid_Indices[toVOIndex], fertFracLiquidTo, toBox);
            } //roots are growing in soil

            return (true);
        }

        // MSA 10.10.28 Does this function require any guarantee that its parameter Scoreboard Boxes are adjacent? Logically it should, right?
        bool Phosphorus::CalculatePhosphorusDiffusion(ProcessActionDescriptor* action, const long& this_x, const long& this_y, const long& this_z, const long& next_x, const long& next_y, const long& next_z, const ProcessTime_t& dT, const bool& drain, Dimension transferDimension, bool transferInPositiveDir)
        {
            Use_Scoreboard;
            Use_VolumeObjectCoordinator;
            // MSA 11.04.28 Replaces Use_Boundaries;
            const BoundaryArray& X_boundaries = m_boundarySet->X_boundaries;
            const BoundaryArray& Y_boundaries = m_boundarySet->Y_boundaries;
            const BoundaryArray& Z_boundaries = m_boundarySet->Z_boundaries;

            // MSA 11.02.04 Simplifying this method according to the technique used in Phosphorus::CalculateNitrateDiffusion.
            // MSA "recoding this method to address subsections of boxes serially."

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

            const bool thisRestricted = trd.unrestrictedProportionFrom < 1;

            // --------------------------------------------------------------------------------------------

            if (thisRestricted)
            {
                // We must first calculate the diffusion from the restricted section (the section inaccessible to the next box) to the unrestricted section.
                if (trd.voFromPermeability > 0.0)
                {
                    const size_t RIndex = trd.unrestrictedTransferWithinVO ? thisBackgroundVOIndex : trd.volumeObjectFrom->GetIndex();
                    const size_t UIndex = trd.unrestrictedTransferWithinVO ? trd.volumeObjectFrom->GetIndex() : thisBackgroundVOIndex;

                    // Call the actual diffusion method twice, once for each direction. Diffusion will only occur if the from-section's Phosphorus concentration
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

                        // Attempt to transfer some Phosphorus from:	(i==0)	the restricted section to the unrestricted, then
                        //												(i==1)	the unrestricted section to the restricted.
                        DoDiffusion(scoreboard, thisBox, thisBox, thisFromIndex, thisToIndex, thisProportion, nextProportion,
                            dist, dT, drain, trd.voFromPermeability);
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

                    // Call the actual diffusion method twice, once for each direction. Diffusion will only occur if the from-section's Phosphorus concentration
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

                        // Attempt to transfer some Phosphorus from:	(i==0)	the restricted section to the unrestricted, then
                        //												(i==1)	the unrestricted section to the restricted.
                        DoDiffusion(scoreboard, nextBox, nextBox, thisFromIndex, thisToIndex, thisProportion, nextProportion,
                            dist, dT, drain, trd.voToPermeability);
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
            //	  (i.e. the one corresponding with the same VO / lack of VO) of the other Box.


            // 1)
            if (!thisRestricted && !nextRestricted)
            {
                Use_VolumeObjectCoordinator;

                SUBSECTION_ITER_WITH_TO_AND_FROM_BOXES_AND_PROPORTIONS_BEGIN

                    // If there is no volume available to transfer out of or into, naturally we can't do any diffusion.
                    if (thisProportion == 0.0 || nextProportion == 0.0) continue;

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
                    thisProportion, nextProportion,
                    dist, dT, drain);

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
                    thisProportion, nextProportion, dist, dT, drain, permeability);
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
                    thisProportion, nextProportion, dist, dT, drain, permeability);
            }

            return true; // Diffusion took place
        } //Phosphorus::CalculatePhosphateDiffusion

        /* ***************************************************************************************************************** */
        // MSA 10.12.08 Major change: this method now operates only on the subsection of the parameter Box spatially coincident with the parameter VolumeObject.
        // This VolumeObject pointer may of course be NULL (for "no VolumeObject").
        // Call the method for each VO and once for NULL for each Box.
        void Phosphorus::CalculateSolidLiquidPartition(ProcessActionDescriptor* action, const double& labileSolidPhaseP, const double& totalAvailableP, const BoxIndex& box_index, VolumeObject* coincidentVO, const double& coincidentProportion)
        {
            //05/09/2002 VMD When liquid phase P is taken up or diffused, need to re-equilabrate with the solid phase P
            //A differencing scheme is used to solve for a liquid and solid phase split
            Use_Scoreboard;

            const double thisVolume = coincidentProportion * Get_BoxIndex_Volume(box_index);

            if (thisVolume <= 0.0) return;

            double liquidPhaseP = totalAvailableP - labileSolidPhaseP; // µgP (amount of P in solution)
            if (liquidPhaseP < 0) { liquidPhaseP = 0.0; }

            const size_t idx = coincidentVO == __nullptr ? 0 : coincidentVO->GetIndex();

            const double waterContent = scoreboard->GetCharacteristicValue(Water_Content_Indices[idx], box_index);
            const double soilDensity = scoreboard->GetCharacteristicValue(Bulk_Density_Indices[idx], box_index); //g/cm^3
            double searchInterval = (labileSolidPhaseP / 2);

            double fertFracLiquid = scoreboard->GetCharacteristicValue(FertFrac_Liquid_Indices[idx], box_index);

            RmAssert(fertFracLiquid <= 1.0, "fertiliser fraction >1!");
            double fertFracSolid = scoreboard->GetCharacteristicValue(FertFrac_Solid_Indices[idx], box_index);
            RmAssert(fertFracSolid <= 1.0, "fertiliser fraction >1!");

            //User defined parameters for each soil type, describes the Freundlich isotherm
            const double freundlich_param_a = scoreboard->GetCharacteristicValue(Buffer_Index_Indices[idx], box_index);
            const double freundlich_param_n = scoreboard->GetCharacteristicValue(Freundlich_n_Indices[idx], box_index);

            //The Mendham et al. freundlich parameters are based upon a labile solid phase P condentration of µg/g soil, and a
            //liquid phase P concentration of µg/cm^3 soln, so I need to convert amounts to concentrations in this calculation
            double newLabileSolidPhaseP = (totalAvailableP > (labileSolidPhaseP * 1.1)) ? totalAvailableP : labileSolidPhaseP; //µgP

            double newLiquidPhaseP = (ProcessUtility::guardedPow((newLabileSolidPhaseP / (soilDensity * thisVolume * freundlich_param_a)), (1.0 / freundlich_param_n), _logger) * waterContent * thisVolume); //µgP/ml * ml/cm3 * cm3 = µgP

            bool pErrorIsLarge = Utility::CSAbs((newLiquidPhaseP + newLabileSolidPhaseP - totalAvailableP)) > (0.001 * totalAvailableP);
            size_t i = 0;
            size_t maxLoopSize = static_cast<size_t>(300.0 / coincidentProportion);
            // MSA in case unsigned int wraps (!)
            if (maxLoopSize == 0) maxLoopSize = std::numeric_limits<size_t>::max();
            while (pErrorIsLarge && i <= maxLoopSize)
            {
                //Determine if the solution is in the LHS or RHS of the distribution
                const bool solutionIsInLHS = (newLiquidPhaseP + newLabileSolidPhaseP) > (totalAvailableP);

                searchInterval /= 2.0;
                newLabileSolidPhaseP += (solutionIsInLHS ? -1.0 : 1.0) * searchInterval; //µgP
                newLiquidPhaseP = (ProcessUtility::guardedPow((newLabileSolidPhaseP / (soilDensity * thisVolume * freundlich_param_a)), (1 / freundlich_param_n), _logger) * waterContent * thisVolume); //µgP/ml * ml/cm3 * cm3 = µgP            

                ++i;
                pErrorIsLarge = Utility::CSAbs((newLiquidPhaseP + newLabileSolidPhaseP - totalAvailableP)) > (0.001 * totalAvailableP);
            }

            if (pErrorIsLarge)
            {
                newLabileSolidPhaseP = totalAvailableP / 2;
                newLiquidPhaseP = totalAvailableP / 2;
                pErrorIsLarge = false;
            }
            // Moved this here so we only have to do the assertion once per method call
            RmAssert(!pErrorIsLarge, "i out of bounds");

            if (newLiquidPhaseP > liquidPhaseP) //then some P has moved from the labile solid phase P pool into the liquid phase pool
            {
                // MSA 11.03.09 This conditional is redundant
                //if((newLiquidPhaseP+newLabileSolidPhaseP)!=(liquidPhaseP+labileSolidPhaseP))
                if ((newLiquidPhaseP + newLabileSolidPhaseP) > (liquidPhaseP + labileSolidPhaseP))
                {
                    newLabileSolidPhaseP -= ((newLiquidPhaseP + newLabileSolidPhaseP) - (liquidPhaseP + labileSolidPhaseP));
                }
                const double fertMoved = (newLiquidPhaseP - liquidPhaseP) * fertFracSolid; //the fertiliser fraction of the labile solid phase determines the proportion moved into liquid phase from fertiliser
                fertFracLiquid = (newLiquidPhaseP == 0.0) ? 0.0 : ((fertFracLiquid * liquidPhaseP + fertMoved) / newLiquidPhaseP);
                RmAssert(fertFracLiquid <= 1, "Invalid proportion");
                scoreboard->SetCharacteristicValue(FertFrac_Liquid_Indices[idx], fertFracLiquid, box_index); //adjust fraction of liquid P in the box that is fertiliser in origin
            }
            else //P has moved from the liquid phase into the solid phase
            {
                const double fertMoved = (liquidPhaseP - newLiquidPhaseP) * fertFracLiquid; //the fertiliser fraction of the labile liquid phase determines the proportion moved into solid phase from fertiliser
                fertFracSolid = (newLabileSolidPhaseP == 0.0) ? 0.0 : ((fertFracSolid * labileSolidPhaseP + fertMoved) / newLabileSolidPhaseP);
                RmAssert(fertFracSolid < 1, "Invalid proportion");
                scoreboard->SetCharacteristicValue(FertFrac_Solid_Indices[idx], fertFracSolid, box_index); //adjust fraction of solid P in the box that is fertiliser in origin
            }

            //newLabileSolidPhaseP;     //µgP labile P in soil
            liquidPhaseP = newLiquidPhaseP; //µgP in soil solution
            RmAssert(liquidPhaseP >= 0, "Invalid proportion");


            const double liquidPhasePConc = (waterContent == 0.0) ? 0.0 : (liquidPhaseP / (waterContent * thisVolume * Mr)); //µmolP/cm^3 solution
            scoreboard->SetCharacteristicValue(Liquid_Phase_P_Indices[idx], liquidPhaseP, box_index); // µg (amount of P in solution)
            scoreboard->SetCharacteristicValue(Liquid_Phase_P_Conc_Indices[idx], liquidPhasePConc, box_index); // µg/cm^3 water (concentration of P in solution)
            scoreboard->SetCharacteristicValue(Labile_Solid_Phase_P_Indices[idx], newLabileSolidPhaseP, box_index);// µg (amount of labile P in soil)
            const double tlp = (liquidPhaseP + newLabileSolidPhaseP) / (soilDensity * thisVolume); // µg/g (initial total amount of labile P)
            scoreboard->SetCharacteristicValue(Total_Labile_P_Indices[idx], tlp, box_index);
        } // Phosphorus::CalculateSolidLiquidPartition(ProcessActionDescriptor *action)

        /* ***************************************************************************************************************** */

        long int Phosphorus::UptakeP(ProcessActionDescriptor* action)
        {
            // MSA 09.10.28 Updated to use arrays and loops for rootOrder-dependent values, as in Nitrate
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
            RmAssert(dT > 0, "Time delta since previous P update waking is negative");

            const SharedAttributeCluster& cluster = Get_Cluster("Plant");
            for (SharedAttributeCluster::const_iterator cluster_iter = cluster.begin();
                cluster_iter != cluster.end();
                ++cluster_iter
                )
            {
                // This is "use the next Plant X attribute list".
                // Note that the name of the Plant/Cluster is in (*cluster_iter).first
                const SharedAttributeList& sa_list = (*cluster_iter).second;

                // This is "use the next attribute list"  RvH
                SharedAttribute* root_length_sa = sa_list[saRootLengthPerPlantIndex];
                SharedAttribute* root_length_ro_sa = sa_list[saRootLengthPerPlantPerRootOrderIndex];
                SharedAttribute* root_radius_ro_sa = sa_list[saRootRadiusPerPlantPerRootOrderIndex];
                SharedAttribute* water_flux_sa = sa_list[saWaterFluxPerPlantPerRootOrderIndex];
                SharedAttribute* phosphorus_uptake_sa = sa_list[saPhosphorusUptakePerPlantIndex];//+µmol Cumulative local uptake from each box, by each plant, spatial
                SharedAttribute* local_cumul_p_uptake_sa = sa_list[saLocalCumulPUptakePerPlantIndex]; //-µg Cumulative local uptake from each box, by each plant, spatial
                SharedAttribute* total_cumul_p_uptake_sa = sa_list[saTotalCumulPUptakePerPlantIndex]; //-µg Cumulative total P uptake by each plant, non-spatial
                SharedAttribute* cumul_fert_p_uptake_sa = sa_list[saCumulFertPUptakePerPlantIndex]; //-µg Cumulative total fertiliser P uptake by each plant, non-spatial
                SharedAttribute* absorption_power_sa = sa_list[saAbsorptionPowerPerPlantIndex];
                SharedAttribute* crop_sowing_sa = sa_list[saSeedingTimeIndex]; //Time in hours from start of simulation (Time zero) that the seed was sown
                SharedAttribute* organic_p_mineralisation_sa = sa_list[saOrgPMineralisationIndex];
                SharedAttribute* organic_p_fraction_sa = sa_list[saOrgPMinFractionIndex];
                SharedAttribute* organic_p_time_sa = sa_list[saOrgPMinTimeIndex];
                SharedAttribute* nutr_soln_renew_sa = sa_list[saNutrientSolnRenewIndex];
                SharedAttribute* nutr_renew_sa = sa_list[saNutrientRenewIndex];
                SharedAttribute* root_hair_radius_sa = sa_list[saRootHairRadiusIndex]; //cm
                SharedAttribute* root_hair_length_sa = sa_list[saRootHairLengthIndex]; //cm
                SharedAttribute* root_hair_density_sa = sa_list[saRootHairDensityIndex]; //number/cm of root
                SharedAttribute* P_util_efficiency_sa = sa_list[saPUtilisationEfficiencyPerPlantIndex];
                SharedAttribute* organic_exudate_sa = saOrganicExudatePerPlantIndex >= 0 ? sa_list[saOrganicExudatePerPlantIndex] : __nullptr;

                const ProcessTime_t nutr_soln_rnw = PROCESS_DAYS(nutr_soln_renew_sa->GetValue()); //time between solution changes, converted from days to seconds
                const double nutr_rnw = PROCESS_DAYS(nutr_renew_sa->GetValue()); //time in seconds between additions of nutrient solution to free draining sandy soil in pots

                const bool organic_p_mineralisation = organic_p_mineralisation_sa->GetValue() == 1;
                if (organic_p_mineralisation)
                {
                    const double organic_p_fraction = organic_p_fraction_sa->GetValue();
                    const ProcessTime_t organic_p_time = static_cast<ProcessTime_t>(organic_p_time_sa->GetValue());
                    //This routine calculates organic P mineralisation rates as taken from field measurements
                    CalculatePhosphateMineralisation(action, dT, organic_p_fraction, organic_p_time);
                }

                //TRS 2018-10 set up for root exudation if needed
                int OrganicExudate = 0; //assume no exudation unless otherwise told
                if (organic_exudate_sa != __nullptr) //i.e. if the XML config knows about exudation
                {
                    try
                    {
                        OrganicExudate = organic_exudate_sa->GetValue(); //what kind of exudation do we have?
                    }
                    catch (...)
                    {
                        OrganicExudate = 0; //the XML config doesn't know about exudation so assume there's none
                    }
                }
                //TRS 2018-11 This is where we do the root exudation mods
                int exudate_test = 5;
                float exudate_factor = 0;

                switch (OrganicExudate) {
                case 0: //no exudation
                    exudate_test = 0;
                    break;
                case 1: //lecithin
                        //insert soilPLevels into appropriate curve function for lecithin
                        //then do the relevant mod
                    exudate_test = 1;
                    break;
                case 2: //citrate
                    exudate_test = 2;
                    exudate_factor = 2.4; //curve function a simple hyperbola for now
                    break;
                case 3: //malate
                    exudate_test = 3;
                    exudate_factor = 0.4; //curve function a simple hyperbola for now
                    break;
                default: //we don't know what this number means, whoops
                    //error message to log "Unrecognised organic exudate"
                    exudate_test = 4;
                }


                for (BoxIndex box_index = 0; box_index < BOX_COUNT; ++box_index)
                {
                    const double box_volume = Get_BoxIndex_Volume(box_index);

                    SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                        const double thisVolume = thisProportion * box_volume;
                    const double soil_density = scoreboard->GetCharacteristicValue(Bulk_Density_Indices[idx], box_index); //g/cm^3

                    // If this section has no volume, no P can be taken up from it.
                    if (thisVolume <= 0.0) continue;

                    if (!m_IS_NUTRIENT_SOLUTION) //roots are growing in soil
                    {
                        double liquidPhaseP = scoreboard->GetCharacteristicValue(Liquid_Phase_P_Indices[idx], box_index); // µg (amount of P in solution)
                        double labileSolidPhaseP = scoreboard->GetCharacteristicValue(Labile_Solid_Phase_P_Indices[idx], box_index); // µg (amount of labile solid-phase P in soil)

                        if (liquidPhaseP <= 0.0) continue; // Insufficient liquid phase P. Go to next box subsection.

                        const double waterContent = scoreboard->GetCharacteristicValue(Water_Content_Indices[idx], box_index);
                        const double wilting_point = scoreboard->GetCharacteristicValue(Wilting_Point_Indices[idx], box_index);

                        if (waterContent <= (wilting_point * 1.0001)) continue; // Insufficient water content. Go to next box subsection.

                        //impedance factor takes the form of a 2nd order polynomial. The user enters the coefficients that fit their soil.
                        //This can easily be changed at any time to a different type of equation
                        const double coeff_c = scoreboard->GetCharacteristicValue(Coeff_C_Indices[idx], box_index);
                        const double coeff_d = scoreboard->GetCharacteristicValue(Coeff_D_Indices[idx], box_index);
                        const double impedance_factor = (coeff_c * ProcessUtility::guardedPow(waterContent, 2, _logger)) + (coeff_d * waterContent);
                        DEBUGGER(impedance_factor < 0); //a check against negative impedance factor

                        const double soil_density = scoreboard->GetCharacteristicValue(Bulk_Density_Indices[idx], box_index); //g/cm^3

                        const double added_P = scoreboard->GetCharacteristicValue(Added_P_Indices[idx], box_index); //µgP
                        const ProcessTime_t crop_sowing = static_cast<ProcessTime_t>(PROCESS_HOURS(crop_sowing_sa->GetValue())); //time from the start of the simulation that the crop was sown, convert from hours to seconds

                        //VMD At this stage time does not start from 0/0/0 0:0:0, but from a specified date, so need to subtract the start time to
                        //get the time since the start of the simulation.
                        const ProcessTime_t current_time = time - action->GetPostOffice()->GetStart();

                        if ((added_P > 0) && (current_time > (crop_sowing + (static_cast<ProcessTime_t>(PROCESS_DAYS(2)))))) //Time has passed 48h, so some of the Placed P (fertiliser) moves into the fixed P pool
                        {
                            labileSolidPhaseP = scoreboard->GetCharacteristicValue(Labile_Solid_Phase_P_Indices[idx], box_index); //µg
                            double recalcitrantFrac = scoreboard->GetCharacteristicValue(Recalcitrant_P_Indices[idx], box_index); //Fraction of the added fertiliser P that moves into a recalcitrant non-available pool
                            double fertFracSolid = scoreboard->GetCharacteristicValue(FertFrac_Solid_Indices[idx], box_index);
                            const double fertSolidPhaseP = fertFracSolid * labileSolidPhaseP;
                            const double newFertSolidPhaseP = (1 - recalcitrantFrac) * fertSolidPhaseP;
                            labileSolidPhaseP = labileSolidPhaseP - fertSolidPhaseP + newFertSolidPhaseP;
                            fertFracSolid = newFertSolidPhaseP / labileSolidPhaseP;

                            scoreboard->SetCharacteristicValue(Labile_Solid_Phase_P_Indices[idx], labileSolidPhaseP, box_index);
                            scoreboard->SetCharacteristicValue(Added_P_Indices[idx], 0, box_index);
                            scoreboard->SetCharacteristicValue(FertFrac_Solid_Indices[idx], fertFracSolid, box_index);
                            scoreboard->SetCharacteristicValue(Total_Labile_P_Indices[idx], ((liquidPhaseP + labileSolidPhaseP) / (soil_density * thisVolume)), box_index);// µg/g (total amount of labile P in soil)
                        }

                        //Since the water content of the box will have changed since the last solid:liquid partition was calculated, need to recalculate
                        //otherwise cannot solve for the partition after P uptake.
                        double totalLabileP = labileSolidPhaseP + liquidPhaseP;
                        double fertFracLiquid = scoreboard->GetCharacteristicValue(FertFrac_Liquid_Indices[idx], box_index);
                        // MSA Unused...
                        //const double fertFracSolid = scoreboard->GetCharacteristicValue(FertFrac_Solid_Indices[idx],box_index);
                        CalculateSolidLiquidPartition(action, labileSolidPhaseP, totalLabileP, box_index, vo, thisProportion);

                        liquidPhaseP = scoreboard->GetCharacteristicValue(Liquid_Phase_P_Indices[idx], box_index); // µg (amount of P in solution)
                        labileSolidPhaseP = scoreboard->GetCharacteristicValue(Labile_Solid_Phase_P_Indices[idx], box_index); // µg (amount of labile solid-phase P in soil)    
                        double P_concentration = liquidPhaseP / (waterContent * thisVolume); //µgP/cm3 H2O                       

                        const double totalRootLength = root_length_sa->GetValue(box_index, idx);

                        if (totalRootLength <= 0.0000000001) continue; // Insufficient root length. Go to next box subsection.
                        // (else) calculate the AMOUNT (µg) of phosphorus taken up

                        //const double absorption_power = absorption_power_sa->GetValue();  //cm3H2O/cmroot.s
                        const double absorption_power = absorption_power_sa->GetValue(box_index, idx); //cm3H2O/cmroot.s
                        const double P_util_eff = P_util_efficiency_sa->GetValue();
                        const double freundlichParamA = scoreboard->GetCharacteristicValue(Buffer_Index_Indices[idx], box_index);
                        const double freundlich_param_n = scoreboard->GetCharacteristicValue(Freundlich_n_Indices[idx], box_index);

                        const double partitionCoefficient = freundlichParamA * freundlich_param_n * ProcessUtility::guardedPow(liquidPhaseP, (freundlich_param_n - 1), _logger);
                        const double bufferPower = (soil_density * partitionCoefficient) + waterContent;

                        // For root lengths, [0] = tap root, [1] = first root order, etc
                        const double root_lengths[ROOT_ORDER_COUNT] = { root_length_ro_sa->GetValue(box_index, 0, idx), root_length_ro_sa->GetValue(box_index, 1, idx), root_length_ro_sa->GetValue(box_index, 2, idx), root_length_ro_sa->GetValue(box_index, 3, idx) };
                        const double root_radii[ROOT_ORDER_COUNT] = { root_radius_ro_sa->GetValue(box_index, 0, idx), root_radius_ro_sa->GetValue(box_index, 1, idx), root_radius_ro_sa->GetValue(box_index, 2, idx), root_radius_ro_sa->GetValue(box_index, 3, idx) };
                        const double water_fluxes[ROOT_ORDER_COUNT] = { water_flux_sa->GetValue(box_index, 0, idx), water_flux_sa->GetValue(box_index, 1, idx), water_flux_sa->GetValue(box_index, 2, idx), water_flux_sa->GetValue(box_index, 3, idx) };
                        double delta_amounts[ROOT_ORDER_COUNT] = { 0,0,0,0 };
                        double factors[4] = { 0,0,0,0 };
                        LOG_ERROR_IF((root_lengths[0] < 0) || (root_lengths[1] < 0) || (root_lengths[2] < 0) || (root_lengths[3] < 0));
                        double totalHairPUptake = 0.0;

                        if (m_MODEL_ROOT_HAIRS)//yes model root hairs
                        {
                            const double root_hair_length = root_hair_length_sa->GetValue(); //cm, average length of each hair
                            const double root_hair_radius = root_hair_radius_sa->GetValue(); //cm
                            const double root_hair_density = root_hair_density_sa->GetValue(); //number of hairs per cm of root

                            for (int i = 0; i < ROOT_ORDER_COUNT; ++i) //calculate uptake by root hairs first
                            {
                                if (root_lengths[i] > 0.0 && water_fluxes[i] > 0.0)
                                {
                                    const double tot_root_hairlength_perRO = root_lengths[i] * root_hair_density * root_hair_length; //cm_root * hairs/cm_root * cm/hair
                                    factors[0] = (-2.0 * PI * root_hair_radius * (absorption_power / (2.0 * PI * root_hair_radius)) * (tot_root_hairlength_perRO)) / bufferPower;
                                    factors[1] = ((absorption_power / (2.0 * PI * root_hair_radius)) / water_fluxes[i]);
                                    factors[2] = (root_hair_radius * water_fluxes[i]) / (m_DIFFUSION_COEFF_SOLN * waterContent * impedance_factor * bufferPower);
                                    factors[3] = (1.0 / (ProcessUtility::guardedPow((PI * tot_root_hairlength_perRO / thisVolume), 0.5, _logger))) / root_hair_radius;
                                    delta_amounts[i] = (factors[0] * ((P_concentration) / (factors[1] + (1.0 - factors[1]) * (2.0 / (2.0 - factors[2])) * (((ProcessUtility::guardedPow(factors[3], (2.0 - factors[2]), _logger)) - 1.0) / (ProcessUtility::guardedPow(factors[3], 2.0, _logger) - 1.0)))));
                                    if (!((delta_amounts[i] < 0.0) && (delta_amounts[i] > -1000.0))) { delta_amounts[i] = 0.0; }
                                }
                            }
                            totalHairPUptake = ((delta_amounts[0] + delta_amounts[1] + delta_amounts[2] + delta_amounts[3]) * dT); //-µgP
                            const double root_hair_zoneP = liquidPhaseP + totalHairPUptake; //µgP
                            P_concentration = root_hair_zoneP / (waterContent * thisVolume); //µgP/cm3 H2O 
                        } //IF( model root hairs )

                        for (int i = 0; i < ROOT_ORDER_COUNT; ++i) //now uptake by the roots themselves
                        {
                            if (root_lengths[i] > 0 && water_fluxes[i] > 0)
                            {
                                //Baldwin et al (1973) model to decide what solute concentration of P is around the root surface based on average rhizosphere concentration, see also Dunbabin (2006) (Plant and Soil)
                                factors[0] = P_util_eff * (-2.0 * PI * root_radii[i] * (absorption_power / (2.0 * PI * root_radii[i])) * (root_lengths[i])) / bufferPower;
                                factors[1] = ((absorption_power / (2 * PI * root_radii[i])) / water_fluxes[i]);
                                factors[2] = (root_radii[i] * water_fluxes[i]) / (m_DIFFUSION_COEFF_SOLN * waterContent * impedance_factor * bufferPower);
                                factors[3] = (1.0 / (ProcessUtility::guardedPow((PI * (root_lengths[i] / thisVolume)), 0.5, _logger))) / root_radii[i];
                                delta_amounts[i] = (factors[0] * ((P_concentration) / (factors[1] + (1.0 - factors[1]) * (2.0 / (2.0 - factors[2])) * (((ProcessUtility::guardedPow(factors[3], (2.0 - factors[2]), _logger)) - 1.0) / (ProcessUtility::guardedPow(factors[3], 2.0, _logger) - 1.0)))));
                                if (!((delta_amounts[i] < 0.0) && (delta_amounts[i] > -1000.0))) { delta_amounts[i] = 0.0; }
                            }
                        }
                        const int boxtipcount = 0; //need a way to get this, others use something like scoreboard->GetCharacteristicValue(TipCount_Indices[idx], box_index);
                        double tipExUptake = 0.0;
                        // if boxtipcount != 0 then:
                        //work out how much extra P they take up from labileSolidPhaseP, using the mods provided before the per-box runthrough.

                        const double totalPUptake = ((delta_amounts[0] + delta_amounts[1] + delta_amounts[2] + delta_amounts[3]) * dT) + totalHairPUptake + tipExUptake; //-µgP
                        // MSA 09.11.04	Commenting this out to reduce log spam
                        //DEBUGGER(totalPUptake == 0);

                        // MSA 11.02.05 Vanessa, is it safe to skip all the rest of this block if totalPUptake is 0?
                        // This would be a good little optimisation, I think, because there are a lot of boxes with 0 root length,
                        // particularly early in the simulation...

                        if (totalPUptake != 0)
                        {
                            fertFracLiquid = scoreboard->GetCharacteristicValue(FertFrac_Liquid_Indices[idx], box_index);

                            double total_cumul_phosphorus_uptake = total_cumul_p_uptake_sa->GetValue(); //-µgP, total cumulative uptake by each plant, non_spatial
                            total_cumul_phosphorus_uptake += totalPUptake;
                            total_cumul_p_uptake_sa->SetValue(total_cumul_phosphorus_uptake);

                            double local_uptake = scoreboard->GetCharacteristicValue(LocalPUptake_Indices[idx], box_index); //-µgP, Local uptake from each soil box, by all plants, spatial
                            local_uptake += (totalPUptake);
                            scoreboard->SetCharacteristicValue(LocalPUptake_Indices[idx], local_uptake, box_index);

                            double cumul_fert_uptake = cumul_fert_p_uptake_sa->GetValue(); //-µg Cumulative total fertiliser P uptake by each plant, non-spatial
                            cumul_fert_uptake += totalPUptake * fertFracLiquid;
                            cumul_fert_p_uptake_sa->SetValue(cumul_fert_uptake);

                            local_uptake = local_cumul_p_uptake_sa->GetValue(box_index, idx); //-µg Cumulative local uptake from each box, by each plant, spatial
                            local_uptake = local_uptake + (totalPUptake);
                            local_cumul_p_uptake_sa->SetValue(local_uptake, box_index, idx);

                            local_uptake = phosphorus_uptake_sa->GetValue(box_index, idx); //+µmol Total phosphorus taken up by each plant at each growth time step and each soil volume (spatial)
                            local_uptake = (-(totalPUptake / Mr) + local_uptake); //convert µg to µmol
                            phosphorus_uptake_sa->SetValue(local_uptake, box_index, idx);

                            DEBUGGER((delta_amounts[0] > 0.0) || (delta_amounts[1] > 0.0) || (delta_amounts[2] > 0.0) || (delta_amounts[3] > 0.0))

                                //05/09/2002 VMD Now have taken up some of the liquid phase P, need to re-equilabrate with the solid phase P
                                //Some more solid phase P will go into solution, so the new liquid phase P will be something greater than the old value minus the total P uptake
                                //A differencing scheme is used so that continue to halve the interval totalPUptake until we actually solve for a liquid and solid phase
                                //P that sum to the new total labile P


                                totalLabileP = labileSolidPhaseP + liquidPhaseP + totalPUptake;
                            if (nutr_rnw > 1e-6) //roots growing in a free draining sandy soil with nutrient solution added frequently to maintain water and nutrient content
                            {
                                totalLabileP = labileSolidPhaseP + liquidPhaseP; //no depletion of P in these simulatoins
                            }
                            DEBUGGER(!((totalLabileP < 8000.0) && (totalLabileP >= 0.0)));
                            CalculateSolidLiquidPartition(action, labileSolidPhaseP, totalLabileP, box_index, vo, thisProportion);
                        } // end if(totalPUptake!=0)
                    }//roots are growing in soil
                    else //roots growing in nutrient solution
                    {
                        ProcessTime_t dT_soln = time - m_time_of_prev_solution_change;

                        if (dT_soln > nutr_soln_rnw)
                        {
                            double labileSolidPhaseP = 0; //µgP labile P in soil
                            scoreboard->SetCharacteristicValue(Labile_Solid_Phase_P_Indices[idx], labileSolidPhaseP, box_index);// µg (amount of labile P in soil)

                            double renew_labile_P = scoreboard->GetCharacteristicValue(Renew_Labile_P_Indices[idx], box_index); // µg/mL = µg/cm^3 water (initial total amount of labile P)
                            double liquid_phase_P_conc = renew_labile_P / Mr; // µmolP/cm^3 water (concentration of P in solution)
                            scoreboard->SetCharacteristicValue(Liquid_Phase_P_Conc_Indices[idx], liquid_phase_P_conc, box_index); // µmolP/cm^3 water (concentration of P in solution)

                            const double waterContent = scoreboard->GetCharacteristicValue(Water_Content_Indices[idx], box_index);
                            double liquidPhaseP = renew_labile_P * thisVolume * waterContent; //µgP in solution
                            scoreboard->SetCharacteristicValue(Liquid_Phase_P_Indices[idx], liquidPhaseP, box_index); // µg (amount of P in solution)        

                            if (box_index == (BOX_COUNT - 1))
                            {
                                m_time_of_prev_solution_change = time;
                            }
                        } //if(dT_soln > nutr_soln_rnw)

                        double liquidPhaseP = scoreboard->GetCharacteristicValue(Liquid_Phase_P_Indices[idx], box_index); // µg (amount of P in solution)
                        const double totalRootLength = root_length_sa->GetValue(box_index, idx);

                        if (totalRootLength <= 0.0000000001 || liquidPhaseP <= 0.0)
                        {
                            continue; // Insufficient root length and/or liquid phase P. Go to next box.
                        }
                        // (else) calculate the AMOUNT (µg) of phosphorus taken up

                        const double P_concentration = liquidPhaseP / thisVolume; //µgP/cm3 H2O       
                        //const double absorption_power = absorption_power_sa->GetValue();  ////cm3H2O/cmroot.s
                        const double absorption_power = absorption_power_sa->GetValue(box_index, idx); ////cm3H2O/cmroot.s
                        const double waterContent = 1.0;
                        const double bufferPower = 1.0;


                        // For root lengths, [0] = tap root, [1] = first root order, etc
                        const double root_lengths[ROOT_ORDER_COUNT] = { root_length_ro_sa->GetValue(box_index, 0, idx), root_length_ro_sa->GetValue(box_index, 1, idx), root_length_ro_sa->GetValue(box_index, 2, idx), root_length_ro_sa->GetValue(box_index, 3, idx) };
                        const double root_radii[ROOT_ORDER_COUNT] = { root_radius_ro_sa->GetValue(box_index, 0, idx), root_radius_ro_sa->GetValue(box_index, 1, idx), root_radius_ro_sa->GetValue(box_index, 2, idx), root_radius_ro_sa->GetValue(box_index, 3, idx) };
                        const double water_fluxes[ROOT_ORDER_COUNT] = { water_flux_sa->GetValue(box_index, 0, idx), water_flux_sa->GetValue(box_index, 1, idx), water_flux_sa->GetValue(box_index, 2, idx), water_flux_sa->GetValue(box_index, 3, idx) };
                        double delta_amounts[ROOT_ORDER_COUNT] = { 0,0,0,0 };
                        double factors[ROOT_ORDER_COUNT * ROOT_ORDER_COUNT] = { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 };
                        LOG_ERROR_IF((root_lengths[0] < 0.0) || (root_lengths[1] < 0.0) || (root_lengths[2] < 0.0) || (root_lengths[3] < 0.0));


                        for (int i = 0; i < ROOT_ORDER_COUNT; ++i)
                        {
                            if (root_lengths[i] > 0.0 && water_fluxes[i] > 0.0)
                            {
                                factors[i * ROOT_ORDER_COUNT] = (-2.0 * PI * root_radii[i] * (absorption_power / (2.0 * PI * root_radii[i])) * (root_lengths[i])) / bufferPower;
                                factors[i * ROOT_ORDER_COUNT + 1] = ((absorption_power / (2.0 * PI * root_radii[i])) / water_fluxes[i]);
                                factors[i * ROOT_ORDER_COUNT + 2] = (root_radii[i] * water_fluxes[i]) / (m_DIFFUSION_COEFF_SOLN * waterContent * bufferPower);
                                factors[i * ROOT_ORDER_COUNT + 3] = (1.0 / (ProcessUtility::guardedPow((PI * (root_lengths[i] / thisVolume)), 0.5, _logger))) / root_radii[i];
                                delta_amounts[i] = (factors[i * ROOT_ORDER_COUNT] * ((P_concentration) / (factors[i * ROOT_ORDER_COUNT + 1] + (1.0 - factors[i * ROOT_ORDER_COUNT + 1]) * (2.0 / (2.0 - factors[i * ROOT_ORDER_COUNT + 2])) * (((ProcessUtility::guardedPow(factors[i * ROOT_ORDER_COUNT + 3], (2.0 - factors[i * ROOT_ORDER_COUNT + 2]), _logger)) - 1.0) / (ProcessUtility::guardedPow(factors[i * ROOT_ORDER_COUNT + 3], 2.0, _logger) - 1.0)))));
                                if (!((delta_amounts[i] < 0.0) && (delta_amounts[i] > -1000.0))) { delta_amounts[i] = 0.0; }
                            }
                        }

                        const double totalPUptake = ((delta_amounts[0] + delta_amounts[1] + delta_amounts[2] + delta_amounts[3]) * dT); //-µgP
                        // MSA 09.11.04	Commenting this out to reduce log spam
                        //DEBUGGER(totalPUptake == 0);

                        // MSA 11.02.05 Vanessa, is it safe to skip all the rest of this block if totalPUptake is 0?
                        // This would be a good little optimisation, I think, because there are a lot of boxes with 0 root length,
                        // particularly early in the simulation...

                        if (totalPUptake != 0)
                        {
                            liquidPhaseP += totalPUptake; //µgP
                            scoreboard->SetCharacteristicValue(Liquid_Phase_P_Indices[idx], liquidPhaseP, box_index); // µg (amount of P in solution)    
                            double liquid_phase_P_conc = liquidPhaseP / (waterContent * thisVolume * Mr); // µmolP/cm^3 water (concentration of P in solution)
                            scoreboard->SetCharacteristicValue(Liquid_Phase_P_Conc_Indices[idx], liquid_phase_P_conc, box_index); // µmolP/cm^3 water (concentration of P in solution)

                            double total_cumul_phosphorus_uptake = total_cumul_p_uptake_sa->GetValue(); //-µgP, total cumulative uptake by each plant, non_spatial
                            total_cumul_phosphorus_uptake += totalPUptake;
                            total_cumul_p_uptake_sa->SetValue(total_cumul_phosphorus_uptake);

                            double local_uptake = scoreboard->GetCharacteristicValue(LocalPUptake_Indices[idx], box_index); //-µgP, Local uptake from each soil box, by all plants, spatial
                            local_uptake += (totalPUptake);
                            scoreboard->SetCharacteristicValue(LocalPUptake_Indices[idx], local_uptake, box_index);

                            local_uptake = local_cumul_p_uptake_sa->GetValue(box_index, idx); //-µg Cumulative local uptake from each box, by each plant, spatial
                            local_uptake = local_uptake + (totalPUptake);
                            local_cumul_p_uptake_sa->SetValue(local_uptake, box_index, idx);

                            local_uptake = phosphorus_uptake_sa->GetValue(box_index, idx); //+µmol Total phosphorus taken up by each plant at each growth time step and each soil volume (spatial)
                            local_uptake = (-(totalPUptake / Mr) + local_uptake); //convert µg to µmol
                            phosphorus_uptake_sa->SetValue(local_uptake, box_index, idx);

                            DEBUGGER((delta_amounts[0] > 0.0) || (delta_amounts[1] > 0.0) || (delta_amounts[2] > 0.0) || (delta_amounts[3] > 0.0))
                        }
                    } //roots growing in nutrient solution

                    SUBSECTION_ITER_WITH_PROPORTIONS_END
                } //for(BoxIndex box_index = 0; box_index < BOX_COUNT; ++box_index)
            } //for ( SharedAttributeCluster::const_iterator cluster_iter = cluster.begin();

            return (return_value);
        } //long int Phosphorus::UptakeP(ProcessActionDescriptor *action)


        /* ***************************************************************************************************************** */

        double Phosphorus::CalculatePhosphateMineralisation(ProcessActionDescriptor* action, const ProcessTime_t& dT, const double& organic_p_fraction, const ProcessTime_t& organic_p_time)
        {
            // VMD This routine calculates organic P mineralisation rates as taken from field measurements
            Use_ReturnValue;
            Use_Scoreboard;
            Use_VolumeObjectCoordinator;

            const double apase_multiplier = saApaseMultiplier->GetValue(); // [[WAS Get_Concentration_Of_Box(Apase_Multiplier_Index,1, 1, 1);]]

            for (BoxIndex box_index = 0; box_index < BOX_COUNT; ++box_index)
            {
                const double box_volume = Get_BoxIndex_Volume(box_index);

                SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                    const double this_volume = box_volume * thisProportion;

                const double bulk_density = scoreboard->GetCharacteristicValue(Bulk_Density_Indices[idx], box_index);
                double organicP = scoreboard->GetCharacteristicValue(OrganicP_Indices[idx], box_index); //µg/g

                //default values assume 0.3% is mineralised over 10 months of the year - needs to be adjusted for each new soil type
                // MSA 11.03.10 Updated this code to use the PROCESS_YEARS macro. What does that 10 months thing mean then?
                // was:
                //const double mineralisation_rate = (apase_multiplier*organicP*organic_p_fraction*12) / static_cast<double>(organic_p_time*365*24*3600); //µg/g/s
                // now:
                const double mineralisation_rate = (apase_multiplier * organicP * organic_p_fraction * 12.0) / static_cast<double>(organic_p_time * PROCESS_YEARS(1)); //µg/g/s
                organicP -= (mineralisation_rate * dT); //µg/g
                scoreboard->SetCharacteristicValue(OrganicP_Indices[idx], organicP, box_index);

                double liquid_phase_P = scoreboard->GetCharacteristicValue(Liquid_Phase_P_Indices[idx], box_index); // µg (amount of P in solution)
                double fertfrac_liquid = scoreboard->GetCharacteristicValue(FertFrac_Liquid_Indices[idx], box_index);
                const double new_liquidP = (mineralisation_rate * bulk_density * this_volume * dT);
                fertfrac_liquid = ((liquid_phase_P + new_liquidP) == 0) ? 0.0 : (liquid_phase_P * fertfrac_liquid) / (liquid_phase_P + new_liquidP);
                liquid_phase_P += new_liquidP; //µg
                scoreboard->SetCharacteristicValue(Liquid_Phase_P_Indices[idx], liquid_phase_P, box_index); // µg (amount of P in solution)
                scoreboard->SetCharacteristicValue(FertFrac_Liquid_Indices[idx], fertfrac_liquid, box_index); // µg (amount of P in solution)

                SUBSECTION_ITER_WITH_PROPORTIONS_END
            }

            return (return_value);
        }

        /* ***************************************************************************************************************** */

        long int Phosphorus::LeachP(ProcessActionDescriptor* action)
        {
            Use_ReturnValue;
            // This should not be called if we are running a nutrient solution simulation.
            if (m_IS_NUTRIENT_SOLUTION)
            {
                // MSA 10.12.07 Should we throw an assertion failure, or just return without doing anything?
                RmAssert(false, "Error! Phosphorus leaching (due to rainfall) is being attempted in a nutrient solution simulation. Please check RainfallEvents.xml and ensure no >0 rainfall events are present. Dismiss this message box to continue (no leaching will be performed).");
                return return_value;
            }

            Use_Scoreboard;
            const BoundaryArray& Y_boundaries = m_boundarySet->Y_boundaries;
            const BoundaryArray& Z_boundaries = m_boundarySet->Z_boundaries;
            Use_VolumeObjectCoordinator;

            // Iterate column-wise, because we are leaching downwards.
            for (long y = 1; y <= NUM_Y; ++y)
            {
                for (long x = 1; x <= NUM_X; ++x)
                {
                    // Whole-column counters of P leached. Initialise to 0.
                    std::vector<double> P_leached;
                    for (size_t i = 0; i <= VO_COUNT; ++i)
                    {
                        P_leached.push_back(0.0);
                    }

                    for (long z = 1; z <= NUM_Z; ++z)
                    {
                        const BoxIndex thisBox = scoreboard->GetBoxIndex(x, y, z);
                        const BoxIndex nextBox = z == NUM_Z ? thisBox : scoreboard->GetBoxIndex(x, y, z + 1);
                        const double box_height = Get_Layer_Bottom(z) - Get_Layer_Top(z);

                        // MSA 11.03.25 Simplifying this method further; essentially reverting it almost to pre-barrier modelling functionality.
                        // We still handle each pair of boxes on a per-subsection basis, but don't bother checking for transfer restriction.
                        // As with Nitrate::LeachN(), we let the barrier modelling water movement restriction in Water
                        // handle the leaching restriction here, because nutrients are leached by water movement.

                        SUBSECTION_ITER_WITH_TO_AND_FROM_BOXES_AND_PROPORTIONS_BEGIN

                            // If there is no volume available to transfer out of or into, naturally we can't do any leaching.
                            if (thisProportion <= 0.0 || nextProportion <= 0.0) continue;

                        double thisWaterIn = scoreboard->GetCharacteristicValue(Rained_Amount_Indices[idx], thisBox);

                        if ((thisWaterIn) < 0.01)
                        {
                            z = NUM_Z + 1; // Break out of this z-loop; go to next value of x.
                            break;
                        }

                        const double thisFieldCapacity = scoreboard->GetCharacteristicValue(Drained_Upper_Limit_Indices[idx], thisBox);
                        double thisLabileSolidPhaseP = scoreboard->GetCharacteristicValue(Labile_Solid_Phase_P_Indices[idx], thisBox);
                        double thisLiquidPhaseP = scoreboard->GetCharacteristicValue(Liquid_Phase_P_Indices[idx], thisBox);
                        double thisWaterContent = scoreboard->GetCharacteristicValue(Water_Content_Indices[idx], thisBox);
                        double thisWaterOut = z < NUM_Z ? scoreboard->GetCharacteristicValue(Rained_Amount_Indices[idx], nextBox)
                            : scoreboard->GetCharacteristicValue(Rain_Out_Bottom_Indices[idx], thisBox); //cm of water


  //work out how much will leach down to the next cell

  // MSA assume thisWaterOut is uniformly distributed throughout this box subsection
  // Note that thisWaterOut is the amount restricted to the current subsection, so we shouldn't need to apply any further modifiers.
                        thisWaterContent += (thisWaterOut / box_height);

                        // MSA TODO Furthermore, do we need to ascertain whether or not a restricted lower box will limit leaching?

                        if (z < NUM_Z)
                        {
                            if ((thisLabileSolidPhaseP / thisProportion) > 10.0)
                            {
                                const double thisTotalLabileP = thisLabileSolidPhaseP + thisLiquidPhaseP;
                                CalculateSolidLiquidPartition(action, thisLabileSolidPhaseP, thisTotalLabileP, thisBox, vo, thisProportion);
                                thisLabileSolidPhaseP = scoreboard->GetCharacteristicValue(Labile_Solid_Phase_P_Indices[idx], thisBox); //µgP 
                                thisLiquidPhaseP = scoreboard->GetCharacteristicValue(Liquid_Phase_P_Indices[idx], thisBox); //µgP (in solution)
                            } //if((thisLabileSolidPhaseP/thisProportion) > 10)

                            thisLiquidPhaseP += P_leached[idx];
                            RmAssert(thisLiquidPhaseP >= 0.0, "liquid phase P <0!");
                            scoreboard->SetCharacteristicValue(Liquid_Phase_P_Indices[idx], thisLiquidPhaseP, thisBox);
                        } //if(z<NUM_Z)
                        else
                        {
                            if ((thisLabileSolidPhaseP / thisProportion) > 10.0)
                            {
                                // Gotta add the total remaining P_leached to this bottom box in the column.
                                const double thisTotalLabileP = thisLabileSolidPhaseP + thisLiquidPhaseP + P_leached[idx];
                                CalculateSolidLiquidPartition(action, thisLabileSolidPhaseP, thisTotalLabileP, thisBox, vo, thisProportion);
                                thisLiquidPhaseP = scoreboard->GetCharacteristicValue(Liquid_Phase_P_Indices[idx], thisBox); //µgP (in solution)
                                RmAssert(thisLiquidPhaseP >= 0.0, "liquid phase P <0!");
                                thisLabileSolidPhaseP = scoreboard->GetCharacteristicValue(Labile_Solid_Phase_P_Indices[idx], thisBox); //µgP 
                            } //if((thisLabileSolidPhaseP/thisProportion) > 10)
                            else
                            {
                                thisLiquidPhaseP += P_leached[idx];
                                scoreboard->SetCharacteristicValue(Liquid_Phase_P_Indices[idx], thisLiquidPhaseP, thisBox);
                            }
                            thisWaterOut = 0.0;
                            thisWaterIn = 0.0;
                        } //else if(z==NUM_Z)

                        //work out the P content of the cell that has lost P to leaching
                        if (thisWaterIn != 0.0)
                        {
                            P_leached[idx] = (thisWaterOut == 0) ? 0.0 : (0.8 * thisLiquidPhaseP * (thisWaterOut / (thisWaterOut + (thisFieldCapacity * box_height))));

                            RmAssert(P_leached[idx] >= 0, "leached P <0!");

                            const double totalLabileP = thisLabileSolidPhaseP + thisLiquidPhaseP - P_leached[idx];
                            CalculateSolidLiquidPartition(action, thisLabileSolidPhaseP, totalLabileP, thisBox, vo, thisProportion);
                        } //if(water_in != 0)

                        SUBSECTION_ITER_WITH_TO_AND_FROM_BOXES_AND_PROPORTIONS_END
                    }// for(long z=1;z<=NUM_Z; ++z)
                } //for (long x=1; x<=NUM_X; ++x)
            } //for (long y=1; y<=NUM_Y; ++y)

            return (return_value);
        } //long int Phosphorus::LeachP(ProcessActionDescriptor *action)
    } /* namespace rootmap */
    
