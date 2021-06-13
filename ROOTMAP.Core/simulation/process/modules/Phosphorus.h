#ifndef Phosphorus_H
#define Phosphorus_H
/////////////////////////////////////////////////////////////////////////////
// Name:        Phosphorus.h
// Purpose:     Declaration of the Phosphorus class
// Created:     DD/MM/YYYY
// Author:      Vanessa Dunbabin
// $Date: August 2010 VMD$
// $Revision: 2 $
// Copyright:   ©2006 University of Tasmania
//
// Substantially re-written, September 2002, Vanessa Dunbabin
// Latest modification: 6 September 2002
/////////////////////////////////////////////////////////////////////////////

#include "simulation/process/common/Process.h"
#include "core/log/Logger.h"

namespace rootmap
{
    class ProcessSharedAttributeOwner;
    class SharedAttribute;

    class Phosphorus : public Process
    {
    public:
        DECLARE_DYNAMIC_CLASS(Phosphorus)
        // Construction function : required for new_by_name()
        Phosphorus();
        ~Phosphorus();

        // Core action overrides
        virtual bool DoesOverride() const;

        virtual long int Register(ProcessActionDescriptor* action);
        virtual long int Initialise(ProcessActionDescriptor* action);
        virtual long int StartUp(ProcessActionDescriptor* action);
        virtual long int Ending(ProcessActionDescriptor* action);

        // Standard action overrides
        virtual long int DoNormalWakeUp(ProcessActionDescriptor* action);
        virtual long int DoExternalWakeUp(ProcessActionDescriptor* action);
        virtual long int DoScoreboardPlantSummaryRegistered(ProcessActionDescriptor* action); // 'SpSr'
        virtual long int DoGeneralPurpose(ProcessActionDescriptor* action); // 'GPm*'

        virtual void Initialise(const ProcessDAI& data);

        // Phosphorus stuff
        virtual long int UptakeP(ProcessActionDescriptor* action);
        virtual long int LeachP(ProcessActionDescriptor* action);
        /**
         *	MSA 09.10.28 Changed return type to bool (i.e. "Did diffusion take place?")
         *	MSA 11.02.24 Added two optimisation parameters, transferDimension and transferInPositiveDir. Use them if transfer plane and dir are known to the caller (saves having to recalculate them)
         */
        bool CalculatePhosphorusDiffusion(ProcessActionDescriptor* action, const long& this_x, const long& this_y, const long& this_z, const long& next_x, const long& next_y, const long& next_z, const ProcessTime_t& dT, const bool& drain, Dimension transferDimension = NoDimension, bool transferInPositiveDir = true);
        /**
         *	MSA 10.02.02 Removed redundant num_ parameters (use NUM_ const member variables instead; number of Scoreboard boxes does not change throughout a simulation)
         */
        virtual long int CalculateDiffusionOrder(ProcessActionDescriptor* action, const long& x, const long& y, const long& z, const ProcessTime_t& dT, const bool& drain);
        /**
         *	MSA 09.10.28 Changed return type to void (was unused).
         */
        void CalculateSolidLiquidPartition(ProcessActionDescriptor* action, const double& labileSolidPhaseP, const double& totalAvailableP, const BoxIndex& box_index, VolumeObject* coincidentVO, const double& coincidentProportion);
        double CalculatePhosphateMineralisation(ProcessActionDescriptor* action, const ProcessTime_t& dT, const double& organic_p_fraction, const ProcessTime_t& organic_p_time);
        virtual long int DiffuseP(ProcessActionDescriptor* action, const bool& drain);

        void SetVolumeObjectCoordinator(const VolumeObjectCoordinator& voc);

        // Public utility constant: returns molecular weight of Phosphorus.
        static const double Mr;

    private:
        RootMapLoggerDeclaration();
        // the time between uptakes
        ProcessTime_t m_time_between_wakings; // or interval or delay
        ProcessTime_t m_time_prev_waking;
        ProcessTime_t m_time_of_previous_diffusion;
        ProcessTime_t m_time_of_prev_solution_change;
        ProcessTime_t m_time_of_prev_nutr_rnw;

        // the scoreboard characteristic index of the first
        // plant summary that we use
        long phosphorus_first_plant_summary_index;

        // the number of plants, branch orders, and summaries
        long m_phosphorus_number_of_plants;
        long m_phosphorus_number_of_branch_orders;

        CharacteristicIndices Liquid_Phase_P_Conc_Indices, Liquid_Phase_P_Indices;
        CharacteristicIndices Labile_Solid_Phase_P_Indices, Total_Labile_P_Indices, Added_P_Indices, Renew_Labile_P_Indices;
        CharacteristicIndices Rained_Amount_Indices, Wetting_Front_Indices, Drained_Upper_Limit_Indices, Bulk_Density_Indices;
        CharacteristicIndices Total_Nitrates, Water_Content_Indices, Water_Amount_Indices, Water_Moved_Indices, Dispersivity_Indices;
        CharacteristicIndices Buffer_Capacity_Indices, Buffer_Index_Indices;
        CharacteristicIndices OrganicP_Indices, Wilting_Point_Indices, LocalPUptake_Indices;
        CharacteristicIndices FertFrac_Solid_Indices, FertFrac_Liquid_Indices;
        CharacteristicIndices Freundlich_n_Indices, Coeff_C_Indices, Coeff_D_Indices, Recalcitrant_P_Indices;
        CharacteristicIndices Rain_Out_Bottom_Indices, Direction_Moved_Indices;
        CharacteristicIndices TipCount_Indices;

        CharacteristicIndex Plus_X_Index, Plus_Y_Index, Plus_Z_Index, Minus_X_Index, Minus_Y_Index, Minus_Z_Index;

        long int m_itsWaterModuleID;

        long NUM_X, NUM_Y, NUM_Z; // number of layers in each dimension
        BoxIndex BOX_COUNT; // How many boxes do we have?

        double m_cumulative_phosphorus_uptake;
        const double m_DIFFUSION_COEFF_SOLN; //diffusion coefficient in free solution = 0.89e-5 cm2/sec for H2PO4- in free solution


        //
        // SharedAttribute support
        //
        // functions that we require to override as part of being a
        // SharedAttributeOwner
        ProcessSharedAttributeOwner* mySharedAttributeOwner;

        int saWaterFluxPerPlantPerRootOrderIndex;
        int saPhosphorusUptakePerPlantIndex;
        int saRootLengthPerPlantIndex;
        int saRootLengthPerPlantPerRootOrderIndex;
        int saRootRadiusPerPlantPerRootOrderIndex;
        int saAbsorptionPowerPerPlantIndex;
        int saLocalCumulPUptakePerPlantIndex;
        int saTotalCumulPUptakePerPlantIndex;
        int saCumulFertPUptakePerPlantIndex;
        int saSeedingTimeIndex;
        int saOrgPMineralisationIndex;
        int saOrgPMinFractionIndex;
        int saOrgPMinTimeIndex;
        int saNutrientSolutionIndex;
        int saNutrientSolnRenewIndex;
        int saNutrientRenewIndex;
        int saRootHairRadiusIndex;
        int saRootHairDensityIndex;
        int saRootHairLengthIndex;
        int saModelRootHairsIndex;
        int saPUtilisationEfficiencyPerPlantIndex;
        int saOrganicExudatePerPlantIndex;

        SharedAttribute* saApaseMultiplier;
        SharedAttribute* saOrganicExudate;

        static const int ROOT_ORDER_COUNT = 4;

        // Variable pointer to a const VOC.
        const VolumeObjectCoordinator* m_volumeObjectCoordinator;


        bool m_diffusion_rotation_toggle;

        bool m_IS_NUTRIENT_SOLUTION; // logically const, but can't be properly initialised upon construction (because we don't know until Initialise() is called)
        bool m_MODEL_ROOT_HAIRS; // logically const, but can't be properly initialised upon construction (because we don't know until Initialise() is called)


        // MSA 11.04.27 Getting Scoreboard boundaries has profiled out to be a major bottleneck. Setting up local copies.
        struct BoundarySet
        {
            const BoundaryArray& X_boundaries;
            const BoundaryArray& Y_boundaries;
            const BoundaryArray& Z_boundaries;

            // No default constructor.

            explicit BoundarySet(const BoundaryArray& x, const BoundaryArray& y, const BoundaryArray& z)
                : X_boundaries(x)
                , Y_boundaries(y)
                , Z_boundaries(z)
            {
            }

            ~BoundarySet()
            {
            }; // Does not own its members
        };

        const BoundarySet* m_boundarySet;

        void PrepareSharedAttributes(ProcessActionDescriptor* /*action*/);

        // MSA 11.02.03 New private helper method.
        bool DoDiffusion(Scoreboard* scoreboard, const BoxIndex& fromBox, const BoxIndex& toBox, const size_t& fromVOIndex, const size_t& toVOIndex, const double& fromProportion, const double& toProportion, const double& avgDistance, const ProcessTime_t& dT, const bool& drain, const double& volumeObjectSurfacePermeability = 0);
    };
} /* namespace rootmap */

#endif // #ifndef Phosphorus_H
