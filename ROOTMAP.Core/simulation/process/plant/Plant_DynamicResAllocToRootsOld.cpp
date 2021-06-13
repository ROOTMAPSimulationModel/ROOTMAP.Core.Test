#include "BoostPrecomp.h"

#include "simulation/process/plant/Plant.h"
#include "simulation/process/plant/PlantCoordinator.h"
//#include "Scoreboard.h"
#include "simulation/process/common/Process_Dictionary.h"
#include "simulation/process/common/ProcessCoordinator.h"
#include "simulation/process/shared_attributes/SharedAttribute.h"

#include "core/utility/Utility.h"
#include "simulation/common/BoostPrecomp.h"

namespace rootmap
{

using Utility::CSRestrict;

#define L_NON_NP_RESOURCE_SIZE 3.0e-4 //Used in root response routine to describe a fixed non_N resource aquisition per unit of tap and 1st order root length
#define H_NON_NP_RESOURCE_SIZE 4.5e-5 //Used in root response routine to describe a fixed non_N resource aquisition per unit of high order root length

#define    unit_Ngwth_rte0 5000
#define    unit_Ngwth_rte1 6000
#define    unit_Ngwth_rte2 1000
#define    unit_Ngwth_rte3 600
#define    unit_Pgwth_rte0 4000
#define    unit_Pgwth_rte1 6000
#define    unit_Pgwth_rte2 1000
#define    unit_Pgwth_rte3 600
#define    unit_nonNPgwth_rte0 5000
#define    unit_nonNPgwth_rte1 500
#define    unit_nonNPgwth_rte2 100
#define    unit_nonNPgwth_rte3 600
#define    unit_branch_int0 0.5
#define    unit_Nbranch_int1 85000
#define    unit_nNbranch_int1 30000
#define    unit_branch_int2 0.125
#define    unit_branch_int3 0.0625
#define    unit_branch_lag0 72
#define    unit_branch_lag1 96
#define    unit_branch_lag2 96
#define    unit_branch_lag3 96
#define    unit_Nfix 0.05
#define    unit_nodule_cost 0.0075
#define    unit_nodule_growth 9
#define       plant_is_a_legume 1    //1 denotes yes, 0 denotes no



//µmols/cm^2.s.  Maximum nitrate flux rate - Michaelis-Menten.
static const double_cs N_flux_max = 5.3e-7; 
//µmols/cm^2.s.  Maximum P flux rate - Michaelis-Menten.
static const double_cs P_flux_max = 64.3e-14; 
//1.188 for L. angustifolius and 1.151 µmols/cm^3 for L. pilosus.  Michaelis-Menten kinetic constant.
static const double_cs N_uptake_kinetic_cons = 0.1188; 
//µmols/cm^3 Taken from Silberbush and Barber (1983) for P uptake by soybean.  Michaelis-Menten kinetic constant.
static const double_cs P_uptake_kinetic_cons = 0.0055; 
//µmols/cm^3 Taken from Silberbush and Barber (1983) for P uptake by soybean 
static const double_cs P_conc_min = 2.0e-4; 

static const double_cs Ndown_regul_factor = 1.200; 

static const double_cs nonNPdown_regul_factor = 3.0; 

static const double_cs Pdown_regul_factor = 1.200;

static const double_cs max_fix_alloc = 0.07;
//allocation factor describes the partitioning of assimilate in response to N, nonNP and P supply to the plant
static const double_cs alloc_factor = 0.3333333333333333;  
//Root system demand for a share of the total resources before emergence of first true leaf
static const double_cs early_root_res_demand = 0.95; 
//The plant has a finite storage capacity for Non-N resources
static const long int nonNP_storage_limit = 4000; 
//The plant has a finite storage capacity for N resources
static const long int N_storage_limit = 4000;  
//The plant has a finite storage capacity for P resources
static const long int P_storage_limit = 4000;  
//average time over which the cotyledons can supply nutrients to the seedling
static const long int mean_coty_life = PROCESS_DAYS(35); // 3024000;  
//Typical time of pod and seed growth phase.
static const long int anthesis = PROCESS_DAYS(199); // 17193600; 
//Early germination
static const long int early_germination_time = PROCESS_DAYS(7); // 604800; 
//Time after which resources are increasingly allocated to the shoots
static const long int resource_shift_time = PROCESS_DAYS(46); // 3974400; 
//Time of first true leaf emerging (2073600 = PROCESS_DAYS(24))
static const long int leaf_emergence = PROCESS_DAYS(14); // 1209600; 

// L. angustifolius seed size = 56.4,
// L. pilosus seed size = 265,
// intermediate seed size = 120.6
static const double_cs seed_size_L_angustifolius = 56.4;
static const double_cs seed_size_L_pilosus = 265.0;
static const double_cs seed_size_intermediate = 120.6;


//
// Scoreboard Indices
static long int rootlength_high_index = -1;
static long int phosphate_concentration_index = -1;
static long int nitrate_amount_index = -1;
static long int Water_Content_Index = -1;    
static long int internal_workings_index = -1;
static long int N_absorption_power_index = -1;
static long int P_absorption_power_index = -1;
static long int tot_root_length_index1 = -1;
static long int tot_root_length_index2 = -1;
static long int tot_root_length_index3 = -1;
static long int tot_root_length_index4 = -1;


//#define Use_Layers  LHorizonArray *x_layers = scoreboardcoordinator->GetLayerList(vSide, pVolumeType); \
//                    LHorizonArray *y_layers = scoreboardcoordinator->GetLayerList(vFront, pVolumeType); \
//                    LHorizonArray *z_layers = scoreboardcoordinator->GetLayerList(vTop, pVolumeType)
#define Get_Left_Of_X_Layer(layer)              Get_Layer_Left(x)
#define Get_Front_Of_Y_Layer(layer)             Get_Layer_Front(y)
#define Get_Top_Of_Z_Layer(layer)               Get_Layer_Top(z)
#define Get_Right_Of_X_Layer(layer)             Get_Layer_Right(x)
#define Get_Back_Of_Y_Layer(layer)              Get_Layer_Back(y)
#define Get_Bottom_Of_Z_Layer(layer)            Get_Layer_Bottom(z)




void Plant::InitialiseDRA(ProcessActionDescriptor * action)
{
    if (!plantDRAInitialised)
    {
        Use_ProcessCoordinator;

        //  The RootLength High characteristic is now a data member of this class: plant_total_root_length_high
        //  rootlength_high_index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("RootLength high");
        phosphate_concentration_index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Liquid Phase P Concentration");
        nitrate_amount_index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Nitrate Amount");
        Water_Content_Index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Water Content");    
        internal_workings_index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Internal Workings");
        N_absorption_power_index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("N Absorption Power");
        P_absorption_power_index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("P Absorption Power");

        //TODO: PlantSummaryRegistration
        tot_root_length_index1 = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Combined Length Plant1");
        tot_root_length_index2 = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Combined Length Plant2");
        tot_root_length_index3 = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Combined Length Plant3");
        tot_root_length_index4 = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Combined Length Plant4");

        plantCumulativeRootRes = 0.0;
        plantResourceRatio = 1.0;
        plantRemainNonNPResourceUnits = 0.0;
        plantRemainNResourceUnits = 0.0;

        plantSeedReserve_N = seed_size_L_angustifolius;
        //N:P ratios of around 12 are common
        plantSeedReserve_P = plantSeedReserve_N/12.0;
        plantSeedSupply_N = 0.0;

        plantNoduleNumber = 0.0;

        plantCumulativeNfix = 0.0;
        
        plantNUptakePlasticity = 1.0;
    }
    plantDRAInitialised = true;
}

/* ********************************************************************************************** */


// 03/04/2000 VMD
//
//This routine allows the root system to respond to the external nitrate
//concentration and the internal nitrogen status of the plant.  It is based
//on the ratio of N to non-N resources within the plant, the total amount
//of resources available within the plant for growth and maintenance process,
//and the amount of resource that any portion of the plant can return based
//on the external resource supply to which it has access.  The response
//system is applied to each individual plant. Initially this routine only
//reproduces root growth and uptake responses to nitrate and water supply -
//will be expanded to include other nutrients such as phosphorus.
//
//
// * What is the "internal nitrogen status of the plant"
// *
//
long int Plant::DynamicResourceAllocationToRoots(ProcessActionDescriptor *action)
{
    Use_ReturnValue;
    Use_Time;
    Use_Scoreboard;
    
    double_cs growth_rate0, growth_rate1, growth_rate2, growth_rate3;
    double_cs seed_N_reserve, seed_P_reserve;
    double_cs N_resource_ratio, P_resource_ratio;
    double_cs resource_ratio;
    double_cs rem_nonNP_resource_units, rem_N_resource_units, rem_P_resource_units;
    double_cs seed_N_resv_supply, seed_P_resv_supply;  
    double_cs N_target_resource_ratio = 1;
    double_cs P_target_resource_ratio = 0.12;
//    double_cs branch_interval0, branch_interval1, branch_interval2, branch_interval3; //Branch interval  and branch lag dynamics were removed due to
//    double_cs branch_lag0, branch_lag1, branch_lag2, branch_lag3; // problems with the branch interval characteristic - could be added back in now.
    double_cs total_RL_upper = 0;
    double_cs total_RL_lower = 0;
    double_cs total_RD_upper = 0;
    double_cs total_RD_lower = 0;    
    double_cs total_root_length = 0;
    double_cs rootlength_high = 0;
    double_cs total_nonNP_uptake = 0;
    double_cs potential_lownonNP_uptake = 0;
    double_cs potential_highnonNP_uptake = 0;
    double_cs tot_n_uptake = 0;
    double_cs tot_p_uptake = 0;
    double_cs total_nitrate_uptake = 0;
    double_cs total_phosphorus_uptake = 0;
    double_cs total_Nfix = 0;
    double_cs P_cons_absorp_pow;
    double_cs N_cons_absorp_pow;
    double_cs tot_N_cons_absorp_pow;
    double_cs tot_P_cons_absorp_pow; //Absorption power calculated from the constitutive system (ie in response to the external nitrate conc. only).
    double_cs actual_N_absorp_pow;
    double_cs actual_P_absorp_pow; // Actual absorption power = constitutive absorption power * IFM;
    double_cs nitrate_conc = 0;
    double_cs box_volume = 0;
    double_cs water_content = 0;
    double_cs phosphate_conc = 0;
    double_cs N_resour_alloc;
    double_cs nonNP_resour_alloc;
    double_cs P_resour_alloc;
    double_cs local_root_length;
    double_cs N_abs_ratio;
    double_cs P_abs_ratio;
    double_cs local_nitrate_uptake;
    double_cs local_phosphorus_uptake;
    double_cs total_resource_units;
    double_cs total_root_resour_units;
    double_cs max_seedN_supply_rte;
    double_cs max_seedP_supply_rte;
    double_cs fixation_demand = 0;
    double_cs additional_fix_units = 0;
    double_cs fix_resour_alloc = 0;
    double_cs nodule_maint_costs = 0;
    double_cs new_nodule_growth = 0;
    double_cs nodule_number = 0;
    double_cs local_water_uptake;
    double_cs total_water_uptake;
    double_cs total_inN_res = 0;
    double_cs total_innonNP_res = 0;
    double_cs total_outN_res = 0;
    double_cs total_outnonNP_res = 0;

    //Root system demand for a share of the total resources
    double_cs root_res_demand = 0.85; 
    
    //How many boxes do we have?
    long num_x, num_y, num_z;
    Get_Number_Of_Layers(num_x,num_y,num_z);
    
    const ProcessTime_t dT = time - plantPreviousTimestamp;
    plantPreviousTimestamp = time;

//    plant_counter = 1;
//    int plantcoordinator_number_of_plants = plants->GetNumItems();
    
//    while(plant_counter<=plantcoordinator_number_of_plants) //Calculates the total nitrate uptake and total root length in the simulation volume
//    {
        rootlength_high = plant_total_root_length_high;// Get_Concentration_Of_Box(rootlength_high_index, plant_counter,1,1);
        total_nitrate_uptake = 0;
        total_phosphorus_uptake = 0;
        total_root_length = 0;
        
        BoxIndex box_index = -1;
        for (long y=1; y<=num_y; y++)
        {
            for (long x=1; x<=num_x; x++)
            {    
                for (long z=1; z<=num_z; z++)
                {
// THE REPLACEMENT
                    box_index = scoreboard->GetBoxIndex(x,y,z);
                    total_nitrate_uptake += saNitrateUptake->GetValue(box_index);
                    total_phosphorus_uptake += saPhosphorusUptake->GetValue(box_index);
                    total_root_length += Get_Concentration_Of_BoxIndex(tot_root_length_index1, box_index);

// THE REPLACED
//                   switch (plant_counter)
//                   {
//                           case 1:
//                           total_nitrate_uptake += Get_Concentration_Of_Box(nitrate_uptake_index1, x, y, z);
//                           total_phosphorus_uptake += Get_Concentration_Of_Box(phosphorus_uptake_index1, x, y, z);
//                        total_root_length +=x, y, z);
//                        break;
//                        case 2:
//                        total_nitrate_uptake += Get_Concentration_Of_Box(nitrate_uptake_index2, x, y, z);
//                        total_phosphorus_uptake += Get_Concentration_Of_Box(phosphorus_uptake_index2, x, y, z);
//                        total_root_length += Get_Concentration_Of_Box(tot_root_length_index2, x, y, z);
//                        break;
//                        case 3:
//                        total_nitrate_uptake += Get_Concentration_Of_Box(nitrate_uptake_index3, x, y, z);
//                        total_phosphorus_uptake += Get_Concentration_Of_Box(phosphorus_uptake_index3, x, y, z);
//                        total_root_length += Get_Concentration_Of_Box(tot_root_length_index3, x, y, z);
//                        break;
//                        case 4:
//                        total_nitrate_uptake += Get_Concentration_Of_Box(nitrate_uptake_index4, x, y, z);
//                        total_phosphorus_uptake += Get_Concentration_Of_Box(phosphorus_uptake_index4, x, y, z);
//                        total_root_length += Get_Concentration_Of_Box(tot_root_length_index4, x, y, z);
//                        break;
//                    }
                } //for (z=1; z<=num_z; z++)
            } //for (x=1; x<=num_x; x++)
        } //for (y=1; y<=num_y; y++)
        
        Set_Concentration_Of_Box(internal_workings_index, total_nitrate_uptake, 1,1,1);
        tot_n_uptake = total_nitrate_uptake;
        tot_p_uptake = total_phosphorus_uptake;
    
        if (total_root_length > 1e-15)  //need to define a zero, otherwise the model puts in stupidly small numbers, such as 1e-34 and then this routine runs
        {
            potential_lownonNP_uptake = (L_NON_NP_RESOURCE_SIZE * (total_root_length - rootlength_high) * dT); //Each unit of tap and 1st order root length obtains a fixed amount of resource
            potential_highnonNP_uptake = (H_NON_NP_RESOURCE_SIZE * (rootlength_high) * dT); //Each unit of high order root length obtains a fixed amount of resource

            // this model works by determining the ratio of the various resource classes.  Initially, there are only two resources classes
            // 1) nitrogen (nitrate) resources and 2) non-nitrogen resources.  Initially, all non-nitrogen resources are assumed to be adequately supplied
            // ie are non-limiting, hence are taken up based upon a fixed potential to acquire non-nitrogen resources (including carbon supply from the shoots). 
            // As a first approximation, higher order laterals (second order and higher) are assumed to have a different potential for acquiring non-nitrogen 
            // resources than low order (tap and 1st order laterals) roots do. 
        
            // The total mobile non-nitrogen and non-phosphorus (nonNP) pool that the plant has available to it is determined by the potential uptake scaled by the plant
            // demand for nonNP resources (nonNP_res_ratio) plus the nonNP that was previously taken up in excess of need and
            // stored in the mobile storage pool (rem_nonNP_resource_units)
// THE REPLACEMENT 
            total_nonNP_uptake = ((potential_lownonNP_uptake + potential_highnonNP_uptake) * plantResourceRatio) + plantRemainNonNPResourceUnits;
            total_nitrate_uptake += plantRemainNResourceUnits;
            total_phosphorus_uptake += plantRemainNResourceUnits;
            max_seedN_supply_rte = (plantSeedReserve_N/(mean_coty_life - time));
            max_seedP_supply_rte = (plantSeedReserve_P/(mean_coty_life - time));
            seed_N_reserve = plantSeedReserve_N;
            seed_P_reserve = plantSeedReserve_P;
            nodule_number = plantNoduleNumber;

// THE REPLACED
//            switch (plant_counter) 
//            {
//                case 1:
//                total_nonNP_uptake = ((potential_lownonNP_uptake + potential_highnonNP_uptake)*Resource_Ratio_Plant1) + rem_nonNP_resource_units_plant1;
//                total_nitrate_uptake += rem_N_resource_units_plant1;
//                total_phosphorus_uptake += rem_P_resource_units_plant1;
//                max_seedN_supply_rte = (seed_N_reserve_plant1/(mean_coty_life - time));
//                max_seedP_supply_rte = (seed_P_reserve_plant1/(mean_coty_life - time));
//                seed_N_reserve = seed_N_reserve_plant1;
//                seed_P_reserve = seed_P_reserve_plant1;
//                nodule_number = nodule_number_plant1;
//                break;
//                case 2:
//                total_nonNP_uptake = ((potential_lownonNP_uptake + potential_highnonNP_uptake)*Resource_Ratio_Plant2) + rem_nonNP_resource_units_plant2;
//                total_nitrate_uptake += rem_N_resource_units_plant2;
//                total_phosphorus_uptake += rem_P_resource_units_plant2;
//                max_seedN_supply_rte = (seed_N_reserve_plant2/(mean_coty_life - time));
//                max_seedP_supply_rte = (seed_P_reserve_plant2/(mean_coty_life - time));
//                seed_N_reserve = seed_N_reserve_plant2;
//                seed_P_reserve = seed_P_reserve_plant2;
//                nodule_number = nodule_number_plant2;
//                break;
//                case 3:
//                total_nonNP_uptake = ((potential_lownonNP_uptake + potential_highnonNP_uptake)*Resource_Ratio_Plant3) + rem_nonNP_resource_units_plant3;
//                total_nitrate_uptake += rem_N_resource_units_plant3;
//                total_phosphorus_uptake += rem_P_resource_units_plant3;
//                max_seedN_supply_rte = (seed_N_reserve_plant3/(mean_coty_life - time));
//                max_seedP_supply_rte = (seed_P_reserve_plant3/(mean_coty_life - time));
//                seed_N_reserve = seed_N_reserve_plant3;
//                seed_P_reserve = seed_P_reserve_plant3;
//                nodule_number = nodule_number_plant3;
//                break;
//                case 4:
//                total_nonNP_uptake = ((potential_lownonNP_uptake + potential_highnonNP_uptake)*Resource_Ratio_Plant4) + rem_nonNP_resource_units_plant4;
//                total_nitrate_uptake += rem_N_resource_units_plant4;
//                total_phosphorus_uptake += rem_P_resource_units_plant4;
//                max_seedN_supply_rte = (seed_N_reserve_plant4/(mean_coty_life - time));
//                max_seedP_supply_rte = (seed_P_reserve_plant4/(mean_coty_life - time));
//                seed_N_reserve = seed_N_reserve_plant4;
//                seed_P_reserve = seed_P_reserve_plant4;
//                nodule_number = nodule_number_plant4;
//                break;
//            } //switch (plant_counter)

        
            // resource intake increases over flower and seed production time.  Quadratic function based upon the phenology and resource demand of lupins
            if((time >= 10800000)&&(time <= 17539200))
            {
// THE REPLACEMENT
                total_nonNP_uptake = (-15.71+(2.5e-6*time)-(8.826e-14*time*time))*((potential_lownonNP_uptake + potential_highnonNP_uptake)*plantResourceRatio) + plantRemainNonNPResourceUnits;
// THE REPLACED
//                switch (plant_counter)
//                {
//                    case 1:
//                    total_nonNP_uptake = (-15.71+(2.5e-6*time)-(8.826e-14*time*time))*((potential_lownonNP_uptake + potential_highnonNP_uptake)*Resource_Ratio_Plant1) + rem_nonNP_resource_units_plant1;
//                    break;
//                    case 2:
//                    total_nonNP_uptake = (-15.71+(2.5e-6*time)-(8.826e-14*time*time))*((potential_lownonNP_uptake + potential_highnonNP_uptake)*Resource_Ratio_Plant2) + rem_nonNP_resource_units_plant2;
//                    break;
//                    case 3:
//                    total_nonNP_uptake = (-15.71+(2.5e-6*time)-(8.826e-14*time*time))*((potential_lownonNP_uptake + potential_highnonNP_uptake)*Resource_Ratio_Plant3) + rem_nonNP_resource_units_plant3;
//                    break;
//                    case 4:
//                    total_nonNP_uptake = (-15.71+(2.5e-6*time)-(8.826e-14*time*time))*((potential_lownonNP_uptake + potential_highnonNP_uptake)*Resource_Ratio_Plant4) + rem_nonNP_resource_units_plant4;
//                    break;
//                } //switch (plant_counter)
            } //if((time >= 10800000)&&(time <= 17539200))
        

            if (plant_is_a_legume == 1) //then are modelling an N2 fixing legume and N2 fixation must be calculated.
            {
                // Depending upon the plant's demand for N, assimilates are provided to support nodule growth.
                // Each nodule is assumed to have the same potential to fix N2.
                total_Nfix = nodule_number * unit_Nfix;
            }
            else
            {
                total_Nfix = 0;
            }


            // Resource supply from the seed/cotyledons.  The seedling derives 100% of its resource requirement from the seed/cotyledon reserves initially.
            // Over time these reserves deplete and resource supply from roots and tops increases.
            // Supply of N from the seed:
            
            if ((seed_N_reserve > 0) && (time < mean_coty_life) && ((total_nonNP_uptake - ((total_nitrate_uptake + total_Nfix)/N_target_resource_ratio))>0))
            {
                   if ((total_nonNP_uptake - ((total_nitrate_uptake + total_Nfix)/N_target_resource_ratio)) < (max_seedN_supply_rte*dT))
                    { //if the N demand is less than the remaining seed N supply then demand can be met
                    seed_N_resv_supply = (total_nonNP_uptake - ((total_nitrate_uptake + total_Nfix)/N_target_resource_ratio));                    
                    if (seed_N_resv_supply < 0){seed_N_resv_supply = 0;}
                    
                    seed_N_reserve -= seed_N_resv_supply;
                    if (seed_N_reserve < 0){seed_N_reserve = 0;} 
                 } //if the N demand is less than the remaining seed N supply then demand can be met
                 
                 else //plant demand is now greater than the seed supply and so fixation needs to kick in
                 {
                    seed_N_resv_supply = max_seedN_supply_rte*dT;  //Supply N from the seed at the max supply rate
                    if (seed_N_resv_supply < 0){seed_N_resv_supply = 0;}
                    
                    seed_N_reserve -= seed_N_resv_supply;                    
                    if (seed_N_reserve < 0){seed_N_reserve = 0;} 
                } // else plant demand is now greater than the seed supply and so fixation needs to kick in
                
            } //if ((seed_N_reserve > 0) && (time < mean_coty_life) && ((total_nonNP_uptake - ((total_nitrate_uptake + total_Nfix)/N_target_resource_ratio))>0))
                
            else
            {
                seed_N_resv_supply = 0;
                seed_N_reserve = 0;
            } // else   
                 
                 
            if ((seed_P_reserve > 0) && (time < mean_coty_life) && ((total_nonNP_uptake - (total_phosphorus_uptake/P_target_resource_ratio))>0))
            {
                   if ((total_nonNP_uptake - (total_phosphorus_uptake/P_target_resource_ratio)) < (max_seedP_supply_rte*dT))
                    { //if the P demand is less than the remaining seed P supply then demand can be met
                    seed_P_resv_supply = (total_nonNP_uptake - ((total_nitrate_uptake + total_Nfix)/N_target_resource_ratio));                    
                    if (seed_P_resv_supply < 0){seed_P_resv_supply = 0;}
                    
                    seed_P_reserve -= seed_P_resv_supply;
                    if (seed_P_reserve < 0){seed_P_reserve = 0;} 
                 } //if the N demand is less than the remaining seed N supply then demand can be met
                 
                 else //plant P demand is now greater than the seed supply
                 {
                    seed_P_resv_supply = max_seedP_supply_rte*dT;  //Supply P from the seed at the max supply rate
                    if (seed_P_resv_supply < 0){seed_P_resv_supply = 0;}
                    
                    seed_P_reserve -= seed_P_resv_supply; 
                    if (seed_P_reserve < 0){seed_P_reserve = 0;}                    
                 } // else plant demand is now greater than the seed supply and so fixation needs to kick in                
            } //if ((seed_P_reserve > 0) && (time < mean_coty_life) && ((total_nonNP_uptake - ((total_phosphorus_uptake)/P_target_resource_ratio))>0))
            
            else
            {
                seed_P_resv_supply = 0;
                seed_P_reserve = 0;
            } // else  


            //The resource ratios are simply the ratio of the resource demand to the resource supply.  At the moment the non-N resource is
            //pegged to the N resource with the aim of attaining parity.  A more sophisticated resource/demand relationship can be included later.
            N_resource_ratio = (total_nitrate_uptake + total_Nfix + seed_N_resv_supply) / total_nonNP_uptake;
            P_resource_ratio = (total_phosphorus_uptake + seed_P_resv_supply) / total_nonNP_uptake;
            
               if((N_resource_ratio/N_target_resource_ratio) < (P_resource_ratio/P_target_resource_ratio)) 
            { //identifying the most limiting resource
                resource_ratio = N_resource_ratio; // N supply is most limiting
            }
            else
            {
                resource_ratio = P_resource_ratio; // P supply is most limiting
            }
            
            Set_Concentration_Of_Box(internal_workings_index, resource_ratio, 4, 1, 1);
            
            if (time < leaf_emergence)
            {
                root_res_demand = early_root_res_demand; //Resource demand of the root system is high during germination
            }
            if (time > resource_shift_time)
            {
                root_res_demand = 1- (0.60*(1-exp((-2.08/anthesis)*time))); // Asmptotic relationship describing the shift of resource supply away from
            }                                                                // root growth and towards tops over the growing season.
            
            
            if (((resource_ratio == N_resource_ratio)&&(resource_ratio < N_target_resource_ratio)) || ((resource_ratio == P_resource_ratio)&&(resource_ratio < P_target_resource_ratio)))
            {   //plant is N or P resource limited.  Thus need to downregulate the uptake of nonNP resources and upregulate the uptake of the N or P or both resources
             
                //Resources are scaled such that all parts of the plant require the same ratio of nonNP and N resource.
                //All other resources are then scaled accordingly  
                if (resource_ratio == N_resource_ratio) // N supply is most limiting
                {
                    total_resource_units = (total_nitrate_uptake + total_Nfix + seed_N_resv_supply)*(1 + (1/N_target_resource_ratio) + (P_target_resource_ratio/N_target_resource_ratio));
                    rem_nonNP_resource_units = total_nonNP_uptake - ((total_nitrate_uptake + total_Nfix + seed_N_resv_supply)/N_target_resource_ratio);
                    rem_P_resource_units = (total_phosphorus_uptake + seed_P_resv_supply) - (total_nitrate_uptake + total_Nfix + seed_N_resv_supply)*(P_target_resource_ratio/N_target_resource_ratio);
                    rem_N_resource_units = 0;                          
                    if (rem_P_resource_units > P_storage_limit)
                    {
                           rem_P_resource_units = P_storage_limit;  //The plant is assumed to have a finite capacity to store resources for later use.
                    }
                }
                else //limited by the supply of P
                {
                    total_resource_units = (total_phosphorus_uptake + seed_P_resv_supply)*(1 + (1/P_target_resource_ratio) + (N_target_resource_ratio/P_target_resource_ratio));
                    rem_nonNP_resource_units = (total_nonNP_uptake - ((total_phosphorus_uptake + seed_P_resv_supply)/P_target_resource_ratio));
                    rem_P_resource_units = 0;
                    rem_N_resource_units = (total_nitrate_uptake + total_Nfix + seed_N_resv_supply) - (total_phosphorus_uptake + seed_P_resv_supply)*(N_target_resource_ratio/P_target_resource_ratio);
                    
                    if (rem_N_resource_units > N_storage_limit)
                        {
                        rem_N_resource_units = N_storage_limit;  //The plant is assumed to have a finite capacity to store resources for later use.
                        }
                }
                                       
                total_root_resour_units = root_res_demand * total_resource_units;
                // VMD 2001 This relation below was used to simulate fixed resource allocation for modelling scenarios where needed
                // to directly compare lupin root systems of equivalent cost
                   //total_root_resour_units = (29000/2)/(1+5119*exp(-0.230*(time/60/60/24)));
                
                if(time < early_germination_time)
                {
                    rem_nonNP_resource_units = 0; //Very young plants are not storing resources for later use.
                }
                if (rem_nonNP_resource_units > nonNP_storage_limit)
                {
                    rem_nonNP_resource_units = nonNP_storage_limit;  //The plant is assumed to have a finite capacity to store resources for later use.
                }
                                
                if (plant_is_a_legume == 1)
                {
                    if (resource_ratio == N_resource_ratio) // N supply is most limiting
                    {
                        fixation_demand = (1/resource_ratio);
                           additional_fix_units = max_fix_alloc * root_res_demand * rem_nonNP_resource_units; //A proportion of the mobile storage pool is allocated to growing more nodules
                           rem_nonNP_resource_units -= additional_fix_units;
                       }
                       else
                       {
                           fixation_demand = 0;
                           additional_fix_units = 0;
                       }
                       //resources are allocated to fixation based upon the current level of fixation and the demand for more fixation.
                       fix_resour_alloc = fixation_demand * max_fix_alloc * (total_Nfix/(total_nitrate_uptake + total_Nfix + seed_N_resv_supply)) * total_root_resour_units;
                       nodule_maint_costs = (unit_nodule_cost * nodule_number);
                    new_nodule_growth = unit_nodule_growth * (fix_resour_alloc + additional_fix_units - nodule_maint_costs);
                    nodule_number += new_nodule_growth;  
                    if (nodule_number < 0) { nodule_number = 0;}  
                } //(plant_is_a_legume == 1)        
            } // if (resource_ratio < 1)
            if (((resource_ratio == N_resource_ratio)&&(resource_ratio > N_target_resource_ratio)) || ((resource_ratio == P_resource_ratio)&&(resource_ratio > P_target_resource_ratio)))
            {    //plant is non_NP resource limited.  Thus need to upregulate the uptake of non_NP resources
                if (resource_ratio > nonNPdown_regul_factor)
                {
                    resource_ratio = nonNPdown_regul_factor; //limited ability to downregulate resource supply
                }

                total_resource_units = total_nonNP_uptake*(1 + N_target_resource_ratio + P_target_resource_ratio) ;
                rem_N_resource_units = (total_nitrate_uptake + total_Nfix + seed_N_resv_supply) - total_nonNP_uptake*N_target_resource_ratio;
                   rem_P_resource_units = (total_phosphorus_uptake + seed_P_resv_supply) - total_nonNP_uptake*P_target_resource_ratio;
                rem_nonNP_resource_units = 0;                
                fixation_demand = 0;                
                                   
                if(time < early_germination_time)
                {
                    rem_N_resource_units = 0; //Very young plants are not storing resources for later use.
                    rem_P_resource_units = 0;
                }
                if (rem_N_resource_units > N_storage_limit)
                {
                    rem_N_resource_units = N_storage_limit;  //The plant is assumed to have a finite capacity to store resources for later use.
                }
                if (rem_P_resource_units > P_storage_limit)
                {
                    rem_P_resource_units = P_storage_limit;  //The plant is assumed to have a finite capacity to store resources for later use.
                }
            } //if (resource_ratio > 1)
            if (((resource_ratio == N_resource_ratio)&&(resource_ratio == N_target_resource_ratio)) || ((resource_ratio == P_resource_ratio)&&(resource_ratio == P_target_resource_ratio)))
            {    // the plant is aquiring resources in an ideal balance, probably due to sufficient supply from seed reserves to meet demand
                total_resource_units = (total_nitrate_uptake + total_Nfix + seed_N_resv_supply) + (total_phosphorus_uptake + seed_P_resv_supply) + total_nonNP_uptake;
                total_root_resour_units = root_res_demand * total_resource_units;
                // VMD 2001 This relation below was used to simulate fixed resource allocation for modelling scenarios where needed
                // to directly compare lupin root systems of equivalent cost
                   //total_root_resour_units = (29000/2)/(1+5119*exp(-0.230*(time/60/60/24)));
                   
                fixation_demand = 0;
                rem_nonNP_resource_units = 0;
                rem_N_resource_units = 0;
                rem_P_resource_units = 0;
            } //if (resource_ratio == 1)
                                                                                        
        
            //15/11/00 Under NitrateLeaching I have saved a characteristic called Internal Workings.  I use this to save values to the first row of scoreboard boxes
            // for variables which describe how the model is working internally such as the amount of N fixed, the N and nonN demands, seed supply of N etc.
            //since these variables don't vary with the scoreboard box, I don't need a separate characteristic for each one, I just allocate each variable to 1 scoreboard box.
            Set_Concentration_Of_Box(internal_workings_index, total_Nfix, 2, 1, 1);
            Set_Concentration_Of_Box(internal_workings_index, nodule_number, 3, 1, 1); 
            
            total_innonNP_res = total_root_resour_units; //added for simulating fixed resource allocation
            plantCumulativeRootRes += total_root_resour_units; //Total resources used by the plant roots over the lifetime of the plant - cost indicator

            
// THE REPLACEMENT 
            plantSeedReserve_N = seed_N_reserve;
            plantSeedReserve_P = seed_P_reserve;
            plantSeedSupply_N += seed_N_resv_supply;
            plantResourceRatio = resource_ratio;
            plantRemainNonNPResourceUnits = rem_nonNP_resource_units;
            plantRemainNResourceUnits = rem_N_resource_units;
            plantNoduleNumber = nodule_number;

// THE REPLACED
//            switch (plant_counter)
//            {
//                case 1:
//                seed_N_reserve_plant1 = seed_N_reserve;
//                seed_P_reserve_plant1 = seed_P_reserve;
//                seed_N_supply_plant1 += seed_N_resv_supply;
//                Resource_Ratio_Plant1 = resource_ratio;
//                rem_nonNP_resource_units_plant1 = rem_nonNP_resource_units;
//                rem_N_resource_units_plant1 = rem_N_resource_units;
//                nodule_number_plant1 = nodule_number;
//                break;
//                case 2:
//                seed_N_reserve_plant2 = seed_N_reserve;
//                seed_P_reserve_plant2 = seed_P_reserve;
//                seed_N_supply_plant2 += seed_N_resv_supply;
//                Resource_Ratio_Plant2 = resource_ratio;
//                rem_nonNP_resource_units_plant2 = rem_nonNP_resource_units;
//                rem_N_resource_units_plant2 = rem_N_resource_units;
//                nodule_number_plant2 = nodule_number;
//                break;
//                case 3:
//                seed_N_reserve_plant3 = seed_N_reserve;
//                seed_P_reserve_plant3 = seed_P_reserve;
//                seed_N_supply_plant3 += seed_N_resv_supply;
//                Resource_Ratio_Plant3 = resource_ratio;
//                rem_nonNP_resource_units_plant3 = rem_nonNP_resource_units;
//                rem_N_resource_units_plant3 = rem_N_resource_units;
//                nodule_number_plant3 = nodule_number;
//                break;
//                case 4:
//                seed_N_reserve_plant4 = seed_N_reserve;
//                seed_P_reserve_plant4 = seed_P_reserve;
//                seed_N_supply_plant4 += seed_N_resv_supply;
//                Resource_Ratio_Plant4 = resource_ratio;
//                rem_nonNP_resource_units_plant4 = rem_nonNP_resource_units;
//                rem_N_resource_units_plant4 = rem_N_resource_units;
//                nodule_number_plant4 = nodule_number;
//                break;
//            } //switch (plant_counter)

            //averaging routine to determine mean value for the regulation factor ratio
            long i=0;
            tot_N_cons_absorp_pow = 0, tot_P_cons_absorp_pow = 0, total_water_uptake = 0;
        
           
           //This routine is used to help calculate the average absorption power.  By knowing this
           //it is then known whether roots in individual boxes have greater or less access to nutrients
           //so their uptake kinetics should be up or down regulated.
            for (long y=1; y<=num_y; y++)
            {
                for (long x=1; x<=num_x; x++)
                {    
                    for (long z=1; z<=num_z; z++)
                    {
                        box_index = scoreboard->GetBoxIndex(x,y,z);

                        box_volume = Get_BoxIndex_Volume(box_index);

                        water_content = Get_Concentration_Of_BoxIndex(Water_Content_Index,box_index);
                        nitrate_conc = (Get_Concentration_Of_BoxIndex(nitrate_amount_index, box_index))/(14.0067*water_content*box_volume);
                        phosphate_conc = Get_Concentration_Of_BoxIndex(phosphate_concentration_index, box_index);
                        tot_N_cons_absorp_pow += N_flux_max/(N_uptake_kinetic_cons + nitrate_conc);
                        tot_P_cons_absorp_pow += P_flux_max/(P_uptake_kinetic_cons + (phosphate_conc - P_conc_min));
                    
// THE REPLACEMENT
                        if((local_root_length = Get_Concentration_Of_BoxIndex(tot_root_length_index1, box_index)) > 1e-15)
                        {
                            total_water_uptake += saWaterUptake->GetValue(box_index);
//                            total_water_uptake += Get_Concentration_Of_BoxIndex(water_uptake_index1, box_index);
                        }

// THE REPLACED
//                        switch (plant_counter)
//                        {
//                            case 1:
//                            if((local_root_length = Get_Concentration_Of_Box(tot_root_length_index1, x, y, z)) > 1e-15)
//                            {
//                                total_water_uptake += Get_Concentration_Of_Box(water_uptake_index1, x, y, z);
//                            }
//                            break;
//                            case 2:
//                            if((local_root_length = Get_Concentration_Of_Box(tot_root_length_index2, x, y, z)) > 1e-15)
//                            {
//                                total_water_uptake += Get_Concentration_Of_Box(water_uptake_index2, x, y, z);
//                            }
//                            break;
//                            case 3:
//                            if((local_root_length = Get_Concentration_Of_Box(tot_root_length_index3, x, y, z)) > 1e-15)
//                            {
//                                total_water_uptake += Get_Concentration_Of_Box(water_uptake_index3, x, y, z);
//                            }
//                            break;
//                            case 4:
//                            if((local_root_length = Get_Concentration_Of_Box(tot_root_length_index4, x, y, z)) > 1e-15)
//                            {
//                                total_water_uptake += Get_Concentration_Of_Box(water_uptake_index4, x, y, z);
//                            }
//                            break;
//                        } //switch (plant_counter)

                        i++;
                    } //for (z=1; z<=num_z; z++)
                } //for (x=1; x<=num_x; x++)
            } //for (y=1; y<=num_y; y++)
        
            Set_Concentration_Of_Box(internal_workings_index, total_water_uptake, 8, 1, 1);
                        
    
            for (long y=1; y<=num_y; y++)
            {
                for (long x=1; x<=num_x; x++)
                {    
                    for (long z=1; z<=num_z; z++)
                    {
                        box_index = scoreboard->GetBoxIndex(x,y,z);
                        
// THE REPLACEMENT
                        local_nitrate_uptake = saNitrateUptake->GetValue(box_index);
                        local_phosphorus_uptake = saPhosphorusUptake->GetValue(box_index);
                        local_water_uptake = saWaterUptake->GetValue(box_index);
                        local_root_length = Get_Concentration_Of_BoxIndex(tot_root_length_index1, box_index);
                        //have now used all the nitrate uptake so reset it to zero
                        saNitrateUptake->SetValue(0.0, box_index);


// THE REPLACED
//                        switch (plant_counter)
//                        {
//                            case 1:
//                            local_nitrate_uptake = Get_Concentration_Of_Box(nitrate_uptake_index1, x, y, z);
//                            local_phosphorus_uptake = Get_Concentration_Of_Box(phosphorus_uptake_index1, x,y,z);
//                            Set_Concentration_Of_Box(nitrate_uptake_index1, 0, x, y, z); //have now used all this uptake so reset to zero
//                            local_root_length = Get_Concentration_Of_Box(tot_root_length_index1, x, y, z);
//                            local_water_uptake = Get_Concentration_Of_Box(water_uptake_index1, x, y, z);
//                            break;
//                            case 2:
//                            local_nitrate_uptake = Get_Concentration_Of_Box(nitrate_uptake_index2, x, y, z);
//                            local_phosphorus_uptake = Get_Concentration_Of_Box(phosphorus_uptake_index2, x,y,z);
//                            Set_Concentration_Of_Box(nitrate_uptake_index2, 0, x, y, z); //have now used all this uptake so reset to zero
//                            local_root_length = Get_Concentration_Of_Box(tot_root_length_index2, x, y, z);
//                            local_water_uptake = Get_Concentration_Of_Box(water_uptake_index2, x, y, z);
//                            break;
//                            case 3:
//                            local_nitrate_uptake = Get_Concentration_Of_Box(nitrate_uptake_index3, x, y, z);
//                            local_phosphorus_uptake = Get_Concentration_Of_Box(phosphorus_uptake_index3, x,y,z);
//                            Set_Concentration_Of_Box(nitrate_uptake_index3, 0, x, y, z); //have now used all this uptake so reset to zero
//                            local_root_length = Get_Concentration_Of_Box(tot_root_length_index3, x, y, z);
//                            local_water_uptake = Get_Concentration_Of_Box(water_uptake_index3, x, y, z);
//                            break;
//                            case 4:
//                            local_nitrate_uptake = Get_Concentration_Of_Box(nitrate_uptake_index4, x, y, z);
//                            local_phosphorus_uptake = Get_Concentration_Of_Box(phosphorus_uptake_index4, x,y,z);
//                            Set_Concentration_Of_Box(nitrate_uptake_index4, 0, x, y, z); //have now used all this uptake so reset to zero
//                            local_root_length = Get_Concentration_Of_Box(tot_root_length_index4, x, y, z);
//                            local_water_uptake = Get_Concentration_Of_Box(water_uptake_index4, x, y, z);
//                            break;
//                        } //switch (plant_counter)
                        
                        //Calculating the root growth and branching rates.
                        if (local_root_length > 1e-15)
                        {   //Calculating the absorption power. Convert nitrate concentration from µgN/cm^3 to µmol NO3-/cm^3.
                            water_content = Get_Concentration_Of_BoxIndex(Water_Content_Index, box_index);
                            box_volume = Get_BoxIndex_Volume(box_index);
                            nitrate_conc = (Get_Concentration_Of_BoxIndex(nitrate_amount_index, box_index))/(water_content*box_volume*14.0067);
                            phosphate_conc = (Get_Concentration_Of_BoxIndex(phosphate_concentration_index, box_index));
                            N_cons_absorp_pow = (N_flux_max/(N_uptake_kinetic_cons + nitrate_conc));  //Michaelis_Menten kinetics - constitutive absorption power.
                            P_cons_absorp_pow = (P_flux_max/(P_uptake_kinetic_cons + (phosphate_conc-P_conc_min)));  //Michaelis_Menten kinetics - constitutive absorption power.
                            N_abs_ratio = (tot_N_cons_absorp_pow/i)/(N_cons_absorp_pow); //Is the local absorption power more or less than the average?
                            P_abs_ratio = (tot_P_cons_absorp_pow/i)/(P_cons_absorp_pow); //Is the local absorption power more or less than the average?
                            if ((N_abs_ratio < 1.03) && (N_abs_ratio > 0.97))
                            {
                                N_abs_ratio = 1;  //Local absorption power is equal to the average therefore uptake is based upon concentration only - no up or down regulation required
                            }
                            if ((P_abs_ratio < 1.03) && (P_abs_ratio > 0.97))
                            {
                                   P_abs_ratio = 1;
                            }
                            if (N_abs_ratio >= 1)
                            {
                                plantNUptakePlasticity = 1/(1+exp(-(-4.0+(4.0*N_abs_ratio))));  //demand relationship for L. angustifolius, uptake upregulated
                            }
                            else
                            {
                                plantNUptakePlasticity = 0.5*(1-exp(-1.58*N_abs_ratio));  //demand relationship for L. angustifolius, uptake downregulated
                            }  
                            if (P_abs_ratio >=1)
                            {
                                plantPUptakePlasticity = 1/(1+exp(-(-4.0+(4.0*P_abs_ratio))));
                            }
                            else
                            {
                                plantPUptakePlasticity = 0.5*(1-exp(-1.58*P_abs_ratio));
                            }                          
                        
                            actual_N_absorp_pow = (1/resource_ratio) * plantNUptakePlasticity * N_cons_absorp_pow * (1/Ndown_regul_factor);
                            actual_P_absorp_pow = (1/resource_ratio) * plantPUptakePlasticity * P_cons_absorp_pow * (1/Pdown_regul_factor);
                            //around flowering and seed filling time the plant tops demand for N increases.
                            if((time >= 10800000)&&(time <= 17539200))
                            {
                                actual_N_absorp_pow = (-64.7+(9.98e-6*time) - (3.52e-13*time*time))*resource_ratio * plantNUptakePlasticity * N_cons_absorp_pow * Ndown_regul_factor;
                                actual_P_absorp_pow = (-64.7+(9.98e-6*time) - (3.52e-13*time*time))*resource_ratio * plantPUptakePlasticity * P_cons_absorp_pow * Pdown_regul_factor;
                            }
                            Set_Concentration_Of_BoxIndex(N_absorption_power_index, actual_N_absorp_pow, box_index);
                            Set_Concentration_Of_BoxIndex(P_absorption_power_index, actual_P_absorp_pow, box_index);
                            Set_Concentration_Of_Box(internal_workings_index, plantNUptakePlasticity, 5, 1, 1);
                            
                            //The roots in a scoreboard box are allocated a percentage of the total resources available dependent upon the amount of both N and
                            //nonNP resources the roots in that box can return.  For now all nonNP resources (other than water) are considered to be non-limiting and are
                            //supplied equally to the plant by all roots.
                            //N supply differs between roots, thus resource allocation between roots differs depending upon their capacity to supply soil N.
                        
                            if (tot_n_uptake == 0) /* NITRATE */
                            {
                                   N_resour_alloc = 0;
                                } //if (tot_n_uptake == 0)
                            else
                               { 
                                   if (plant_is_a_legume == 1)
                                  {
                                      //Allocate root resources to those roots returning the greatest soil N to the plant
                                    N_resour_alloc = (local_nitrate_uptake / tot_n_uptake) * alloc_factor * (total_root_resour_units - fix_resour_alloc); 
                                  }
                                else
                                {
                                    //Allocate root resources to those roots returning the greatest soil N to the plant - No N2 fixation
                                    N_resour_alloc = (local_nitrate_uptake / tot_n_uptake) * alloc_factor * (total_root_resour_units);
                                }
                            } //else NITRATE
                    
                               if(total_water_uptake == 0) /* WATER */
                            {
                                nonNP_resour_alloc = 0;
                            } //if(total_water_uptake == 0)
                            else
                            {
                                if (plant_is_a_legume == 1)
                                {
                                    //Allocate root resources to those roots returning the greatest soil water to the plant
                                       nonNP_resour_alloc = (local_water_uptake / total_water_uptake) * alloc_factor * (total_root_resour_units - fix_resour_alloc);
                                   }
                                else
                                {
                                    //Allocate root resources to those roots returning the greatest soil water to the plant - No N2 fixation
                                    nonNP_resour_alloc = (local_water_uptake / total_water_uptake) * alloc_factor * (total_root_resour_units);
                                }
                            
                                //No root growth response to non-nitrogen resources, allocation based upon root length only
                                //nonNP_resour_alloc = (local_root_length / total_root_length) * (1 - alloc_factor) * (total_root_resour_units - fix_resour_alloc);
                            } //else WATER
                       
                            if(tot_p_uptake == 0) /* PHOSPHORUS */
                            {
                                P_resour_alloc = 0;
                            } //if(total_p_uptake == 0)
                               else
                            {
                                if (plant_is_a_legume == 1)
                                {
                                    //Allocate root resources to those roots returning the greatest P to the plant
                                       P_resour_alloc = (local_phosphorus_uptake / tot_p_uptake) * alloc_factor * (total_root_resour_units - fix_resour_alloc);
                                   }
                                   else
                                   {  
                                    //Allocate root resources to those roots returning the greatest soil water to the plant - No N2 fixation
                                    P_resour_alloc = (local_phosphorus_uptake / tot_p_uptake) * alloc_factor * (total_root_resour_units);
                                }
                            } //else PHOSPHORUS
                    
                            total_inN_res += N_resour_alloc;
                            //total_innonNP_res += nonNP_resour_alloc;
                    
                
                            if ((N_resour_alloc > 0) || (nonNP_resour_alloc > 0) || (P_resour_alloc > 0))
                            {
                                growth_rate0 = 50*((unit_Ngwth_rte0 * N_resour_alloc) + (unit_Pgwth_rte0 * P_resour_alloc) + (unit_nonNPgwth_rte0 * nonNP_resour_alloc)) / (local_root_length * dT);
                                growth_rate0 = CSRestrict(growth_rate0, 0.0, 0.20);
                                
                                growth_rate1 = 100*((unit_Ngwth_rte1 * N_resour_alloc) + (unit_Pgwth_rte1 * P_resour_alloc) + (unit_nonNPgwth_rte1 * nonNP_resour_alloc)) / (local_root_length * dT);
                                growth_rate1 = CSRestrict(growth_rate1, 0.0, 0.20);

                                growth_rate2 = 10*((unit_Ngwth_rte2 * N_resour_alloc) + (unit_Pgwth_rte2 * P_resour_alloc) + (unit_nonNPgwth_rte2 * nonNP_resour_alloc)) / (local_root_length * dT);
                                growth_rate2 = CSRestrict(growth_rate2, 0.0, 0.20);

                                growth_rate3 = ((unit_Ngwth_rte3 * N_resour_alloc) + (unit_Pgwth_rte3 * P_resour_alloc) + (unit_nonNPgwth_rte3 * nonNP_resour_alloc)) / (local_root_length * dT);
                                growth_rate3 = CSRestrict(growth_rate3, 0.0, 0.30);

                    
                                if(!((growth_rate0 >= 0) && (growth_rate0 < 1.0)))
                                {
                                   // Debugger();
                                }

// THE REPLACEMENT
                                saGrowthRateMax->SetValue(growth_rate0, box_index, 0);
                                saGrowthRateMax->SetValue(growth_rate1, box_index, 1);
                                saGrowthRateMax->SetValue(growth_rate2, box_index, 2);
                                saGrowthRateMax->SetValue(growth_rate3, box_index, 3);


// THE REPLACED
//                                switch (plant_counter)
//                                   {
//                                    case 1:
//                                    Set_Concentration_Of_Box(growth_rate_index01, growth_rate0, x, y, z);
//                                    Set_Concentration_Of_Box(growth_rate_index11, growth_rate1, x, y, z);
//                                    Set_Concentration_Of_Box(growth_rate_index21, growth_rate2, x, y, z);
//                                    Set_Concentration_Of_Box(growth_rate_index31, growth_rate3, x, y, z);
//                                    break;
//                                    case 2:
//                                    Set_Concentration_Of_Box(growth_rate_index02, growth_rate0, x, y, z);
//                                    Set_Concentration_Of_Box(growth_rate_index12, growth_rate1, x, y, z);
//                                    Set_Concentration_Of_Box(growth_rate_index22, growth_rate2, x, y, z);
//                                    Set_Concentration_Of_Box(growth_rate_index32, growth_rate3, x, y, z);
//                                    break;
//                                    case 3:
//                                    Set_Concentration_Of_Box(growth_rate_index03, growth_rate0, x, y, z);
//                                    Set_Concentration_Of_Box(growth_rate_index13, growth_rate1, x, y, z);
//                                    Set_Concentration_Of_Box(growth_rate_index23, growth_rate2, x, y, z);
//                                    Set_Concentration_Of_Box(growth_rate_index33, growth_rate3, x, y, z);
//                                    break;
//                                    case 4:
//                                    Set_Concentration_Of_Box(growth_rate_index04, growth_rate0, x, y, z);
//                                    Set_Concentration_Of_Box(growth_rate_index14, growth_rate1, x, y, z);
//                                    Set_Concentration_Of_Box(growth_rate_index24, growth_rate2, x, y, z);
//                                    Set_Concentration_Of_Box(growth_rate_index34, growth_rate3, x, y, z);
//                                    break;
//                                } //switch (plant_counter)
                            } //if ((N_resour_alloc >0) || (nonNP_resour_alloc > 0))  
                        }//if (local_root_length > 1e-15                          
                                    
                    } //for (z=1; z<=num_z; z++)
                } //for (x=1; x<=num_x; x++)
            } //for (y=1; y<=num_y; y++)
        } //if (total_root_length > 0)

        //Set_Concentration_Of_Box(internal_workings_index, total_inN_res, 4, 1, 1);
        //Set_Concentration_Of_Box(internal_workings_index, total_innonN_res, 5, 1, 1);
        //Set_Concentration_Of_Box(internal_workings_index, total_outN_res, 8, 1, 1);
        //Set_Concentration_Of_Box(internal_workings_index, total_outnonN_res, 9, 1, 1);
    
//        plant_counter++;
//    }//while(plant_counter<rootlength_number_of_plants)
        
    return (return_value);    
}


} // namespace rootmap

