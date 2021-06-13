#ifndef Plant_H
#define Plant_H
#include "simulation/process/common/Process.h"
#include "simulation/scoreboard/Scoreboard.h"
#include "simulation/process/modules/VolumeObjectCoordinator.h"

#include "core/common/Structures.h"
#include "core/common/DoubleCoordinates.h"
#include "core/log/Logger.h"
#include "core/scoreboard/ScoreboardBox.h"

#include <vector>
#include <list>

namespace rootmap
{
    class PlantType;
    class PlantCoordinator;
    class RootTip;
    class Branch;
    class Axis;
    class PossibleAxis;
    class SharedAttribute;
    class BranchEditData;
    class PlantDAI;
    class ProcessDAI;

    typedef double RootExtremesArray[Z + 1][positive + 1];

    class Plant : public Process
    {
        RootMapLoggerDeclaration();

        // #pragma mark "Data members"
        PlantType* m_plant_type;
        DoubleCoordinate m_plant_origin;
        DoubleCoordinate m_plant_seed_location;
        ProcessTime_t m_plant_seeding_time;
        Axis* m_plant_first_axis;
        Branch* m_plant_first_branch;

        PlantCoordinator& m_plant_coordinator;

        long m_plant_total_branches;
        long m_plant_total_tips;
        double m_plant_total_root_length;
        double m_plant_total_root_length_high;

        RootExtremesArray m_plant_root_extremes;
        bool m_plant_is_more_extreme;

        PossibleAxis* m_plant_possible_seminal_axes;
        PossibleAxis* m_plant_possible_nodal_axes;

    public:
        // #pragma mark "Construction"
        // The default constructor
        //Plant();
        explicit Plant(PlantCoordinator& coordinator);

        /// Initialisation
        virtual void Initialise(const ProcessDAI& data);

        // destruction
        ~Plant();

        // called by the default constructor
        void Defaults();

        // These two functions WOULD be used, however this class gets saved via the plantcoordinator,
        // and as such uses FillDescriptor, SaveToFile and IPlant(JSimulationFile *file)
        //	virtual void ReadProcessData (JSimulationFile *input_file, JSFDataInfo *data_info, ScoreboardCoordinator *sbl);
        //	virtual void SaveProcessData (JSimulationFile *output_file);

        // file access
        virtual bool DoesOverride() const { return (true); }
        //    void FillDescriptor(JSFPlantDescriptor *descriptor);
        //    void SaveToFile(JSimulationFile *file);


        // #pragma mark "Making and Forgetting"
        void ForgetRoots();
        void ForgetBranch(Branch* b);

        Branch* MakeBranch(DoubleCoordinate* position,
            double length,
            double start_age,
            RootTip* child,
            Branch* child_branch,
            Branch* parent_branch,
            BoxIndex box,
            long int branch_order,
            WrapDirection wrap_direction,
            AxisType axis_type);

        RootTip* MakeRootTip(DoubleCoordinate* position,
            Vec3d* heading,
            double age,
            BoxIndex box,
            long branch_order,
            WrapDirection wrap,
            AxisType axis_type);

        Axis* MakeAxis(double start_lag_15C,
            DoubleCoordinate* position,
            Vec3d* start_orientation,
            AxisType type_of_axis);

        int MakeSeminals();
        int MakeNodals();
        Boolean IsUnstarted();
        Boolean Start(ProcessActionDescriptor* action);

        Axis* MakeSeminal(const PossibleAxis& possible_seminal);
        int AddAxis(Axis* new_axis);

        // #pragma mark "Bookkeeping"
        void AdjustRootLength(const double& length,
            const DoubleCoordinate* old_position,
            const BoxCoordinate* box_coord,
            const BoxIndex& box,
            const size_t& volumeObjectIndex,
            const DoubleCoordinate* new_position,
            const long& branch_order,
            const WrapDirection& wrapping);

        void IncrementBranchCount(DoubleCoordinate* position, const size_t& volumeObjectIndex, long int branch_order);
        void IncrementBranchCount(BoxIndex box, DoubleCoordinate* position, const size_t& volumeObjectIndex, long int branch_order, WrapDirection wrapping);
        void DecrementBranchCount(DoubleCoordinate* position, const size_t& volumeObjectIndex, long int branch_order);
        void DecrementBranchCount(BoxIndex box, DoubleCoordinate* position, const size_t& volumeObjectIndex, long int branch_order, WrapDirection wrapping);

        void IncrementTipCount(DoubleCoordinate* position, const size_t& volumeObjectIndex, long int branch_order);
        void IncrementTipCount(BoxIndex box, DoubleCoordinate* position, const size_t& volumeObjectIndex, long int branch_order, WrapDirection wrapping);
        void DecrementTipCount(DoubleCoordinate* position, const size_t& volumeObjectIndex, long int branch_order);
        void DecrementTipCount(BoxIndex box, DoubleCoordinate* position, const size_t& volumeObjectIndex, long int branch_order, WrapDirection wrapping);

        // #pragma mark "Extremes & Soilrects"
        void GetSoilRect(DoubleRect* r, ViewDirection direction);
        void UpdateExtremes(const DoubleCoordinate* possible_new_extreme);
        Boolean IsMoreExtreme() { return (m_plant_is_more_extreme); }

        // #pragma mark "Growing"
        Branch* GrowRootTip(RootTip* root_tip,
            long int branch_order,
            double time_step_fraction,
            double time_step,
            AxisType axis_type);

        void GrowBranch(Branch* this_branch,
            Branch* last_branch,
            RootTip* parent,
            long int branch_order,
            double time_step,
            AxisType axis_type);

        void GrowAxis(Axis* axis, double time_step, double adjusted_time);
        void StartAxis(Axis* axis, double time_step, double adjusted_time);
        void NextTimeStep(const ProcessTime_t& time_step);

        // #pragma mark "Branches"
        Branch* BranchEdit(Branch* this_branch,
            Branch* last_branch,
            RootTip* parent,
            long int branch_order,
            double time_step,
            double branch_lag_time,
            AxisType axis_type);

        void WhereAndWhen(Branch* this_branch,
            Branch* last_branch,
            RootTip* parent,
            double branch_lag_time,
            double time_step,
            BranchEditData* edit_data);

        Branch* InsertBranch(Branch* at_branch,
            long branch_order,
            double time_step,
            BranchEditData* bed,
            AxisType axis_type);

        void MoveBranch(Branch* the_branch,
            RootTip* the_parent,
            long branch_order,
            double time_step,
            BranchEditData* bed,
            AxisType axis_type);

        Branch* RemoveBranch(Branch* the_branch,
            RootTip* the_parent,
            long int branch_order);

        // #pragma mark "Drawing"
        virtual bool DoesDrawing() const;
        virtual bool DoesRaytracerOutput() const;
        virtual DoubleCoordinate GetOrigin() const;

        virtual void DrawScoreboard(const DoubleRect& area, Scoreboard* scoreboard);
        // MSA New DrawRoots method outsources specific drawing instructions
        // (e.g. DrawLine, DrawCylinder, etc) to the buffers themselves
        void DrawRoots(const DoubleRect& area, Scoreboard* scoreboard);
        void DrawBranch(const DoubleCoordinate& starting_point, Branch* this_branch, RootTip* parent);

        long int DoActivateRaytracerOutput();
        long int DoRaytracerOutput(RaytracerData* raytracerData);
    private:
        bool AboveGround(DoubleCoordinate* position, Vec3d* heading) const;

        void DrawRootTip(const DoubleCoordinate& origin, RootTip* rootTip);

        DoubleCoordinate GetGrowthDestination(const DoubleCoordinate& startPoint, const Vec3d& heading, const double& growthRate);

        Branch* GrowRootTipSegment(RootTip* root_tip, const DoubleCoordinate& startPoint, const Vec3d& undeflectedHeading, const double& growth_increment, const double& interval, BoxIndex& box, long& branch_order, AxisType& axis_type, WrapDirection& wrap_direction);

        // Private; used only by DoRaytracerDataCache() - caching a branch on its own makes no sense
        // and would introduce timing weirdness
        void RaytracerCacheBranch(const long& pNum, const DoubleCoordinate& starting_point, Branch* this_branch, RootTip* parent, RaytracerData* raytracerData);
        void RaytracerCacheRootTip(const long& pNum, const DoubleCoordinate& origin, RootTip* rootTip, RaytracerData* raytracerData);

        double GetRadius(const BoxIndex& box_index, const long& branch_order, const size_t& volumeObjectIndex) const;
        // #pragma mark "TopicArrayPane Support"
        //void GetTopicName(StringPtr topicName);
        //void GetTopicDetail(CStringArray & topicDetail);


    private:
        //
        //
        // Shared Attributes
        SharedAttribute* saSeminalBranchLagTime;
        SharedAttribute* saNodalBranchLagTime;
        SharedAttribute* saSeminalDeflectionIndex;
        SharedAttribute* saNodalDeflectionIndex;
        SharedAttribute* saSeminalFinalBranchInterval;
        SharedAttribute* saNodalFinalBranchInterval;
        SharedAttribute* saSeminalGeotropismIndex;
        SharedAttribute* saNodalGeotropismIndex;

        SharedAttribute* saInitialBranchAngle;
        SharedAttribute* saInitialBranchInterval;
        SharedAttribute* saRootConductance;
        SharedAttribute* saTipGrowthDuration;

        SharedAttribute* saSeminalGrowthRate;
        SharedAttribute* saNodalGrowthRate;
        SharedAttribute* saTotalRootLength;
        SharedAttribute* saRootLengthPerPlantPerRootOrder;
        SharedAttribute* saRootRadiusPerPlantPerRootOrder;
        SharedAttribute* saGrowthRateMax;
        SharedAttribute* saSeminalUnitGwthRte;
        SharedAttribute* saNodalUnitGwthRte;

        // These are usually per-PlantType, non-Scoreboard.
        SharedAttribute* saInitialSeminalDeflection;
        SharedAttribute* saGerminationLag;
        SharedAttribute* saTemperatureOfZeroGrowth;
        SharedAttribute* saFirstSeminalProbability;
        SharedAttribute* saPreviousTotalRootLength;
        SharedAttribute* saSeedingTime;
        SharedAttribute* saCotyLife;
        SharedAttribute* saLeafEmergence;

        // usually per-Plant, owned by other processes
        SharedAttribute* saNitrateUptake;
        SharedAttribute* saPhosphorusUptake;
        SharedAttribute* saWaterUptake;
        SharedAttribute* saPlantIsLegume;
        SharedAttribute* saOrganicExudate;
        SharedAttribute* saImaxDeclineCurve;
        SharedAttribute* saPConcMin;
        SharedAttribute* saNFluxMax;
        SharedAttribute* saPFluxMax;
        SharedAttribute* saNUptakeKineticCons;
        SharedAttribute* saPUptakeKineticCons;
        SharedAttribute* saPlasticityFactorP;
        SharedAttribute* saDownRegulationFactor;
        SharedAttribute* saPlasticityFactorN;
        SharedAttribute* saNRegulationFactor;
        SharedAttribute* saPRegulationFactor;
        SharedAttribute* saRemNResUnits;
        SharedAttribute* saRemPResUnits;
        SharedAttribute* saPlantTargetResRatio;
        SharedAttribute* saPlantActualResRatio;
        SharedAttribute* saNAbsorptionPower;
        SharedAttribute* saPAbsorptionPower;
        SharedAttribute* saCumulNFix;

        void PrepareSharedAttributes(ProcessActionDescriptor* action);

        // #pragma mark "DynamicResourceAllocation support"
    public:
        long int DynamicResourceAllocationToRoots(ProcessActionDescriptor* action);

    private:
        void InitialiseDRA(ProcessActionDescriptor* action);
        //double ImaxDeclineCurveFn(ProcessActionDescriptor *action,const int & ImaxDeclineCurve) const;
        const double ImaxDeclineCurveFn(ProcessActionDescriptor* action, const int& ImaxDeclineCurve) const;
        //void CalculateNutrientUptakePlasticity(ProcessActionDescriptor *action, double total_root_length, double N_resource_ratio, double P_resource_ratio);
        void CalculateNutrientUptakePlasticity(ProcessActionDescriptor* action, const double& plantActualResourceRatio, const double& avg_pot_NInflux, const double& avg_pot_PInflux, const double& N_root_fraction, const double& P_root_fraction);

        //TODO: initialise these in the constructor	MSA 09.11.17 DONE (m_plantDRAInitialised(false), m_plantPreviousTimestamp(0))
        bool m_plantDRAInitialised;
        ProcessTime_t m_plantPreviousTimestamp;

        double m_plantCumulativeRootRes;
        double m_plantRemainNResourceUnits;
        double m_plantNoduleNumber;
        double m_plantCumulativeNfix;
        bool m_plantIsLegume;

        bool m_raytracerOutputActive;

        // MSA Map of all new (not yet cached to raytracerData) PlantElementIdentifiers 
        // MSA 11.04.05 These booleans are not simply members of each PlantElement for two reasons:
        // 1) "Not yet cached to RaytracerData" is an implementation detail, not an intrinsic property of Branch etc. 
        // 2) When they have been read once, they are cleared. In most cases they will be stored only for a short time, 
        //    so there's no sense in maintaining a "isNew" boolean for the life of the PlantElement.
        std::map<PlantElementIdentifier, bool> m_new_segment_map;
        unsigned long m_maxmap;

        // Private members for Plant_DynamicResAlloctoRoots calculations

        // Scoreboard Indices
        CharacteristicIndices Root_Length_High_Indices, Liquid_Phase_P_Concentration_Indices, Nitrate_Amount_Indices, Nitrate_Concentration_Indices;
        CharacteristicIndices Internal_Workings_Indices, Other_Soil_Resources_Indices, Total_Labile_P_Indices;

        long NUM_X, NUM_Y, NUM_Z; // number of layers in each dimension
        BoxIndex BOX_COUNT; // Total number of boxes in the Scoreboard
        static const int ROOT_ORDER_COUNT = 4;

        const VolumeObjectList* m_volumeObjectListPtr;
    };

    typedef std::vector<Plant *> PlantArray;
    typedef PlantArray::iterator PlantArrayIterator;
    typedef std::list<Plant *> PlantList;
} /* namespace rootmap */

#endif // #ifndef Plant_H
