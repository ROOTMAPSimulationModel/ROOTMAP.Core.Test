#ifndef Nitrate_H
#define Nitrate_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ClassTemplate.h
// Purpose:     Declaration of the ClassTemplate class
// Created:     DD/MM/YYYY
// Author:      Vanessa Dunbabin
// $Date: August 2010 $
// $Revision: 2 $
// Copyright:   ©2006 University of Tasmania
//
// NitrateLeaching.h This is the nitrate leaching header file.  It keeps track of little packets of
//nitrate each one being 25 ugN.  These little nitrates are summed to give an average nitrate
//concentration for the box.  The nitrate leaching uses the water movement process to determine
//the nitrate movement.  Each nitrate will move according to a distribution of which the mean
//corresponds to the amount of water that has gone passed the nitrate and the variance is dependent
//on both the mean and the displacement dependent dispersivity.
//
// Author:             Vanessa Dunbabin
// Initial Development:     April 1998
// Latest Revision:         20 July 1998 VMD
//                    revised to use Rob's Node Coordinate Arrays
//
/////////////////////////////////////////////////////////////////////////////


//other files whose definitions we use in this file
#include "simulation/process/common/Process.h"
#include "core/common/DoubleCoordinates.h"
#include "core/log/Logger.h"
#include "simulation/process/raytrace/RaytracerData.h"
#include "simulation/process/modules/VolumeObjectCoordinator.h"

namespace rootmap
{
    // Forward declarations.
    class Node;
    class ProcessSharedAttributeOwner;
    class SharedAttribute;

    // MSA 09.11.02 Imported from the now-obsolete DoubleCoordinateNodeArray
    struct DoubleCoordinateNitrate : public DoubleCoordinate
    {
        double s;
        BoxIndex containingBox;
        size_t containingSubsection;
        bool f; // MSA "Is this Nitrate from fertiliser?" true/false
        bool containingBoxSubsectionKnown; // If false, code handling this DCN will re-ascertain its containing BoxIndex. DO NOT manually flip this from false to true!
    private:
        bool draw; // MSA 09.11.16 "Should this Nitrate packet be drawn?" true/false

    public:
        // default constructor
        DoubleCoordinateNitrate()
            : DoubleCoordinate()
            , s(0)
            , f(false)
            , draw(true)
            , containingBox(InvalidBoxIndex)
            , containingSubsection(4294960000)
            , containingBoxSubsectionKnown(false)
        {
        }

        // default constructor with draw setting
        DoubleCoordinateNitrate(const bool d)
            : DoubleCoordinate()
            , s(0)
            , f(false)
            , draw(d)
            , containingBox(InvalidBoxIndex)
            , containingSubsection(4294960000)
            , containingBoxSubsectionKnown(false)
        {
        }

        // copy-ish constructor
        DoubleCoordinateNitrate(const DoubleCoordinate& dc)
            : DoubleCoordinate(dc)
            , s(0)
            , f(false)
            , draw(true)
            , containingBox(InvalidBoxIndex)

            , containingSubsection(4294960000)
            , containingBoxSubsectionKnown(false)
        {
        }

        // copy-ish constructor with draw setting
        DoubleCoordinateNitrate(const DoubleCoordinate& dc, const bool d)
            : DoubleCoordinate(dc)
            , s(0)
            , f(false)
            , draw(d)
            , containingBox(InvalidBoxIndex)
            , containingSubsection(4294960000)
            , containingBoxSubsectionKnown(false)
        {
        }

        // MSA 10.11.10 Use/modify/clone this method to apply different conditions to the drawing of a packet.
        inline const bool DoDraw(const double& minPacketSizeToDraw) const
        {
            // For example, right now we do not want to draw partial packets (so we provide this method with a parameter of m_NITRATE_PACKET_SIZE).
            return draw && s >= minPacketSizeToDraw;
        }

        inline void SetContainingBoxSubsection(const BoxIndex& bi, const size_t& subsection)
        {
            containingBox = bi;
            containingSubsection = subsection;
            containingBoxSubsectionKnown = true;
        }

        // MSA 10.11.10 This method is necessitated by the change of 'draw' to a private member.
        // MSA 11.04.14 No longer used
        /*
        inline const bool doUndraw() const
        {
            return draw && s==0;
        }
        */
    };

    class Nitrate : public Process
    {
    public:
        // this is wx's macro for dynamic by-name instantiation
        DECLARE_DYNAMIC_CLASS(Nitrate)
        // this is rootmap's macro to force this class to be linked into the executable
        // (otherwise the linker strips it out because there is no explicit use of the class)
        DECLARE_DYNAMIC_CLASS_FORCE_USE(Nitrate)

        Nitrate();
        ~Nitrate();

        //virtual long int WakeUp(ProcessActionDescriptor *action);
        // ///////////////
        // Note: don't implement overridden virtual methods inline in header file
        virtual long int GetPeriodicDelay(long int);
        virtual long int DoNormalWakeUp(ProcessActionDescriptor* action);
        virtual long int Register(ProcessActionDescriptor* action);
        virtual long int Initialise(ProcessActionDescriptor* action);
        virtual long int StartUp(ProcessActionDescriptor* action);
        virtual long int DoScoreboardPlantSummaryRegistered(ProcessActionDescriptor* action);
        virtual long int DoExternalWakeUp(ProcessActionDescriptor* action);
        virtual long int DoGeneralPurpose(ProcessActionDescriptor* action); // 'GPm*'

        long InitialisePackets(ProcessActionDescriptor* action);

        // MSA 10.11.02 Overriding the Process implementation of this method
        // to create per-VolumeObject variants of Characteristics.
        // This implementation calls the Process method when it's done.
        virtual void Initialise(const ProcessDAI& data);

        // 20030308 RvH - don't inline virtual overrides
        virtual bool DoesOverride() const;
        virtual bool DoesDrawing() const;

        // MSA adding support for raytracing nitrates
        virtual bool DoesRaytracerOutput() const;
        virtual long int DoRaytracerOutput(RaytracerData* raytracerData);

        virtual void SetVolumeObjectCoordinator(const VolumeObjectCoordinator& voc);

        // //////////////
        // Drawing
        virtual void DrawScoreboard(const DoubleRect& area, Scoreboard* scoreboard);

        long int LeachN(ProcessActionDescriptor* action);

        long int UptakeN(ProcessActionDescriptor* action);

        long int DiffuseN(ProcessActionDescriptor* action);

        long int MassFlowN(ProcessActionDescriptor* action);

        long int RedistributeDots(ProcessActionDescriptor* action);

        /**
         *    MSA 09.10.28 Changed return type to bool (i.e. "Did diffusion take place?")
         *    MSA 11.02.24 Added two optimisation parameters, transferDimension and transferInPositiveDir. Use them if transfer plane and dir are known to the caller (saves having to recalculate them)
         */
        bool CalculateNitrateDiffusion(ProcessActionDescriptor* action, const long& this_x, const long& this_y, const long& this_z, const long& next_x, const long& next_y, const long& next_z, const ProcessTime_t& dT, Dimension transferDimension = NoDimension, bool transferInPositiveDir = true);

        void CalculateNitrateMineralisation(ProcessActionDescriptor* action, const ProcessTime_t& dT);
        void CalculateSoilTemp(ProcessActionDescriptor* action, const bool initialise);

    private:

        RootMapLoggerDeclaration();
        ProcessTime_t m_prev_soln_change;
        ProcessTime_t m_time_between_wakings;
        ProcessTime_t m_time_prev_waking;
        ProcessTime_t m_time_of_previous_diffusion;
        ProcessTime_t m_time_prev_temp;
        ProcessTime_t m_time_of_prev_solution_change;
        ProcessTime_t m_time_of_prev_nutr_rnw;
        double m_total_upper_uptake;
        double m_total_lower_uptake;
#if defined NITRATE_USE_CUMULATIVE_NITRATE_UPTAKE
        double Cumulative_Nitrate_Uptake, Cumulative_Nitrate_Uptake1, Cumulative_Nitrate_Uptake2, Cumulative_Nitrate_Uptake3, Cumulative_Nitrate_Uptake4;
#endif // #if defined NITRATE_USE_CUMULATIVE_NITRATE_UPTAKE

        double m_cumulativeMineralisation;

        long int itsWaterModuleID;
        long int itsPlantCoordinatorModuleID;

        // Nitrate packet storage and iteration
        std::list<DoubleCoordinateNitrate> m_nitrateList;
        typedef std::list<DoubleCoordinateNitrate>::iterator NitrateIterator;
        typedef std::list<DoubleCoordinateNitrate>::const_iterator ConstNitrateIterator;

        CharacteristicIndices Rained_Amount_Indices, Wetting_Front_Indices, Drained_Upper_Limit_Indices;
        CharacteristicIndices Water_Content_Indices, Water_Moved_Indices, Direction_Moved_Indices, Dispersivity_Indices;
        CharacteristicIndices Nitrate_Leached_Indices, Nitrate_Concentration_Indices, Nitrate_Amount_Indices, Nitrate_Renew_Indices, Nitrate_AmountCheck_Indices;
        CharacteristicIndices Rain_Out_Indices, FertFracN_Indices;
        CharacteristicIndices Coeff_C_Indices, Coeff_D_Indices, FertiliserN_Indices, BackgroundN_Indices, Added_N_Indices, Added_N2_Indices;
        CharacteristicIndices Rate_Constant_Indices, Q10_Indices;
        CharacteristicIndices Mineralisable_N_Indices, Wilting_Point_Indices;
        CharacteristicIndices Mineralised_N_Indices, Bulk_Density_Indices;
        CharacteristicIndices Packet_Size_Indices, Rate_Constant_Temperature_Indices;
        CharacteristicIndices Thermal_Diffusivity_Indices;

        CharacteristicIndex Estimate_Soil_Temp_Index, Radiation_Extinction_Coeff_Index, Temperature_Index;

        long NUM_X, NUM_Y, NUM_Z; // number of layers in each dimension
        BoxIndex BOX_COUNT; // How many boxes do we have?

        static const int ROOT_ORDER_COUNT = 4;

        Process* nitrate_PlantCoordinatorModule;

        long nitrate_first_plant_summary_index;

        // the number of plants, branch orders, and summaries
        long nitrate_number_of_plants;
        long nitrate_number_of_branch_orders;

        //
        // SharedAttribute support
        //
        // functions that we require to override as part of being a
        // SharedAttributeOwner
        ProcessSharedAttributeOwner* mySharedAttributeOwner;

        // 20030308 RvH
        void PrepareSharedAttributes(ProcessActionDescriptor* /*action*/);
        /**
         *  09.10.20 MSA Private helper function for recalculating Nitrate amount in each ScoreboardBox
         */
        void RecalculateNitrateAmounts(Scoreboard* scoreboard, ScoreboardCoordinator* scoreboardcoordinator, VolumeObjectCoordinator* volumeobjectcoordinator);

        // MSA 10.11.11 Another private helper function. Counts all nitrate (amount) in the Scoreboard
        double CountTotalNitrates(const Scoreboard* scoreboard) const;

        /**
         *    09.10.26 MSA Private helper function for generating a new pseudorandom nitrate position within the given box coordinates
         */
        void RandomiseNitratePosition(DoubleCoordinate* dc, const double& boxleft, const double& boxfront, const double& boxtop, const double& boxwidth, const double& boxdepth, const double& boxheight);

        // MSA 11.01.27 New private helper method.
        bool DoDiffusion(Scoreboard* scoreboard, const BoxIndex& fromBox, const BoxIndex& toBox, const size_t& fromVOIndex, const size_t& toVOIndex, const double& fromProportion, const double& toProportion, const double& avgDistance, const ProcessTime_t& dT, const double& volumeObjectSurfacePermeability = 0);

        int saWaterFluxPerPlantPerRootOrderIndex;
        int saAbsorptionPowerPerPlantIndex;
        int saNitrateUptakePerPlantIndex;
        int saCumulNitrateUptakePerPlantIndex;
        int saCumulFertNUptakePerPlantIndex;
        int saNitrateUptakePerPlantPerRootOrderIndex;
        int saLocalNitrateUptakePerPlantIndex;
        int saLocalFertNUptakePerPlantIndex;
        int saGerminationLagIndex;
        int saAddFertNIndex;
        int saRootLengthPerPlantIndex;
        int saRootLengthPerPlantPerRootOrderIndex;
        int saRootRadiusPerPlantPerRootOrderIndex;
        int saBaseTempIndex;
        int saSeedingTimeIndex;
        int saNutrientSolutionIndex;
        int saNutrientSolnRenewIndex;
        int saNutrientRenewIndex;

        int saRootLengthPerPlantPerRootOrderBoogalooIndex;
        int saRootLengthPerPlantBoogalooIndex;

        // Pertinent Weather Data SharedAttributes (scalar)
        SharedAttribute* saMaxTemperature;
        SharedAttribute* saMinTemperature;
        // Other scalar SharedAttributes
        SharedAttribute* saGreenCover;
        SharedAttribute* saDegreeDays;
        SharedAttribute* saGrowStartDegreeDays;
        SharedAttribute* saMineralisationDepth;
        SharedAttribute* saCalcNitrateMineralisation;

        wxColour m_dotColour;

        const double m_NITRATE_DRAW_PROPORTION;
        const double m_DIFFUSION_COEFF_SOLN;

        // These three variables are logically const, but can't be properly initialised upon construction (because we don't know until Initialise() is called)
        bool m_IS_NUTRIENT_SOLUTION;
        bool m_CALCULATE_NITRATE_MINERALISATION;
        double m_NITRATE_PACKET_SIZE;

        // Variable pointer to a const VOC.
        const VolumeObjectCoordinator* m_volumeObjectCoordinator;

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
    };

    inline void Nitrate::RandomiseNitratePosition(DoubleCoordinate* dc, const double& boxleft, const double& boxfront, const double& boxtop, const double& boxwidth, const double& boxdepth, const double& boxheight)
    {
        using Utility::Random1;
        dc->x = boxleft + (Random1() * boxwidth);
        dc->y = boxfront + (Random1() * boxdepth);
        dc->z = boxtop + (Random1() * boxheight);
        //I was having problems with nitrate packets falling exactly on a box edge
        //and the computer not knowing which box it was in->
        if (dc->z == boxtop) { dc->z += 0.01; }
        if (dc->z == (boxtop + boxheight)) { dc->z -= 0.01; }
        if (dc->x == boxleft) { dc->x += 0.01; }
        if (dc->x == (boxleft + boxwidth)) { dc->x -= 0.01; }
        if (dc->y == boxfront) { dc->y += 0.01; }
        if (dc->y == (boxfront + boxdepth)) { dc->y -= 0.01; }
    }
} /* namespace rootmap */

#endif // #ifndef Nitrate_H
