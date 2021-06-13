#ifndef Water_H
#define Water_H
/////////////////////////////////////////////////////////////////////////////
// Name:        Water.h
// Purpose:     Declaration of the Water class
// Created:     DD/MM/YYYY
// Author:      Vanessa Dunbabin
// $Date: 2009-04-07 (Tues 7th April 2009) $
// $Revision: 2 $
// Copyright:   ©2006 University of Tasmania, Dr Vanessa Dunbabin, Centre for Legumes in Meditteranean Agriculture, Grains Research and Development Corporation, Department of Agriculture and Food Western Australia
/////////////////////////////////////////////////////////////////////////////

#include "simulation/process/common/Process.h"
#include "simulation/process/interprocess_communication/Message.h"
#include "core/log/Logger.h"

namespace rootmap
{
    struct RainfallEvent : public SpecialProcessData
    {
        double rainfall_rad;
        double rainfall_maxT;
        double rainfall_minT;
        double rainfall_amount;
        double rainfall_pan;
        ProcessTime_t rainfall_time;
    };


    class ProcessSharedAttributeOwner;
    class SharedAttribute;

    class Water : public Process
    {
    public:
        DECLARE_DYNAMIC_CLASS(Water)

    private:

        struct WettingFrontInfo
        {
            bool drawable; // This flag is initialised to false in the Water constructor,
            // then set to true the first time a wetting front is drawn by Water::Drain()
            // to ensure we don't try to redraw a wetting front that is not yet defined.
            bool eraseMode; // Set eraseMode before manually calling DrawScoreboard to "undraw" the existing WettingFrontInfo.
            // EraseMode automatically switches off after each use.
            ScoreboardStratum stratum;
            std::vector<BoxCoordinate> boxen;
            std::vector<double> frontLocations;
            ViewDirection vDir;
            wxColour colour;
        };

        // MSA 09.09.10 Changed all time variables to type ProcessTime_t
        // MSA 09.09.09 Changed all class-level variables to use the naming convention 
        // "water_UpperCamelCase" (see Process pointer names below[// 98.10.14 RvH])
        //        * Left the SharedAttribute stuff below alone, though.
        ProcessTime_t water_TimeOfPreviousLeaching;
        ProcessTime_t water_TimeOfPreviousWaterUptake;
        ProcessTime_t water_timeBetweenWakings;
        ProcessTime_t water_TimeOfPreviousEvaporation;
        ProcessTime_t water_TimeOfPreviousRedistribution;
        ProcessTime_t water_TimeOfPreviousCall;
        ProcessTime_t water_TimeOfPreviousRain;

        CharacteristicIndex water_DrawWettingFrontIndex, water_WaterRoutineIndex, water_ResidueCoverIndex, water_MaintainWaterIndex;

        CharacteristicIndices Rained_Amount_Indices, Wetting_Front_Indices, Drained_Upper_Limit_Indices;
        CharacteristicIndices Water_Content_Indices, Water_Moved_Indices, Direction_Moved_Indices;
        CharacteristicIndices Rain_Out_Bottom_Indices, Cumul_Local_Water_Uptake_Indices, Wilting_Point_Indices, Residue_Cover_Indices;
        CharacteristicIndices Coefficient_A_Indices, Coefficient_B_Indices, Alpha_Indices, Saturation_Indices;
        CharacteristicIndices Coefficient_M_Indices, Coefficient_N_Indices, K_Saturation_Indices;

        double water_CumulativeEvaporation;
        double water_AverageEvaporation;

        // 98.10.14 RvH
        Process* water_NitrateModule;
        Process* water_PlantCoordinatorModule;
        Process* water_PhosphorusModule;
        Process* water_OrganicMatterModule;

        // MSA 09.10.20 Support for redrawing wetting front on DrawScoreboard event
        WettingFrontInfo m_wettingFrontInfo;

        bool m_IS_NUTRIENT_SOLUTION; // logically const, but can't be properly initialised upon construction (because we don't know until PrepareSharedAttributes() is called)
        bool m_CALCULATE_EVAPORATION; // logically const, but can't be properly initialised upon construction (because we don't know until PrepareSharedAttributes() is called)
        bool m_MAINTAIN_WATER_CONTENT; // logically const, but can't be properly initialised upon construction (because we don't know until PrepareSharedAttributes() is called)
        bool m_DRAW_WETTING_FRONT; // logically const, but can't be properly initialised upon construction (because we don't know until PrepareSharedAttributes() is called)

    public:
        /**
         * called implicitly by wxCreateDynamicObject(), which is used to create
         * processes which override functionality
         */
        Water();

        /** destructor*/
        ~Water();

        // MSA 10.12.02 Overriding the Process implementation of this method
        // to create per-VolumeObject variants of Characteristics.
        // This implementation calls the Process method when it's done.
        virtual void Initialise(const ProcessDAI& data);

        /**
         * Replacement for WaterDataInputHandler class of old
         */
        virtual void InitialiseSpecial(const ProcessSpecialDAI& data);

        virtual long int Register(ProcessActionDescriptor* action);
        virtual long int Initialise(ProcessActionDescriptor* action);
        virtual long int DoNormalWakeUp(ProcessActionDescriptor* action);
        virtual long int StartUp(ProcessActionDescriptor* action);
        virtual long int DoScoreboardPlantSummaryRegistered(ProcessActionDescriptor* action);

        /// OBSOLETE
        virtual long int DoSpecialInput(ProcessActionDescriptor* action);

        virtual bool DoesOverride() const;

        long int Drain(ProcessActionDescriptor* action);
        long int UptakeWater(ProcessActionDescriptor* action);
        // MSA 09.09.11 combined the two RedistributeWater() functions into one parameterised version
        long int RedistributeWater(ProcessActionDescriptor* action, const int& routineNumber);
        long int EvaporateWater(ProcessActionDescriptor* action);
        long int AddRainfall(ProcessActionDescriptor* action, double rad, double maxT, double minT, double amount, double pan);

        virtual bool DoesDrawing() const;
        virtual bool DoesRaytracerOutput() const;
        virtual long int DoRaytracerOutput(RaytracerData* raytracerData);
        virtual void DrawScoreboard(const DoubleRect& area, Scoreboard* scoreboard);

        void SetVolumeObjectCoordinator(const VolumeObjectCoordinator& voc);

    private:
        RootMapLoggerDeclaration();
        /**
         * A list of rainfall events given from InitialiseSpecial. Sent to
         * PostOffice in Initialise()
         */
        std::list<RainfallEvent *> m_rainfallEvents;

        long water_first_plant_summary_index;


        // the number of plants, branch orders, and summaries
        long water_number_of_plants;
        long water_number_of_branch_orders;

        long NUM_X, NUM_Y, NUM_Z; // number of layers in each dimension
        BoxIndex BOX_COUNT; // How many boxes do we have?

        //
        // SharedAttribute support
        //
        // functions that we require to override as part of being a
        // SharedAttributeOwner
        ProcessSharedAttributeOwner* mySharedAttributeOwner;

        ScoreboardCoordinator* m_scoreboardCoordinator;

        // usually per-Plant, owned by this process
        int saWaterUptakePerPlantIndex;
        int saCumulWaterUptakePerPlantIndex;
        int saTotalPlantWaterUptakeIndex;
        int saWaterFluxPerPlantIndex;
        // usually per-Plant, owned by other processes
        int saRootRadiusPerPlantIndex;
        SharedAttribute* saRootRadius;

        // Weather Data SharedAttributes (scalar)
        SharedAttribute* saRainAmount;
        SharedAttribute* saPanEvaporation;
        SharedAttribute* saMaxTemperature;
        SharedAttribute* saMinTemperature;
        SharedAttribute* saRadiation;
        // Other scalar SharedAttributes
        SharedAttribute* saGreenCover;
        SharedAttribute* saAvgGreenCoverFactor;
        SharedAttribute* saCumulEvapBeforeGermination;
        SharedAttribute* saCumulEvapAfterGermination;
        SharedAttribute* saResidueCover;
        SharedAttribute* saDegreeDays;
        SharedAttribute* saGrowStartDegreeDays;
        SharedAttribute* saCalculateEvap;

        int saRootLengthPerPlantIndex;
        int saRootLengthPerPlantPerRootOrderIndex;
        int saGerminationLagIndex;
        int saTimeFullCoverIndex;
        int saTimeRipeningIndex;
        int saTimeHarvestIndex;
        int saDegDaysFullCoverIndex;
        int saDegDaysRipeningIndex;
        int saDegDaysHarvestIndex;
        int saGroundCoverMaxIndex;
        int saGroundCoverHarvestIndex;
        int saWaterUpregulationLimitIndex;
        int saDegreeDaysModelIndex;
        int saSeedingTimeIndex;
        int saNutrientSolutionIndex;
        int saNutrientSolnRenewIndex;

        static const size_t ROOT_ORDER_COUNT = 4;

        // 
        void PrepareSharedAttributes(ProcessActionDescriptor* action);


        /**
         *    MSA 09.09.29 Calculates root surface area.
         *    Caution: array bounds unchecked. Ensure that the array sizes of "radii" and "lengths" both equal "count".
         *    MSA TODO Convert this to use by-reference lists of radii and lengths, not pointers. Is it 1983? No?
         *    MSA 11.04.05 Decided to leave this as it is, to avoid complicating the code that calls it.
         */
        double RootSurfaceArea(const double* radii, const double* lengths, const size_t count) const;

        // Variable pointer to a const VOC.
        const VolumeObjectCoordinator* m_volumeObjectCoordinator;

        bool DoRedistribution(Scoreboard* scoreboard, const int& routineNumber, const BoxIndex& fromBox, const BoxIndex& toBox, const size_t& fromVOIndex, const size_t& toVOIndex, const double& fromProportion, const double& toProportion, const double& fromBoxDistance, const double& toBoxDistance, const ProcessTime_t& dT, const double& volumeObjectSurfacePermeability = 0);
        bool DoDrain(Scoreboard* scoreboard, const BoxIndex& fromBox, const BoxIndex& toBox, const size_t& fromVOIndex, const size_t& toVOIndex, const double& fromProportion, const double& toProportion, const double& boxTopFrom, const double& boxBottomFrom, const bool& isBottomLayer, const double& volumeObjectSurfacePermeability = 0);
    };

    inline double Water::RootSurfaceArea(const double* radii, const double* lengths, const size_t count) const
    {
        double sa = 0;
        for (size_t i = 0; i < count; ++i)
        {
            sa += radii[i] * lengths[i];
        }
        return 2.0 * PI * sa;
    }
} /* namespace rootmap */

#endif // #ifndef Water_H
