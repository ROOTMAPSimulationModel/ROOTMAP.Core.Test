#include "simulation/process/plant/Plant.h"
#include "simulation/process/plant/PlantCoordinator.h"
#include "simulation/process/common/Process_Dictionary.h"
#include "simulation/process/common/Process.h"
#include "simulation/process/modules/Phosphorus.h"
#include "simulation/process/common/ProcessCoordinator.h"
#include "simulation/process/shared_attributes/SharedAttribute.h"
#include "simulation/process/modules/VolumeObjectCoordinator.h"
#include "simulation/process/modules/VolumeObject.h"

#include "simulation/scoreboard/Scoreboard.h"
#include "simulation/scoreboard/ScoreboardCoordinator.h"
#include "core/scoreboard/BoundaryArray.h"

#include "core/utility/Utility.h"
#include "core/common/RmAssert.h"


namespace rootmap
{
    using Utility::CSRestrict;

#define LOG_ERROR_IF(s) if ( (s) ) { RootMapLogError(LOG_LINE "Encountered error in logic : " #s); }

    static const double MAX_FIX_ALLOC = 0.07;
    static const double MIN_FIX_ALLOC = 0.04;
    //allocation factor describes the partitioning of assimilate in response to N, nonNP and P supply to the plant
    static const double ALLOC_FACTOR = 0.5;
    //The plant has a finite storage capacity for N resources
    static const double N_STORAGE_LIMIT = 50.0;
    //The plant has a finite storage capacity for P resources
    static const double P_STORAGE_LIMIT = 0.0;

    static const double SMALL_AS_ZERO = 1e-15;

    static const double UNIT_N_FIX = 10.0;
    static const double UNIT_NODULE_COST = 0.0075;
    static const double UNIT_NODULE_GROWTH = 15;


#define Get_Left_Of_X_Layer(layer)              Get_Layer_Left(x)
#define Get_Front_Of_Y_Layer(layer)             Get_Layer_Front(y)
#define Get_Top_Of_Z_Layer(layer)               Get_Layer_Top(z)
#define Get_Right_Of_X_Layer(layer)             Get_Layer_Right(x)
#define Get_Back_Of_Y_Layer(layer)              Get_Layer_Back(y)
#define Get_Bottom_Of_Z_Layer(layer)            Get_Layer_Bottom(z)


    void Plant::InitialiseDRA(ProcessActionDescriptor* action)
    {
        if (!m_plantDRAInitialised)
        {
            Use_ScoreboardCoordinator;
            ScoreboardStratum stratum = ScoreboardStratum(ScoreboardStratum::Soil);
            Scoreboard* scoreboard = scoreboardcoordinator->GetScoreboard(stratum);
            // MSA 09.11.09 Determine the Scoreboard dimensions here, save doing it repeatedly
            Get_Number_Of_Layers(NUM_X, NUM_Y, NUM_Z); // number of layers in each dimension
            BOX_COUNT = scoreboard->GetNumberOfBoxes(); // How many boxes do we have?

            Use_ProcessCoordinator;
            Use_VolumeObjectCoordinator;

            SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Other_Soil_Resources_Indices, "Other Soil Resources");
            SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Liquid_Phase_P_Concentration_Indices, "Liquid Phase P Concentration");
            SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Nitrate_Concentration_Indices, "Nitrate Concentration");
            SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Nitrate_Amount_Indices, "Nitrate Amount");
            SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Internal_Workings_Indices, "Internal Workings");
            SetCharacteristicIndices(processcoordinator, volumeobjectcoordinator, Total_Labile_P_Indices, "Total Labile P");

            m_plantCumulativeRootRes = 0.0;
            m_plantNoduleNumber = 0.0;
            m_plantCumulativeNfix = 0.0;

            m_plantIsLegume = (saPlantIsLegume->GetValue() != 0);

            // MSA 11.03.24 Need to initialise previous time here

            Use_PostOffice;
            m_plantPreviousTimestamp = postoffice->GetStart();
        }
        m_plantDRAInitialised = true;
    }

    /* ********************************************************************************************** */
    // 03/04/2000 VMD
    //
    //7/11/2003 reworked into multiplant/batch mode format
    //This routine allows the root system to respond to the external nitrate, water and phosphorus
    //concentration and the current internal nitrogen, water and phosphorus demand of the plant.  It is based
    //on the ratio of N&P to non-N and non-P resources within the plant, the total amount
    //of resources available within the plant for growth and maintenance processes,
    //and the amount of resource that any portion of the plant can return based
    //on the external resource supply to which it has access.  The response
    //system is applied to each individual plant.
    // 07/07 2010 VMD Reworked to simplify the routine.  Other resources are removed.  Only N and P considered.
    long int Plant::DynamicResourceAllocationToRoots(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_Time;
        Use_ScoreboardCoordinator;
        ScoreboardStratum stratum = ScoreboardStratum(ScoreboardStratum::Soil);
        Scoreboard* scoreboard = scoreboardcoordinator->GetScoreboard(stratum);
        Use_VolumeObjectCoordinator;

        if (!m_plantDRAInitialised)
        {
            InitialiseDRA(action);
        }

        // (Mainly cumulative) variables, intentionally scoped as accessible by the entire method
        double total_root_resource_units = 0;
        double total_inN_res = 0;
        double fix_resource_alloc = 0;
        double plantActualResourceRatio = 1;
        double tot_pot_N_influx = 0;
        double tot_pot_P_influx = 0;
        double total_root_length = 0;
        double total_nitrate_uptake = 0;
        double total_phosphorus_uptake = 0;
        double unit_nitrate_uptake = 0;
        double unit_phosphorus_uptake = 0;
        size_t spatial_sections_with_roots = 0;

        const ProcessTime_t dT = time - m_plantPreviousTimestamp;

        m_plantPreviousTimestamp = time;
        //VMD At this stage time does not start from 0/0/0 0:0:0, but from a specified date, so need to subtract the start time to
        //get the time since the start of the simulation.
        const ProcessTime_t current_time = time - action->GetPostOffice()->GetStart();

        // MSA 11.02.25 Converting this block to static variables. They do not change over the course of the simulation, right?
        // MSA 11.06.13 No, fool, but a SIMULATION and an execution of ROOTMAP are not the same thing.

        //Time in hours for the seeds to germinate after being sown
        const ProcessTime_t time_of_germination = static_cast<ProcessTime_t>(PROCESS_HOURS(saGerminationLag->GetValue())); //converting the germination lag from hours to seconds
        //Time from zero (start of simulation) in hours when the seeds are sown
        const ProcessTime_t time_of_seeding = static_cast<ProcessTime_t>(PROCESS_HOURS(saSeedingTime->GetValue())); //converting the seeding time from hours to seconds
        const ProcessTime_t cotyledon_life = static_cast<ProcessTime_t>(PROCESS_DAYS(saCotyLife->GetValue())); // converting the days of nutrient supply from the cotyledons from days to seconds
        const ProcessTime_t time_of_leafemerge = static_cast<ProcessTime_t>(PROCESS_DAYS(saLeafEmergence->GetValue())); //converting from days to seconds from germination for the 1st true leaf to emerge
        const ProcessTime_t cotyledon_end_time = time_of_seeding + time_of_germination + cotyledon_life; //Time in seconds from the start of the simulation (time zero)when the cotyledons are no longer a source of resources for the plant
        const ProcessTime_t grow_start_time = time_of_seeding + time_of_germination; //Time in seconds from the start of the simulation (time zero)when seed started to grow

        //This routine is used to help calculate the average absorption power.  By knowing this
        //it is then known whether roots in individual boxes have greater or less access to nutrients
        //so their uptake kinetics are up or down regulated.

        //There is a unique Imax decline curve for each plant.  User defines which one of these curves to use.  These curves are derived from a wide range of data in the literature for grain crops.
        //User need to include their own code for crop-specific modelling, or make ImaxDecline = 1 for no decline in uptake kinetics over time.
        const int ImaxDeclineCurve = saImaxDeclineCurve->GetValue();
        const double ImaxDecline = ImaxDeclineCurveFn(action, ImaxDeclineCurve);

        // MSA 11.03.16 These SharedAttributes do not vary by VolumeObject.
        // !!! Or by ScoreboardBox!
        const double P_conc_min = saPConcMin->GetValue(); //µmol/cm3
        const double N_flux_max = saNFluxMax->GetValue(); //µmol/cm/s
        const double P_flux_max = saPFluxMax->GetValue(); //µmol/cm/s
        const double N_uptake_kinetic_cons = saNUptakeKineticCons->GetValue();
        const double P_uptake_kinetic_cons = saPUptakeKineticCons->GetValue();

        //averaging routine to determine mean value for the regulation factor ratio
        for (BoxIndex box_index = 0; box_index < BOX_COUNT; ++box_index)
        {
            SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                const double local_root_length = saTotalRootLength->GetValue(box_index, idx);

            // MSA 11.02.02 Not worrying about checking coincident volumes here, because a logically-zero-volume should not have any root length.

            if (local_root_length <= SMALL_AS_ZERO)
            {
                continue;
            }

            ++spatial_sections_with_roots;

            const double nitrate_conc = (scoreboard->GetCharacteristicValue(Nitrate_Concentration_Indices[idx], box_index)) / (14.0067); //µmolN/cm3H20
            const double phosphate_conc = (scoreboard->GetCharacteristicValue(Liquid_Phase_P_Concentration_Indices[idx], box_index)) / Phosphorus::Mr; // µmolP/cm3H20 (P in solution phase)

            tot_pot_N_influx += (N_flux_max * ImaxDecline * nitrate_conc) / (N_uptake_kinetic_cons + nitrate_conc); //((µmol/cm/s)/(µmol/cm3))*(µmol/cm3H20) = (cm3 H20 / (cm root . s))*(µmol/cm3H20)= µmol/(cm root.s)
            tot_pot_P_influx += (P_flux_max * ImaxDecline * phosphate_conc) / (P_uptake_kinetic_cons + (phosphate_conc - P_conc_min));

            total_nitrate_uptake += saNitrateUptake->GetValue(box_index, idx); //µmolN, from Nitrate.cpp, Total nitrate taken up by each plant at each time step from each soil volume (spatial)
            total_phosphorus_uptake += saPhosphorusUptake->GetValue(box_index, idx); //+µmol Total phosphorus taken up by each plant at each time step and each soil volume (spatial)
            unit_nitrate_uptake += (saNitrateUptake->GetValue(box_index, idx)) / local_root_length; //µmolN/cm
            unit_phosphorus_uptake += (saPhosphorusUptake->GetValue(box_index, idx)) / local_root_length; //µmolP/cm
            total_root_length += local_root_length;

            SUBSECTION_ITER_WITH_PROPORTIONS_END
        } //for(BoxIndex box_index=0; box_index<BOX_COUNT; ++box_index)

        if (spatial_sections_with_roots <= 0)
        {
            return (return_value);
        }

        const double avg_pot_NInflux = tot_pot_N_influx / spatial_sections_with_roots;
        const double avg_pot_PInflux = tot_pot_P_influx / spatial_sections_with_roots;

        const double avg_unit_nitrate_uptake = unit_nitrate_uptake / spatial_sections_with_roots; //µmolN/cm/box
        const double avg_unit_phosphorus_uptake = unit_phosphorus_uptake / spatial_sections_with_roots; //µmolP/cm/box

        const double tot_n_uptake = total_nitrate_uptake;
        const double tot_p_uptake = total_phosphorus_uptake;

        if (current_time >= cotyledon_end_time)
        {
            if ((total_nitrate_uptake <= 0) && (total_phosphorus_uptake <= 0))
            {
                return (return_value); // No resources for plant growth
            }

            double plantRemainNResourceUnits = saRemNResUnits->GetValue();
            double plantRemainPResourceUnits = saRemPResUnits->GetValue();
            const double plantTargetResourceRatio = saPlantTargetResRatio->GetValue();
            plantActualResourceRatio = saPlantActualResRatio->GetValue();

            total_nitrate_uptake += plantRemainNResourceUnits;
            total_phosphorus_uptake += plantRemainPResourceUnits;

            // This model determines the ratio of the various resource classes.  Currently there are only two resources classes
            // 1) nitrogen (nitrate) resources; 2) P (phosphate) resources.  Initially, all non-nitrogen resources are assumed 
            // to be adequately supplied ie non-limiting, hence are taken up based upon a fixed potential to acquire non-nitrogen resources
            // (including carbon supply from the shoots). This was changed by vmd 31/03/03 so that a gradient of 'other' resources can be considered.  This
            // allows the simulation of different zones of 'fertility' - but this was taken back out again by VMD 07/06/2010


            double total_Nfix = 0; // Initialise N fixation to the non-legume case

            if (m_plantIsLegume) //then are modelling an N2 fixing legume and N2 fixation must be calculated.
            {
                // Depending upon the plant's demand for N, assimilates are provided to support nodule growth.
                // Each nodule is assumed to have the same potential to fix N2.
                total_Nfix = m_plantNoduleNumber * UNIT_N_FIX;
                if ((current_time > (grow_start_time + time_of_leafemerge)) && (0 == m_plantNoduleNumber)) //fixation does not contribute before leaf emergence
                {
                    total_Nfix = 0.5 * UNIT_N_FIX;
                }
                double cumul_Nfix = saCumulNFix->GetValue();
                cumul_Nfix += total_Nfix;
                saCumulNFix->SetValue(cumul_Nfix);
            } //if (m_plantIsLegume)

            total_nitrate_uptake += total_Nfix;

            plantActualResourceRatio = 1; // Reset to value appropriate for zero nitrate & phosphorus uptake
            double total_resource_units = 0; // Initialise to value appropriate for zero nitrate & phosphorus uptake

            if (total_nitrate_uptake != 0 || total_phosphorus_uptake != 0)
            {
                //plant resource ratio > 1.0 is P limited, resource ratio < 1.0 is N limited
                plantActualResourceRatio = (total_nitrate_uptake / total_phosphorus_uptake) / plantTargetResourceRatio;

                total_resource_units = (plantActualResourceRatio > 1.0) ? (total_nitrate_uptake / plantActualResourceRatio) : total_nitrate_uptake;
            }

            //TODO change this routine to make it more generic, make it days or degree-days based?
            double root_res_demand = 1 - (0.60 * (1 - exp((-1.0 / PROCESS_DAYS(200)) * (current_time - grow_start_time))));//root:shoot ratio can be derived from the dynamic wheat module or input by the user
            //root_res_demand = 1;
            plantRemainPResourceUnits = 0;
            plantRemainNResourceUnits = 0;
            total_root_resource_units = root_res_demand * total_resource_units;

            if (m_plantIsLegume) //then are modelling an N2 fixing legume and N2 fixation must be calculated.
            {
                //resources are allocated to fixation based upon the current level of fixation
                //fix_resource_alloc = MIN_FIX_ALLOC * (total_Nfix/(total_nitrate_uptake + total_Nfix)) * total_resource_units * root_res_demand;
                fix_resource_alloc = (total_Nfix / (total_nitrate_uptake + total_Nfix)) * total_resource_units * root_res_demand;
                if (plantActualResourceRatio > 1.0)
                {
                    fix_resource_alloc = (total_Nfix / (total_nitrate_uptake + total_Nfix)) * (total_resource_units / plantActualResourceRatio) * root_res_demand;
                }
                const double nodule_maint_costs = (UNIT_NODULE_COST * m_plantNoduleNumber);
                const double new_nodule_growth = UNIT_NODULE_GROWTH * (fix_resource_alloc - nodule_maint_costs);
                m_plantNoduleNumber += new_nodule_growth;
                if (m_plantNoduleNumber < 0) { m_plantNoduleNumber = 0; }
            } //(plant_is_a_legume == 1)  

            if (plantActualResourceRatio != 1)
            { //plant is N or P resource limited.  Thus need to upregulate the uptake of the N or P resources

                if (plantActualResourceRatio < 1.0) // N supply is most limiting
                {
                    plantRemainPResourceUnits = (total_phosphorus_uptake) * (1 - plantActualResourceRatio);
                    plantRemainNResourceUnits = 0;
                    if (plantRemainPResourceUnits > P_STORAGE_LIMIT)
                    {
                        plantRemainPResourceUnits = P_STORAGE_LIMIT; //The plant is assumed to have a finite capacity to store resources for later use.
                    }
                } // N supply is most limiting
                else //limited by the supply of P
                {
                    total_resource_units = (total_nitrate_uptake / plantActualResourceRatio);
                    plantRemainPResourceUnits = 0;
                    plantRemainNResourceUnits = total_nitrate_uptake * (1 - 1 / plantActualResourceRatio);
                    if (plantRemainNResourceUnits > N_STORAGE_LIMIT)
                    {
                        plantRemainNResourceUnits = N_STORAGE_LIMIT; //The plant is assumed to have a finite capacity to store resources for later use.
                    }
                } //else

                total_root_resource_units = root_res_demand * total_resource_units;

                if (m_plantIsLegume) //then are modelling an N2 fixing legume and N2 fixation must be calculated.
                {
                    const double fixation_demand = (plantActualResourceRatio < 1.0) /* N supply is most limiting */ ? (1 / plantActualResourceRatio) : 1;

                    //resources are allocated to fixation based upon the current level of fixation and the demand for more fixation.
                    // MSA 11.02.03 Note that fixation_demand is capped at 10.
                    fix_resource_alloc = Utility::CSMin(fixation_demand, 10.0) * MIN_FIX_ALLOC * (total_Nfix / (total_nitrate_uptake + total_Nfix)) * total_root_resource_units;

                    const double nodule_maint_costs = (UNIT_NODULE_COST * m_plantNoduleNumber);
                    const double new_nodule_growth = UNIT_NODULE_GROWTH * (fix_resource_alloc - nodule_maint_costs);
                    m_plantNoduleNumber += new_nodule_growth;
                    if (m_plantNoduleNumber < 0) { m_plantNoduleNumber = 0; }
                } //(plant_is_a_legume == 1)        
            } // if plant is N or P resource limited.

            m_plantCumulativeRootRes += total_root_resource_units; //Total resources used by the plant roots over the lifetime of the plant - cost indicator
            total_root_resource_units -= fix_resource_alloc;
            LOG_ERROR_IF(0 >= total_root_resource_units);
            RmAssert(total_root_resource_units > 0, "No root resource units");

            saRemNResUnits->SetValue(plantRemainNResourceUnits);
            saRemPResUnits->SetValue(plantRemainPResourceUnits);
        } //if(current_time >= cotyledon_end_time)

               //Cumulative Variables
        double N_above_avg_RL = 0;
        double P_above_avg_RL = 0;

        //TODO: need to check that the shared attributes are working properly.  Are we actually getting each different plant in here wrt nitrate, phosphorus etc
        for (BoxIndex box_index = 0; box_index < BOX_COUNT; ++box_index)
        {
            SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                // First, check if total root length is insignificant.
                const double local_root_length = saTotalRootLength->GetValue(box_index, idx);
            if (local_root_length <= SMALL_AS_ZERO) //total resources taken up by the root system since the last time the roots grew
            {
                // If insignificant, go to the next box.
                continue;
            }

            RmAssert(thisProportion > 0, "ERROR: Root growth detected in logically-zero volume");

            //assume that there is no compensatory growth in the first few days while cotyledons are supplying resources to the seedling.
            if (current_time < cotyledon_end_time)
            {
                //Early root growth rates
                saSeminalGrowthRate->SetValue(saGrowthRateMax->GetValue(0, 0, idx), box_index, 0, idx);
                saNodalGrowthRate->SetValue(saGrowthRateMax->GetValue(0, 0, idx), box_index, 0, idx);
                saSeminalGrowthRate->SetValue(saGrowthRateMax->GetValue(0, 1, idx), box_index, 1, idx);
                saNodalGrowthRate->SetValue(saGrowthRateMax->GetValue(0, 1, idx), box_index, 1, idx);
                saSeminalGrowthRate->SetValue(saGrowthRateMax->GetValue(0, 2, idx), box_index, 2, idx);
                saNodalGrowthRate->SetValue(saGrowthRateMax->GetValue(0, 2, idx), box_index, 2, idx);
                saSeminalGrowthRate->SetValue(saGrowthRateMax->GetValue(0, 3, idx), box_index, 3, idx);
                saNodalGrowthRate->SetValue(saGrowthRateMax->GetValue(0, 3, idx), box_index, 3, idx);

                //Calculating the absorption power. Convert nitrate concentration from µgN/cm^3 to µmol N/cm^3.
                const double nitrate_conc = (scoreboard->GetCharacteristicValue(Nitrate_Concentration_Indices[idx], box_index)) / (14.0067); //µmolN/cm3H20
                const double phosphate_conc = (scoreboard->GetCharacteristicValue(Liquid_Phase_P_Concentration_Indices[idx], box_index)) / Phosphorus::Mr; // µmolP/cm3H20 (P in solution phase)

                const double N_absorp_pow = (N_flux_max) / (N_uptake_kinetic_cons + nitrate_conc); //Michaelis_Menten kinetics - constitutive absorption power. (µmol/cm/s)/(µmol/cm3) = cm3 H20 / (cm root . s)
                const double P_absorp_pow = (P_flux_max) / (P_uptake_kinetic_cons + (phosphate_conc - P_conc_min)); //Michaelis_Menten kinetics - constitutive absorption power.  (µmol/cm/s)/(µmol/cm3) = cm3 H20 / (cm root . s)

                saNAbsorptionPower->SetValue(N_absorp_pow, box_index, idx);
                saPAbsorptionPower->SetValue(P_absorp_pow, box_index, idx);
            } //if (current_time < cotyledon_end_time)
            else //(current_time >= cotyledon_end_time) No supply of resources from the cotyledons now.
            {
                const double nitrate_conc = (scoreboard->GetCharacteristicValue(Nitrate_Concentration_Indices[idx], box_index)) / (14.0067); //µmolN/cm3H20
                const double phosphate_conc = (scoreboard->GetCharacteristicValue(Liquid_Phase_P_Concentration_Indices[idx], box_index)) / Phosphorus::Mr; // µmolP/cm3H20 (P in solution phase)

                //Absorption power calculated from the constitutive system (ie in response to the external nitrate conc. only).
                const double N_cons_absorp_pow = (N_flux_max * ImaxDecline * nitrate_conc) / (N_uptake_kinetic_cons + nitrate_conc); //((µmol/cm/s)/(µmol/cm3))*(µmol/cm3H20) = (cm3 H20 / (cm root . s))*(µmol/cm3H20)= µmol/(cm root.s)
                if (N_cons_absorp_pow > avg_pot_NInflux) { N_above_avg_RL += local_root_length; }
                const double P_cons_absorp_pow = (P_flux_max * ImaxDecline * phosphate_conc) / (P_uptake_kinetic_cons + (phosphate_conc - P_conc_min));
                if (P_cons_absorp_pow > avg_pot_PInflux) { P_above_avg_RL += local_root_length; }

                const double local_nitrate_uptake = saNitrateUptake->GetValue(box_index, idx);
                const double local_phosphorus_uptake = saPhosphorusUptake->GetValue(box_index, idx);

                // MSA 10.08.30 The local root volume as calculated here is currently unused, and thus has been commented out.
                // Uncomment this block if it becomes necessary later on.
                /*
                double localRootVolume = 0;
                for(size_t i=0; i<ROOT_ORDER_LENGTH; ++i)
                {
                    const double thisOrderRootLength = saRootLengthPerPlantPerRootOrder->GetValue(box_index, i, idx);
                    const double thisOrderRootRadius = saRootRadiusPerPlantPerRootOrder->GetValue(box_index, i, idx);
                    localRootVolume += PI * (pow(thisOrderRootRadius,2)*thisOrderRootLength);
                }
                */

                //have now used all the nitrate, phosphorus and water uptake so reset to zero
                saNitrateUptake->SetValue(0.0, box_index, idx);
                saPhosphorusUptake->SetValue(0.0, box_index, idx);
                saWaterUptake->SetValue(0.0, box_index, idx);

                //Calculating the root growth rates. 

                //The roots in a scoreboard box are allocated a percentage of the total resources available dependent upon the amount of N&P
                //resources the roots in that box can return. 

                double N_resour_alloc0 = 0; // Initialise to appropriate values for tot_n_uptake==0 case
                double N_resour_alloc1 = 0; // Initialise to appropriate values for tot_n_uptake==0 case
                double N_resour_alloc2 = 0; // Initialise to appropriate values for tot_n_uptake==0 case

                if (tot_n_uptake != 0) // NITRATE
                {
                    double plasticity_factor0 = 1; // Initialise to appropriate values for (local_nitrate_uptake/local_root_length) <= avg_unit_nitrate_uptake case
                    double plasticity_factor1 = 1; // Initialise to appropriate values for (local_nitrate_uptake/local_root_length) <= avg_unit_nitrate_uptake case
                    double plasticity_factor2 = 1; // Initialise to appropriate values for (local_nitrate_uptake/local_root_length) <= avg_unit_nitrate_uptake case

                    if ((local_nitrate_uptake / local_root_length) > avg_unit_nitrate_uptake)
                    {
                        const double plasticity_factor = ((local_nitrate_uptake / local_root_length) / avg_unit_nitrate_uptake);
                        const double plasticity_factor0_max = saPlasticityFactorN->GetValue(box_index, 0, idx); //root growth plasticity factor is plant specific, and is input from the xml file.
                        const double plasticity_factor1_max = saPlasticityFactorN->GetValue(box_index, 1, idx);
                        const double plasticity_factor2_max = saPlasticityFactorN->GetValue(box_index, 2, idx);
                        plasticity_factor0 = CSRestrict(plasticity_factor, 1.0, plasticity_factor0_max);
                        plasticity_factor1 = CSRestrict(plasticity_factor, 1.0, plasticity_factor1_max);
                        plasticity_factor2 = CSRestrict(plasticity_factor, 1.0, plasticity_factor2_max);
                    }

                    //Allocate root resources to those roots returning the greatest soil N to the plant
                    if (m_plantIsLegume)
                    {
                        N_resour_alloc0 = (local_root_length / total_root_length) * plasticity_factor0 * ALLOC_FACTOR * (total_root_resource_units - fix_resource_alloc);
                        N_resour_alloc1 = (local_root_length / total_root_length) * plasticity_factor1 * ALLOC_FACTOR * (total_root_resource_units - fix_resource_alloc);
                        N_resour_alloc2 = (local_root_length / total_root_length) * plasticity_factor2 * ALLOC_FACTOR * (total_root_resource_units - fix_resource_alloc);
                    }
                    else //No N2 fixation
                    {
                        N_resour_alloc0 = (local_root_length / total_root_length) * plasticity_factor0 * ALLOC_FACTOR * (total_root_resource_units);
                        N_resour_alloc1 = (local_root_length / total_root_length) * plasticity_factor1 * ALLOC_FACTOR * (total_root_resource_units);
                        N_resour_alloc2 = (local_root_length / total_root_length) * plasticity_factor2 * ALLOC_FACTOR * (total_root_resource_units);
                    }
                } // END NITRATE

                double P_resour_alloc0 = 0; // Initialise to appropriate values for tot_p_uptake==0 case
                double P_resour_alloc1 = 0; // Initialise to appropriate values for tot_p_uptake==0 case
                double P_resour_alloc2 = 0; // Initialise to appropriate values for tot_p_uptake==0 case

                if (tot_p_uptake != 0) // PHOSPHORUS
                {
                    const double down_reg_factor = saDownRegulationFactor->GetValue();
                    double plasticity_factor0 = down_reg_factor; // Initialise to appropriate values for (local_phosphorus_uptake/local_root_length) > avg_unit_phosphorus_uptake case
                    double plasticity_factor1 = down_reg_factor; // Initialise to appropriate values for (local_phosphorus_uptake/local_root_length) > avg_unit_phosphorus_uptake case
                    double plasticity_factor2 = down_reg_factor; // Initialise to appropriate values for (local_phosphorus_uptake/local_root_length) > avg_unit_phosphorus_uptake case

                    if ((local_phosphorus_uptake / local_root_length) > avg_unit_phosphorus_uptake)
                    {
                        const double plasticity_factor = ((local_phosphorus_uptake / local_root_length) / avg_unit_phosphorus_uptake);
                        const double plasticity_factor0_max = saPlasticityFactorP->GetValue(box_index, 0, idx); //root growth plasticity factor is plant specific, and is input from the external file.
                        const double plasticity_factor1_max = saPlasticityFactorP->GetValue(box_index, 1, idx);
                        const double plasticity_factor2_max = saPlasticityFactorP->GetValue(box_index, 2, idx);
                        plasticity_factor0 = CSRestrict(plasticity_factor, 1.0, plasticity_factor0_max);
                        plasticity_factor1 = CSRestrict(plasticity_factor, 1.0, plasticity_factor1_max);
                        plasticity_factor2 = CSRestrict(plasticity_factor, 1.0, plasticity_factor2_max);
                    }

                    //Allocate root resources to those roots returning the greatest P to the plant    
                    if (m_plantIsLegume)
                    {
                        P_resour_alloc0 = (local_root_length / total_root_length) * plasticity_factor0 * ALLOC_FACTOR * (total_root_resource_units - fix_resource_alloc);
                        P_resour_alloc1 = (local_root_length / total_root_length) * plasticity_factor1 * ALLOC_FACTOR * (total_root_resource_units - fix_resource_alloc);
                        P_resour_alloc2 = (local_root_length / total_root_length) * plasticity_factor2 * ALLOC_FACTOR * (total_root_resource_units - fix_resource_alloc);
                    }
                    else //No P fixation
                    {
                        P_resour_alloc0 = (local_root_length / total_root_length) * plasticity_factor0 * ALLOC_FACTOR * (total_root_resource_units);
                        P_resour_alloc1 = (local_root_length / total_root_length) * plasticity_factor1 * ALLOC_FACTOR * (total_root_resource_units);
                        P_resour_alloc2 = (local_root_length / total_root_length) * plasticity_factor2 * ALLOC_FACTOR * (total_root_resource_units);
                    }
                } // END PHOSPHORUS

                total_inN_res += (N_resour_alloc0 + N_resour_alloc1 + N_resour_alloc2);


                if ((N_resour_alloc0 > 0) || (P_resour_alloc0 > 0))
                {
                    const double thisVolume = thisProportion * scoreboard->GetBoxVolume(box_index);
                    // Check the ratio of root length to volume. If it exceeds 40, apply a space limit on root growth (reduce all growth rates to 0).
                    if ((local_root_length / thisVolume) > 40)
                    {
                        for (size_t i = 0; i < ROOT_ORDER_COUNT; ++i)
                        {
                            saSeminalGrowthRate->SetValue(0, box_index, i, idx);
                            saNodalGrowthRate->SetValue(0, box_index, i, idx);
                        }
                    }
                    else
                    {
                        const double resource_allocation[ROOT_ORDER_COUNT] = { P_resour_alloc0 + N_resour_alloc0,
                            P_resour_alloc1 + N_resour_alloc1,
                            P_resour_alloc2 + N_resour_alloc2,
                            P_resour_alloc2 + N_resour_alloc2 };
                        for (size_t i = 0; i < ROOT_ORDER_COUNT; ++i)
                        {
                            const double seminalUnitGrowthRate = saSeminalUnitGwthRte->GetValue(0, i, idx);
                            const double nodalUnitGrowthRate = saNodalUnitGwthRte->GetValue(0, i, idx);
                            const double maxGrowthRate = saGrowthRateMax->GetValue(0, i, idx);

                            double seminalGrowthRate = (seminalUnitGrowthRate * resource_allocation[i]) / (local_root_length * dT);
                            double nodalGrowthRate = (nodalUnitGrowthRate * resource_allocation[i]) / (local_root_length * dT);
                            const double restrictionMinimum = i == 0 ? (maxGrowthRate / 5) : 0;
                            seminalGrowthRate = CSRestrict(seminalGrowthRate, restrictionMinimum, maxGrowthRate);

                            const double total_labileP = (scoreboard->GetCharacteristicValue(Total_Labile_P_Indices[idx], box_index));

                            if ((total_labileP > 1000) && (i > 0))
                            {
                                seminalGrowthRate *= 0.3;
                                nodalGrowthRate *= 0.3;
                            }

                            if (i == 0)
                            {
                                RmAssert(((seminalGrowthRate >= 0) && (seminalGrowthRate < 5.0)), "Root growth rate 0 out of range");
                            }

                            auto prevSeminal = saSeminalGrowthRate->GetValue(box_index, i, idx);
                            auto prevNodal = saNodalGrowthRate->GetValue(box_index, i, idx);
                            if (prevSeminal < seminalGrowthRate)
                            {
                                LOG_DEBUG << "Box " << box_index << ". Seminal growth rate increased from " << prevSeminal << " to " << seminalGrowthRate;
                            }
                            if (prevNodal < nodalGrowthRate)
                            {
                                LOG_DEBUG << "Box " << box_index << ". Nodal growth rate increased from " << prevNodal << " to " << nodalGrowthRate;
                            }
                            saSeminalGrowthRate->SetValue(seminalGrowthRate, box_index, i, idx);
                            saNodalGrowthRate->SetValue(nodalGrowthRate, box_index, i, idx);
                        }
                    }
                } //if ((N_resour_alloc > 0) || (nonNP_resour_alloc > 0) || (P_resour_alloc > 0))
            } //else (current_time >= cotyledon_end_time) No supply of resources from the cotyledons now.

            SUBSECTION_ITER_WITH_PROPORTIONS_END
        } //for(BoxIndex box_index=0; box_index<BOX_COUNT; ++box_index)

        if (current_time >= cotyledon_end_time)
        {
            const double N_root_fraction = (N_above_avg_RL / total_root_length); //fraction of the total root length in which there is upregulated uptake
            const double P_root_fraction = (P_above_avg_RL / total_root_length); //fraction of the total root length in which there is upregulated uptake
            CalculateNutrientUptakePlasticity(action, plantActualResourceRatio, avg_pot_NInflux, avg_pot_PInflux, N_root_fraction, P_root_fraction);

        }

        return (return_value);
    }

    const double Plant::ImaxDeclineCurveFn(ProcessActionDescriptor* action, const int& ImaxDeclineCurve) const
    {
        //There is a unique Imax decline curve for each plant.  User defines which one of these curves to use.  These curves are derived from a wide range of data in the literature for grain crops.
        //User need to include their own code for crop-specific modelling, or make ImaxDecline = 1 for no decline in uptake kinetics over time.
        Use_Time;

        //VMD At this stage time does not start from 0/0/0 0:0:0, but from a specified date, so need to subtract the start time to
        //get the time since the start of the simulation.
        const ProcessTime_t current_time = time - action->GetPostOffice()->GetStart();

        //Time in hours for the seeds to germinate after being sown
        const ProcessTime_t time_of_germination = static_cast<ProcessTime_t>(PROCESS_HOURS(saGerminationLag->GetValue(InvalidBoxIndex))); //converting the germination lag from hours to seconds
        //Time from zero (start of simulation) in hours when the seeds are sown
        const ProcessTime_t time_of_seeding = static_cast<ProcessTime_t>(PROCESS_HOURS(saSeedingTime->GetValue(InvalidBoxIndex))); //converting the seeding time from hours to seconds
        const ProcessTime_t grow_start_time = time_of_seeding + time_of_germination; //Time in seconds from the start of the simulation (time zero)when seed started to grow
        double ImaxDecline = 1;

        const ProcessTime_t timeDiff = current_time - grow_start_time;

        switch (ImaxDeclineCurve)
        {
        case 0:
            //    ImaxDecline = 1; // no decline in Imax with plant age (ImaxDecline is already initialised to this value)
            break;
        case 1:
            if (((timeDiff) / 86400) > 20)
            {
                //if(((timeDiff)/86400)>=30){ImaxDecline = 0.2;}
                //else
                //{
                //ImaxDecline = 2.6 - (0.08*(timeDiff)/86400);
                //}
                if (((timeDiff) / 86400) >= 48) { ImaxDecline = 0.2; }
                else
                {
                    ImaxDecline = 1.5 - (0.027 * (timeDiff) / 86400);
                }
            }
            // else ImaxDecline=1
            break;
        case 2:
            if (((timeDiff) / 86400) > 30)
            {
                if (((timeDiff) / 86400) >= 40) { ImaxDecline = 0.2; }
                else
                {
                    ImaxDecline = 3.4 - (0.08 * (timeDiff) / 86400);
                }
            }
            // else ImaxDecline=1
            break;
        case 3:
            if (((timeDiff) / 86400) > 30)
            {
                if (((timeDiff) / 86400) >= 40) { ImaxDecline = 0.5; }
                else
                {
                    ImaxDecline = 2.5 - (0.05 * (timeDiff) / 86400);
                }
            }
            // else ImaxDecline=1
            break;
        case 4:
            if (((timeDiff) / 86400) > 20)
            {
                if (((timeDiff) / 86400) >= 30) { ImaxDecline = 0.5; }
                else
                {
                    ImaxDecline = 2.0 - (0.05 * (timeDiff) / 86400);
                }
            }
            // else ImaxDecline=1
            break;
        default:
            LOG_ERROR_IF((ImaxDecline > 1) || (ImaxDecline < 0.2));
            RmAssert((ImaxDeclineCurve == 0), "Imax decline curve not selected");
            break;
        } //switch (plant_counter)

        LOG_ERROR_IF((ImaxDecline > 1) || (ImaxDecline < 0.2));
        RmAssert(((ImaxDecline >= 0.2) && (ImaxDecline <= 1.0)), "Imax decline value out of range");

        return (ImaxDecline);
    }


    void Plant::CalculateNutrientUptakePlasticity(ProcessActionDescriptor* action, const double& plantActualResourceRatio, const double& avg_pot_NInflux, const double& avg_pot_PInflux, const double& N_root_fraction, const double& P_root_fraction)
    {
        //In here we calculate whether the potential for nutrient uptake (uptake kinetics) are up- or down-regulated in each local soil volume
        //based on whether the local nutrient concentration in that soil volume is higher or lower than average, and also based on
        //plant demand for the resource.

        Use_VolumeObjectCoordinator;
        Use_ScoreboardCoordinator;
        ScoreboardStratum stratum = ScoreboardStratum(ScoreboardStratum::Soil);
        Scoreboard* scoreboard = scoreboardcoordinator->GetScoreboard(stratum);

        //There is a unique Imax decline curve for each plant.  User defines which one of these curves to use.  These curves are derived from a wide range of data in the literature for grain crops.
        //User need to include their own code for crop-specific modelling, or make ImaxDecline = 1 for no decline in uptake kinetics over time.
        const int ImaxDeclineCurve = saImaxDeclineCurve->GetValue();
        const double ImaxDecline = ImaxDeclineCurveFn(action, ImaxDeclineCurve);

        // These SAs are not spatially variant
        const double P_conc_min = saPConcMin->GetValue(); //µmol/cm3
        const double N_flux_max = saNFluxMax->GetValue(); //µmol/cm.s
        const double P_flux_max = saPFluxMax->GetValue(); //µmol/cm.s
        const double N_uptake_kinetic_cons = saNUptakeKineticCons->GetValue(); //µmol/cm3
        const double P_uptake_kinetic_cons = saPUptakeKineticCons->GetValue(); //µmol/cm3
        const double Nregulation_factor = saNRegulationFactor->GetValue(); //plant specific nutrient plasticity factor input from xml file
        const double Pregulation_factor = saPRegulationFactor->GetValue();

        for (BoxIndex box_index = 0; box_index < BOX_COUNT; ++box_index)
        {
            SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                const double local_root_length = saTotalRootLength->GetValue(box_index, idx);
            if (local_root_length <= SMALL_AS_ZERO)
            {
                continue; // Insignificant root length in this box; skip to the next one.
            }

            //Calculating the absorption power. Convert nitrate concentration from µgN/cm^3 to µmol N/cm^3.
            const double nitrate_conc = (scoreboard->GetCharacteristicValue(Nitrate_Concentration_Indices[idx], box_index)) / (14.0067); //µmolN/cm3H20
            const double phosphate_conc = (scoreboard->GetCharacteristicValue(Liquid_Phase_P_Concentration_Indices[idx], box_index)) / Phosphorus::Mr; // µmolP/cm3H20 (P in solution phase)

            const double N_cons_absorp_pow = (N_flux_max * ImaxDecline) / (N_uptake_kinetic_cons + nitrate_conc); //Michaelis_Menten kinetics - constitutive absorption power. (µmol/cm/s)/(µmol/cm3) = cm3 H20 / (cm root . s)
            const double P_cons_absorp_pow = (P_flux_max * ImaxDecline) / (P_uptake_kinetic_cons + (phosphate_conc - P_conc_min)); //Michaelis_Menten kinetics - constitutive absorption power.  (µmol/cm/s)/(µmol/cm3) = cm3 H20 / (cm root . s)
            double N_flux_ratio = (N_cons_absorp_pow * nitrate_conc) / (avg_pot_NInflux); //Is the local potential influx more or less than the average?
            double P_flux_ratio = (P_cons_absorp_pow * phosphate_conc) / (avg_pot_PInflux); //Is the local potential influx more or less than the average?

            //Local potential flux is equal to the average therefore uptake is based upon concentration only - no up regulation required
            if ((N_flux_ratio < 1.03) && (N_flux_ratio > 0.97)) { N_flux_ratio = 1; }
            if ((P_flux_ratio < 1.03) && (P_flux_ratio > 0.97)) { P_flux_ratio = 1; }
            double plantNUpRegulation = 1;
            double plantPUpRegulation = 1;

            //uptake upregulated (no downregulation), the regulation factor is plant specific and is a value of 1-5 describing where in the upregulation envelope a plant lies.
            if (N_flux_ratio >= 1.03) { plantNUpRegulation = ((4 - (4 * N_root_fraction)) * Nregulation_factor) + 1; }
            if (P_flux_ratio >= 1.03) { plantPUpRegulation = ((4 - (4 * P_root_fraction)) * Pregulation_factor) + 1; }
            LOG_ERROR_IF(plantPUpRegulation < 0); // a check against negative plantPUpRegulation
            RmAssert(plantPUpRegulation >= 0, "Negative Plant P Up Regulation");
            LOG_ERROR_IF(plantNUpRegulation < 0); // a check against negative plantNUpRegulation
            RmAssert(plantNUpRegulation >= 0, "Negative Plant N Up Regulation");

            // Actual absorption power = constitutive absorption power * IFM;
            double actual_N_absorp_pow = N_cons_absorp_pow;
            double actual_P_absorp_pow = P_cons_absorp_pow;

            if (plantActualResourceRatio < 1.0) // N limited
            {
                actual_N_absorp_pow = (1 / plantActualResourceRatio) * plantNUpRegulation * N_cons_absorp_pow;
                actual_P_absorp_pow = P_cons_absorp_pow;
            }
            if (plantActualResourceRatio > 1.0) // P limited
            {
                actual_P_absorp_pow = (plantActualResourceRatio)* plantPUpRegulation * P_cons_absorp_pow;
            }

            if (actual_N_absorp_pow > 0.01) { actual_N_absorp_pow = N_cons_absorp_pow; }
            if (actual_P_absorp_pow > 0.01) { actual_P_absorp_pow = P_cons_absorp_pow; }
            if (actual_N_absorp_pow < 0) { actual_N_absorp_pow = N_cons_absorp_pow; }
            if (actual_P_absorp_pow < 0) { actual_P_absorp_pow = P_cons_absorp_pow; }

            saNAbsorptionPower->SetValue(actual_N_absorp_pow, box_index, idx);
            saPAbsorptionPower->SetValue(actual_P_absorp_pow, box_index, idx);

            SUBSECTION_ITER_WITH_PROPORTIONS_END
        } //for(BoxIndex box_index=0; box_index<BOX_COUNT; ++box_index)
    }
} /* namespace rootmap */
