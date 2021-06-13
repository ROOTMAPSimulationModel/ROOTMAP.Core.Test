#ifndef PlantCoordinator_H
#define PlantCoordinator_H

#include "simulation/process/common/Process.h"
#include "simulation/process/plant/PlantFwd.h"
#include "simulation/process/common/ProcessGroupLeader.h"

#include "core/scoreboard/ScoreboardBox.h"
#include "core/common/Structures.h"
#include "core/log/Logger.h"


namespace rootmap
{
    // "Forward Declarations"
    class Plant;
    class PlantSummary;
    class Scoreboard;
    class ProcessActionDescriptor;
    class ScoreboardCoordinator;
    class VolumeObjectCoordinator;
    class ScoreboardStratum;
    class SimulationEngine;
    class ProcessSharedAttributeOwner;

    struct ScoreboardBoxIndex;

    class PlantCoordinator
        : public ProcessGroupLeader
    {
    public:
        DECLARE_DYNAMIC_CLASS(PlantCoordinator)

        // #pragma mark "Construction, Destruction"
        PlantCoordinator();
        void Initialise(SimulationEngine& engine);

        ~PlantCoordinator();

        // #pragma mark "Characteristic Access"
        virtual bool DoesOverride() const;

        // #pragma mark "Interprocess Activity"
        virtual long int Register(ProcessActionDescriptor* action);
        virtual long int Initialise(ProcessActionDescriptor* action);
        virtual long int StartUp(ProcessActionDescriptor* action);
        virtual long int Ending(ProcessActionDescriptor* action);
        virtual long int WakeUp(ProcessActionDescriptor* action);

        // Responses to "Standard Interprocess Communications Suite" messages
        virtual long int DoNormalWakeUp(ProcessActionDescriptor* action); // 'Nrml'
        virtual long int DoDelayedReceivalReaction(ProcessActionDescriptor* action); // 'ReDR'
        virtual long int DoExternalWakeUp(ProcessActionDescriptor* action);
        virtual long int DoScoreboardPlantSummaryRegistered(ProcessActionDescriptor* action);

        virtual long int DoRegisterGlobalPlantSummary(ProcessActionDescriptor* action); // 'ReDR'
        virtual long int DoRegisterScoreboardPlantSummary(ProcessActionDescriptor* action); // 'ReDR'

        ScoreboardCoordinator* GetScoreboardCoordinator();
        const VolumeObjectCoordinator* GetVolumeObjectCoordinator();
        void SetVolumeObjectCoordinator(const VolumeObjectCoordinator& voc);
        //void SetScoreboardCoordinator(ScoreboardCoordinator * scoreboardcoordinator);

        // Mandatory overrides (abstract in ProcessGroupLeader)
        virtual Process* FindProcessByProcessID(ProcessIdentifier process_id);
        virtual Process* FindProcessByProcessName(const std::string& pname);


        //
        //
        //
        //
        // #pragma mark "File Access"
        virtual bool DoesSaveProcessData() const;

        /**
         *    MSA N.B. The PlantCoordinator itself does not (currently)
         *    draw anything to the RaytracerData object.
         *    However, it may be the <source> class for the
         *    raytracer Output Rule, and as such needs access
         *    to the object pointer for outputting to file.
         */
        virtual bool DoesRaytracerOutput() const;
        virtual long int DoActivateRaytracerOutput();
        virtual long int DoRaytracerOutput(RaytracerData* raytracerData);
        virtual DoubleCoordinate GetOrigin() const;

        virtual ProcessTime_t GetPreviousTimeStamp() const;

    private:
        RootMapLoggerDeclaration();


    public:
        // ////////////////////////////////
        //      Plant
        // ////////////////////////////////
        void AddPlant(Plant* new_plant);

        void RemovePlant(Plant* plant_);

        Plant* FindPlantByProcessID(long plant_processID);

        Plant* FindPlantByPlantName(const std::string& search_name);

        bool DoesPlantExist(Plant* p);

        long GetNumPlants();

        Plant* GetPlant(long plant_number);

        const PlantArray& GetPlants() const;

        PlantArray& GetPlants();

    private:
        PlantIdentifier GetNewPlantId();

        void StartPlant(Plant* plant);


    public:
        // ////////////////////////////////
        //      PlantSummary
        // ////////////////////////////////

        void AddSummary(PlantSummary* new_summary);

        void DeleteSummary(PlantSummary* new_summary);

        void DeleteSummary(PlantSummaryIdentifier summary_id);

        PlantSummary* FindSummaryByCharacteristicIndex(CharacteristicIndex characteristic_index);

        PlantSummary* FindSummaryByID(PlantSummaryIdentifier summary_id);

        PlantSummaryIdentifier GetNewSummaryID(SummaryRoutineGroup summary_type_index, const ScoreboardStratum& volume);

        PlantSummaryArray& GetPlantSummaries();

        const PlantSummaryArray& GetPlantSummaries() const;

        // Summary Usage/Maintenance
        void AdjustRootLength(Plant* p, const double& length, const DoubleCoordinate* first_point, const BoxCoordinate* first_box_coord, const BoxIndex& first_box, const size_t& volumeObjectIndex, const DoubleCoordinate* last_point, const long& branch_order, const WrapDirection& wrap);
        void AdjustTipCount(Plant* p, const long count, const BoxIndex box, const DoubleCoordinate* position, const size_t& volumeObjectIndex, const long branch_order, const WrapDirection wrap);
        void AdjustBranchCount(Plant* p, long count, BoxIndex box, DoubleCoordinate* position, const size_t& volumeObjectIndex, long branch_order, WrapDirection wrap);


        // ////////////////////////////////
        //      PlantType
        // ////////////////////////////////
        void AddPlantType(PlantType* new_type);

        PlantType* FindPlantTypeByName(const std::string& name);

        PlantType* FindPlantTypeByID(long plant_type_id);

        long int GetNumPlantTypes();

        PlantTypeArray& GetPlantTypes();

        const PlantTypeArray& GetPlantTypes() const;

        PlantType* GetPlantType(long planttype_index);

    private:
        long GetNewPlantTypeID() const;


        // ////////////////////////////////
        //      Plant Parameters
        // ////////////////////////////////
    public:
        // #pragma mark "Plant Parameters"
        const double GetReferenceTemperature() const { return (reference_temperature); }
        void SetReferenceTemperature(const double& new_ref_temp) { reference_temperature = new_ref_temp; }
        const double TempAdjustRate(const double& rate_to_adjust, const BoxIndex& box, const double& temp_of_zero_growth) const;

        BoxIndex FindBoxByDoubleCoordinate(const DoubleCoordinate* position);
        BoxIndex FindBoxByDoubleCoordinate(const DoubleCoordinate* position, WrapDirection* wrapping);
        BoxIndex FindBoxByDoubleCoordinate(const DoubleCoordinate* position, WrapDirection* wrapping, BoxCoordinate* box_coord);


        ProcessSharedAttributeOwner* GetSharedAttributeOwner();

    private:

        // ////////////////////////////////
        //      SharedAttributes
        // ////////////////////////////////
        // #pragma mark "SharedAttribute support"
        //
        // SharedAttribute support
        //
        // functions that we require to override as part of being a
        // SharedAttributeOwner
        ProcessSharedAttributeOwner* mySharedAttributeOwner;


    private:
        // ////////////////////////////////
        //      Weird Data Members
        // ////////////////////////////////
        // #pragma mark "Data Members"
        //    long next_plant_characteristic_index[TOTAL_AVAILABLE_SCOREBOARDS];
        double plant_minimum;
        double plant_maximum;
        double plant_default;
        long int maximum_root_branching_order;
        double reference_temperature;
        long int temperature_index;//scoreboard index of the temperature characteristic
        ProcessTime_t plant_growth_time_step;
        //long int  time_of_prev_waking;
        ProcessTime_t m_previousWakingTimeStamp;
        ProcessTime_t time_betw_responses;
        long int itsNitrateModuleID;


        // ////////////////////////////////
        //      Lists of Plants, Types, Summaries, SummaryRegistrations
        // ////////////////////////////////
        PlantTypeArray m_plantTypes;

        PlantArray m_plants;

        PlantSummaryMultimap m_plantSummaries;
        PlantSummaryArray m_plantSummariesALL;

        ScoreboardCoordinator* m_scoreboardCoordinator;
        const VolumeObjectCoordinator* m_volumeObjectCoordinator;

        // see roundup Issue 15 for the description of what is wrong here
        Scoreboard* m_soil_scoreboard_Issue15;

        PlantSummaryRegistrationSet m_plantSummaryRegistrations;

        friend class Plant;
    };


    inline PlantArray& PlantCoordinator::GetPlants()
    {
        return m_plants;
    }

    inline const PlantArray& PlantCoordinator::GetPlants() const
    {
        return m_plants;
    }

    inline PlantTypeArray& PlantCoordinator::GetPlantTypes()
    {
        return m_plantTypes;
    }

    inline const PlantTypeArray& PlantCoordinator::GetPlantTypes() const
    {
        return m_plantTypes;
    }

    inline PlantSummaryArray& PlantCoordinator::GetPlantSummaries()
    {
        return m_plantSummariesALL;
    }

    inline const PlantSummaryArray& PlantCoordinator::GetPlantSummaries() const
    {
        return m_plantSummariesALL;
    }

    inline ScoreboardCoordinator* PlantCoordinator::GetScoreboardCoordinator()
    {
        return m_scoreboardCoordinator;
    }

    inline const VolumeObjectCoordinator* PlantCoordinator::GetVolumeObjectCoordinator()
    {
        return m_volumeObjectCoordinator;
    }

    inline ProcessSharedAttributeOwner* PlantCoordinator::GetSharedAttributeOwner()
    {
        return mySharedAttributeOwner;
    }

    // MSA 10.10.04
    // Overriding the default Process::GetPreviousTimeStamp.
    // The PlantCoordinator implementation now uses a second previous timestamp,
    // purely for timing plant growth. (Other factors such as output rules may
    // mess with the value of Process::m_previousTimeStamp, so we can't rely on it
    // to measure the time between growth steps.)
    //
    // [Implementation note: m_previousGrowthTimeStamp is, essentially, time_of_previous_waking.]
    inline ProcessTime_t PlantCoordinator::GetPreviousTimeStamp() const
    {
        return m_previousWakingTimeStamp;
    }


    struct ScoreboardBoxIndex
    {
        Scoreboard* scoreboard;
        BoxIndex boxIndex;

        ScoreboardBoxIndex(Scoreboard* scoreboard_, BoxIndex boxIndex_);
    };

    inline ScoreboardBoxIndex::ScoreboardBoxIndex(Scoreboard* scoreboard_, BoxIndex boxIndex_)
        : scoreboard(scoreboard_)
        , boxIndex(boxIndex_)
    {
    }
} /* namespace rootmap */

#endif // #ifndef PlantCoordinator_H
