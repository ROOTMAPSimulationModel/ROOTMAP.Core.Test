#ifndef PlantSummaryRegistration_H
#define PlantSummaryRegistration_H
/////////////////////////////////////////////////////////////////////////////
// Name:        PlantSummaryRegistration.h
// Purpose:     Declaration of the PlantSummaryRegistration class
// Created:     2000
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/common/Structures.h"
#include "simulation/process/interprocess_communication/ProcessActionDescriptor.h"
#include "simulation/process/interprocess_communication/Message.h"
#include "simulation/process/plant/PlantFwd.h"
#include "core/log/Logger.h"

// macro for use in Processes' "Initialise", "StartUp", "WakeUp" message-handlers

namespace rootmap
{
    // Forward declarations
    class Process;
    class PlantCoordinator;
    class Plant;


    /**
     * This class is used by ProcessModules that wish to have PlantSummaries
     * created. Because it derives from SpecialProcessData, it participates in
     * PostOffice message sending.
     *
     * A PlantSummaryRegistration is a restricted super-set of a SharedAttributeFamily:
     *
     * It is like a SharedAttributeFamily in the way that it is used to form a set
     * of Characteristics, based on variations.
     *
     * It is restricted in that the variations are pre-defined (rather than
     * infinitely expandable).
     *
     * It is a super-set in so far as the "Variations" are more than would be
     * expected in a SharedAttributeFamily; the WrapOrder does not translate to a
     * Variation.
     *
     * This class uses, and is sympathetic to, SharedAttributes.  This makes
     * iterating over PlantSummaries by Plant the same as iterating over a
     * SharedAttribute. Searches can be conducted for PlantSummaries as if they were
     * a SharedAttributeFamily.
     */
    class PlantSummaryRegistration : public SpecialProcessData
    {
    public:
        Boolean IsPerBox();
        long GetStartIndex();
        long GetNumberOfPlants();
        size_t GetNumberOfSpatialSubsections();
        long GetNumberOfBranchOrders();

        WrapDirection GetWrapDirection();
        SummaryRoutine GetSummaryRoutine();

        static ScoreboardStratum GetSummaryRoutineStratum(SummaryRoutine sr);

        // Construction and Destruction
        //
        // Defaults to per-box, all plants (not individually), all branch/root orders
        // (not individually), no wrapping, no summary type.
        //
        // Combined with, for example a SummaryRoutine of srRootLength, this would
        // give a PlantSummary: Root Length Total.
        //
        // The reason for defaulting to no wrapping is on the basis that if one were
        PlantSummaryRegistration();

        // Mutation
        //
        // Register to Summarise per ScoreboardBox.  This is the default.  Set to
        // false for summarising on a global basis.
        void RegisterByPerBox(Boolean per_box);

        //
        // Register to summarise all plants as a group with one summary
        void RegisterAllPlants();

        //
        // Register for per-Plant summaries.
        void RegisterAllPlantCombos();

        //
        // Register for a particular branch/root order to be summarised.
        // Set to -1 for all branch/root orders in the one summary
        void RegisterBranchOrder(long a_branch_order);

        //
        // An alternative to RegisterBranchOrder(-1)
        void RegisterAllBranchOrders();

        //
        // Register for per-RootOrder summaries
        void RegisterAllBranchOrderCombos();

        //
        void RegisterAllVolumeObjects();

        //
        // Register for per-VO summaries
        void RegisterAllVolumeObjectCombos();

        //
        // Register for wrapping with x and/or y wrapping of root values.  This is
        // mostly provided because PlantSummary does also. 
        void RegisterWrapping(Boolean wrap_x, Boolean wrap_y);

        //
        // Register the type of summary required
        void RegisterSummaryType(SummaryRoutine summary_type);

        //
        // Register the ProcessModule that is doing the registering.  This is
        // required for 
        void RegisterModule(Process* module);

        //
        // Mirror of the PlantCoordinator function
        long int MakeScoreboardSummaries(ProcessActionDescriptor* action,
            PlantCoordinator* plantcoordinator
        );

        //
        // For cases where the summaries have already been made, but the dest process
        // still needs its DoScoreboardPlantSummaryRegistered() called
        long int AlreadyMadeScoreboardSummaries(ProcessActionDescriptor* action,
            PlantCoordinator* plantcoordinator
        );

    private:
        RootMapLoggerDeclaration();
        //    Distinguishes whether this is a per-plant or per-box summary
        // [informal KLUDGE]
        //#pragma mark "Only per-box ’till PlantSummary modified"
        Boolean psr_per_box;

        // if true, overrides the above, cycles through the list of plants
        Boolean psr_all_plants_individually;

        // Branch Order
        //    // -1 = ALL, other +ve = as expected
        //    long psr_branch_order;
        // if true, overrides the above, cycles thru’ the range of branch orders
        // WHICH MEANS per-RootOrder SharedAttributeVariation.
        Boolean psr_all_branch_order_combinations;

        Boolean psr_all_volume_object_combinations;

        // Wrapping values : filled in correctly by one of the accessing methods
        WrapDirection psr_wrap;

        // SummaryRoutine
        SummaryRoutine psr_summary_type;

        Process* psr_requesting_module;

        // These data members are filled in by the PlantCoordinator
        /* if only one summary was requested, this is its index/id#.
        If multiple were requested, this is the first scoreboard index */
        long psr_start_index;

        /* indicates to the requestor the number of plants used
        in making the summaries. This might be useful for a number
        of reasons, including looping through the plant summaries */
        long psr_number_of_plants;

        /* indicates to the requestor the number of branch orders used
        in making the summaries. This might be useful for a number
        of reasons, including looping through the branch orders */
        long psr_number_of_branch_orders;

        /* indicates to the requestor the number of VolumeObject-wise spatial subsections
        used in making the summaries. This might be useful for a number
        of reasons, including looping through the spatial subsections */
        long psr_number_of_spatial_subsections;

        /* this is how many summaries were produced in all. Naturally,
        this only applies to multiple summary requests. The requesting
        module would normally use this to keep count while looping
        through the summaries, doing what it does with them */
        long psr_number_of_summaries;


        friend class PlantCoordinator;

        friend bool operator==(const PlantSummaryRegistration& lhs,
            const PlantSummaryRegistration& rhs
            );
    public:
        bool GetNewPlantSummaryInformation(long int& rootOrder,
            Plant*& plant,
            PlantSet& plants,
            size_t& voIndex
        );

    private:
        // super secret implementation details
        //    PlantArrayIterator * myPlantIterator;
        PlantArray::iterator myPlantIterator;
        Plant* myPlant;
        PlantCoordinator* myPlantcoordinator;
        long int myRootOrderIterator;
        size_t myVOIndexIterator;
    };

    inline WrapDirection PlantSummaryRegistration::GetWrapDirection()
    {
        return psr_wrap;
    }

    inline SummaryRoutine PlantSummaryRegistration::GetSummaryRoutine()
    {
        return psr_summary_type;
    }
} /* namespace rootmap */

#endif // #ifndef PlantSummaryRegistration_H
