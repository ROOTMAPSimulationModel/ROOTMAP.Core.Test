#ifndef Plant_H
#define Plant_H
#include "simulation/process/common/Process.h"
#include "simulation/scoreboard/Scoreboard.h"

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

    typedef double_cs RootExtremesArray[Z+1][positive+1];


    // typedef enum
    // {
    //     plantpart_axis = 'axis',
    //     plantpart_branch = 'brch',
    //     plantpart_prebranchmarker = 'pbch',
    //     plantpart_roottip = 'rtip',
    //     plantpart_possibleaxis = 'paxs'
    //     
    // } PlantPart;

    class Plant : public Process
    {
        RootMapLoggerDeclaration();

        // #pragma mark "Data members"
        PlantType         * plant_type;
        DoubleCoordinate    plant_origin;
        DoubleCoordinate    plant_seed_location;
        long                plant_seeding_time;
        Axis              * plant_first_axis;
        Branch            * plant_first_branch;

        PlantCoordinator  & plant_coordinator;

        long                plant_total_branches;
        long                plant_total_tips;
        double_cs           plant_total_root_length;
        double_cs           plant_total_root_length_high;

        RootExtremesArray   plant_root_extremes;
        Boolean             plant_is_more_extreme;

        PossibleAxis      * plant_possible_seminal_axes;
        PossibleAxis      * plant_possible_nodal_axes;

    public:
        // #pragma mark "Construction"
        // The default constructor
        //Plant();
        explicit Plant (PlantCoordinator & coordinator);

        /// Initialisation
        virtual void Initialise(const ProcessDAI & data);

        // destruction
        ~Plant();

        // called by the default constructor
        void Defaults();

        // These two functions WOULD be used, however this class gets saved via the plantcoordinator,
        // and as such uses FillDescriptor, SaveToFile and IPlant(JSimulationFile *file)
        //    virtual void ReadProcessData (JSimulationFile *input_file, JSFDataInfo *data_info, ScoreboardCoordinator *sbl);
        //    virtual void SaveProcessData (JSimulationFile *output_file);

        // file access
        virtual Boolean DoesOverride() { return (true); }
        //    void FillDescriptor(JSFPlantDescriptor *descriptor);
        //    void SaveToFile(JSimulationFile *file);


        // #pragma mark "Making and Forgetting"
        void ForgetRoots();
        void ForgetBranch(Branch * b);

        Branch * MakeBranch(    DoubleCoordinate  * position,
            double_cs           length,
            double_cs           start_age,
            RootTip           * child,
            Branch            * child_branch,
            Branch            * parent_branch,
            BoxIndex            box,
            long int            branch_order,
            WrapDirection       wrap_direction);

        RootTip * MakeRootTip(  DoubleCoordinate  * position,
            DoubleCoordinate  * heading,
            double_cs           age,
            BoxIndex            box,
            long                branch_order,
            WrapDirection       wrap);

        Axis * MakeAxis     (   double_cs           start_lag_15C,
            DoubleCoordinate  * position,
            DoubleCoordinate  * start_orientation,
            AxisType            type_of_axis);

        int MakeSeminals();
        int MakeNodals();
        Boolean IsUnstarted();
        Boolean Start(ProcessActionDescriptor * action);

        Axis * MakeSeminal(const PossibleAxis & possible_seminal);
        int AddAxis(Axis * new_axis);

        // #pragma mark "Bookkeeping"
        void AdjustRootLength(double_cs length, DoubleCoordinate* old_position, BoxCoordinate* box_coord, BoxIndex box, DoubleCoordinate* new_position, long branch_order, WrapDirection wrapping);

        void IncrementBranchCount(DoubleCoordinate *position, long int branch_order);
        void IncrementBranchCount(BoxIndex box, long int branch_order, WrapDirection wrapping);
        void DecrementBranchCount(DoubleCoordinate *position, long int branch_order);
        void DecrementBranchCount(BoxIndex box, long int branch_order, WrapDirection wrapping);

        void IncrementTipCount(DoubleCoordinate *position, long int branch_order);
        void IncrementTipCount(BoxIndex box, long int branch_order, WrapDirection wrapping);
        void DecrementTipCount(DoubleCoordinate *position, long int branch_order);
        void DecrementTipCount(BoxIndex box, long int branch_order, WrapDirection wrapping);

        // #pragma mark "Extremes & Soilrects"
        void    GetSoilRect(DoubleRect* r, ViewDirection direction);
        void    UpdateExtremes(DoubleCoordinate* possible_new_extreme);
        Boolean IsMoreExtreme() { return (plant_is_more_extreme); }

        // #pragma mark "Growing"
        Branch* GrowRootTip(    RootTip*        root_tip, 
            long int        branch_order,
            double_cs            time_step_fraction,
            double_cs            time_step);

        void GrowBranch(        Branch*        this_branch,
            Branch*        last_branch,
            RootTip*        parent,
            long int        branch_order,
            double_cs            time_step);

        void GrowAxis(Axis* axis, double_cs time_step, double_cs adjusted_time);
        void StartAxis(Axis* axis, double_cs time_step, double_cs adjusted_time);
        void NextTimeStep(double_cs time_step);

        // #pragma mark "Branches"
        Branch* BranchEdit  (    Branch    * this_branch,
            Branch    * last_branch,
            RootTip   * parent,
            long int    branch_order,
            double_cs   time_step,
            double_cs   branch_lag_time);

        void WhereAndWhen   (    Branch    * this_branch,
            Branch    * last_branch,
            RootTip   * parent,
            double_cs   branch_lag_time,
            double_cs   time_step,
            BranchEditData * edit_data);

        Branch* InsertBranch(    Branch    * at_branch,
            long        branch_order,
            double_cs   time_step,
            BranchEditData * bed);

        void MoveBranch     (   Branch    * the_branch,
            RootTip   * the_parent,
            long        branch_order,
            double_cs   time_step,
            BranchEditData * bed);

        Branch* RemoveBranch(   Branch    * the_branch,
            RootTip   * the_parent,
            long int    branch_order);

        // #pragma mark "Drawing"
        virtual bool DoesDrawing() const;
        virtual bool DoesRaytracerOutput() const;
        virtual DoubleCoordinate GetOrigin();

        virtual void DrawScoreboard(const DoubleRect & area, Scoreboard * scoreboard);
        // MSA New DrawRoots method outsources specific drawing instructions
        // (e.g. DrawLine, DrawCylinder, etc) to the buffers themselves
        void DrawRoots(const DoubleRect & area, Scoreboard * scoreboard);
        void DrawBranch(const DoubleCoordinate & starting_point, Branch* this_branch,
            RootTip * parent, ProcessDrawing & drawing);

        long int DoRaytracerOutput(RaytracerData * raytracerData);
    private:
        // Private; used only by DoRaytracerDataCache() - caching a branch on its own makes no sense
        // and would introduce timing weirdness
        void RaytracerCacheBranch(const DoubleCoordinate & starting_point, Branch * this_branch, RootTip * parent, RaytracerData * raytracerData);
        // #pragma mark "TopicArrayPane Support"
        //void GetTopicName(StringPtr topicName);
        //void GetTopicDetail(CStringArray & topicDetail);


    private:
        //
        //
        // Shared Attributes
        SharedAttribute * saBranchLagTime;
        SharedAttribute * saDeflectionIndex;
        SharedAttribute * saFinalBranchInterval;
        SharedAttribute * saGeotropismIndex;
        SharedAttribute * saGrowthRateMax;
        SharedAttribute * saInitialBranchAngle;
        SharedAttribute * saInitialBranchInterval;
        SharedAttribute * saRootConductance;
        SharedAttribute * saTipGrowthDuration;

        SharedAttribute * saTotalRootLength;

        // These are usually per-PlantType, non-Scoreboard.
        SharedAttribute * saInitialSeminalDeflection;
        SharedAttribute * saGerminationLag;
        SharedAttribute * saTemperatureOfZeroGrowth;
        SharedAttribute * saFirstSeminalProbability;
        SharedAttribute * saPreviousTotalRootLength;
        SharedAttribute * saSeedingTime;

        // usually per-Plant, owned by other processes
        SharedAttribute * saNitrateUptake;
        SharedAttribute * saPhosphorusUptake;
        SharedAttribute * saWaterUptake;
        SharedAttribute * saSeedSize;
        SharedAttribute * saPlantIsLegume;
        //    SharedAttribute * sa;
        //    SharedAttribute * sa;

        // see roundup Issue 15 for the description of what is wrong here
        Scoreboard        * m_soil_scoreboard_Issue15;

        void PrepareSharedAttributes(ProcessActionDescriptor * action);

        // #pragma mark "DynamicResourceAllocation support"
    public:
        long int DynamicResourceAllocationToRoots(ProcessActionDescriptor * action);

    private:
        void InitialiseDRA(ProcessActionDescriptor * action);

        //TODO: initialise these in the constructor DONE
        bool plantDRAInitialised;
        ProcessTime_t plantPreviousTimestamp;

        double_cs plantCumulativeRootRes;
        double_cs plantResourceRatio;
        double_cs plantRemainNonNPResourceUnits;
        double_cs plantRemainNResourceUnits;
        double_cs plantSeedReserve_N;
        double_cs plantSeedReserve_P;
        double_cs plantSeedSupply_N;
        double_cs plantNoduleNumber;
        double_cs plantCumulativeNfix;

        double_cs plantNUptakePlasticity;
        double_cs plantPUptakePlasticity;
        int plantIsLegume;

        // MSA Map of all PlantElementIdentifiers and the branch orders they are assigned
        std::map<unsigned long, unsigned long> m_branch_order_map;
        // MSA Map of all new (not yet cached to m_raytracerData) PlantElementIdentifiers 
        std::map<unsigned long, bool> m_new_segment_map;
        unsigned long m_maxmap;

    };

    typedef std::vector< Plant * > PlantArray;
    typedef PlantArray::iterator   PlantArrayIterator;
    typedef std::list< Plant * > PlantList;

} // namespace rootmap

#endif // #ifndef Plant_H
