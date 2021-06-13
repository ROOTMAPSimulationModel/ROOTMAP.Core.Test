/////////////////////////////////////////////////////////////////////////////
// Name:        Plant.cpp
// Purpose:     Implementation of the Plant class
// Created:     05/04/1992
// Author:      RvH
// $Date: 2009-07-10 06:43:25 +0800 (Fri, 10 Jul 2009) $
// $Revision: 69 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/common/Constants.h"
#include "core/common/RmAssert.h"

#include "simulation/process/plant/PlantSummary.h"
#include "simulation/process/plant/PlantType.h"
#include "simulation/process/plant/Plant.h"
#include "simulation/process/plant/PlantCoordinator.h"
#include "simulation/process/plant/PossibleAxis.h"
#include "simulation/process/plant/Branch.h"
#include "simulation/process/plant/Axis.h"
#include "simulation/process/plant/RootTip.h"
#include "simulation/process/plant/PlantCalculations.h"

#include "simulation/process/shared_attributes/SharedAttributeSearchHelper.h"
#include "simulation/process/shared_attributes/SharedAttribute.h"
#include "simulation/process/shared_attributes/ProcessSharedAttributeOwner.h"

#include "simulation/process/common/Process.h"
#include "simulation/process/common/ProcessCoordinator.h"
#include "simulation/process/common/ProcessDrawing.h"

#include "simulation/scoreboard/Scoreboard.h"
#include "simulation/scoreboard/ScoreboardCoordinator.h"

#include "simulation/data_access/interface/PlantDAI.h"
#include "simulation/data_access/interface/ProcessDAI.h"
#include "simulation/process/raytrace/RaytracerData.h"

#include "core/utility/Utility.h"
#include <cstdlib>
#include <cmath>
#include <numeric>

namespace rootmap
{
    struct VOIntersectionData
    {
        VolumeObject* vo;
        std::vector<DoubleCoordinate> points;
        Vec3d finalHeading;
        VolumeObject::InteractionType interaction;

        VOIntersectionData(VolumeObject* v, std::vector<DoubleCoordinate> p, Vec3d f, VolumeObject::InteractionType it)
            : vo(v), points(p), finalHeading(f), interaction(it)
        {
        }
    };

    RootMapLoggerDefinition(Plant);

    using Utility::CSRestrict;
    using Utility::Random1;

#if 0
#define RootMapLogDebug_PlantLowLevel(msg) RootMapLogDebug(msg)
#else
#define RootMapLogDebug_PlantLowLevel(msg)
#endif

    class BranchEditData
    {
    public:
        // PARAMETERS PASSED INTO BranchEdit()
        Branch* this_branch;
        Branch* last_branch;
        RootTip* parent;
        long int branch_order;
        double time_step;
        double branch_lag_time;

        // BranchEdit() LOCAL VARIABLES
        Vec3d segment_heading;
        DoubleCoordinate next_position;
        double branch_interval;
        double branch_age;
        double time_step_fraction;
        Branch* return_branch;
        BoxIndex box;
        WrapDirection wrap;

        BranchEditData();
        BranchEditData(Branch* tb,
            Branch* lb,
            RootTip* pa,
            long int bo,
            double ts,
            double blt);
    };

#ifdef _CS_DEBUG_PLANT_NEWHEADING_UNITVECTOR_
    static long int do_unit_heading_check;
    static void DebugNewHeadingUnitVector(DoubleCoordinate *heading);
#endif


#ifdef PLANT_DEBUG_TIPCOUNT
#include <set>
    static std::set<RootTip *> plantRootTips;
#endif // #ifdef PLANT_DEBUG_TIPCOUNT


    /* __PLANT__PLANT__PLANT__PLANT__PLANT__PLANT__PLANT__PLANT__PLANT__
    Plant
    The one and original default constructor
    __PLANT__PLANT__PLANT__PLANT__PLANT__PLANT__PLANT__PLANT__PLANT__ */
    Plant::Plant(PlantCoordinator& coordinator)
        : Process()
        , saSeminalBranchLagTime(0)
        , saNodalBranchLagTime(0)
        , saSeminalDeflectionIndex(0)
        , saNodalDeflectionIndex(0)
        , saSeminalFinalBranchInterval(0)
        , saNodalFinalBranchInterval(0)
        , saSeminalGeotropismIndex(0)
        , saNodalGeotropismIndex(0)
        , saGrowthRateMax(0)
        , saSeminalGrowthRate(0)
        , saNodalGrowthRate(0)
        , saInitialBranchAngle(0)
        , saInitialBranchInterval(0)
        , saRootConductance(0)
        , saTipGrowthDuration(0)
        , saTotalRootLength(0)
        , saInitialSeminalDeflection(0)
        , saGerminationLag(0)
        , saTemperatureOfZeroGrowth(0)
        , saFirstSeminalProbability(0)
        , saNitrateUptake(0)
        , saPhosphorusUptake(0)
        , saWaterUptake(0)
        , saPreviousTotalRootLength(0)
        , saPlantIsLegume(0)
        , saOrganicExudate(0)
        , m_plantDRAInitialised(false)
        , m_plant_type(0)
        , m_plant_seeding_time(0)
        , m_plant_first_axis(0)
        , m_plant_first_branch(0)
        , m_plant_coordinator(coordinator)
        , m_plant_origin(0, 0, 0)
        , m_plant_seed_location(0, 0, 0)
        , m_plant_total_branches(0)
        , m_plant_total_tips(0)
        , m_plant_total_root_length(0.0)
        , m_plant_total_root_length_high(0.0)
        , m_plant_is_more_extreme(true)
        , m_plant_possible_seminal_axes(0)
        , m_plant_possible_nodal_axes(0)
        , m_plantPreviousTimestamp(0)
        , m_maxmap(m_new_segment_map.max_size() - 1)
        , m_raytracerOutputActive(false)
        , m_volumeObjectListPtr(__nullptr)
    {
        RootMapLoggerInitialisation("rootmap.Plant");
        Plant::Defaults();
    }

    void Plant::Initialise(const ProcessDAI& data)
    {
        Process::Initialise(data);

        try
        {
            const PlantDAI& plant_data = dynamic_cast<const PlantDAI &>(data);

            m_plant_type = m_plant_coordinator.FindPlantTypeByName(plant_data.getTypeName());
            m_plant_origin = plant_data.getOrigin();
            m_plant_seed_location = plant_data.getSeedLocation();
            m_plant_seeding_time = plant_data.getSeedingTime() * 24; //converting from days to hours
            m_plant_possible_seminal_axes = plant_data.getPossibleSeminals();
            m_plant_possible_nodal_axes = plant_data.getPossibleNodals();

            RootMapLogDebug("Initialised Plant " << plant_data.getTypeName());
        }
        catch (std::bad_cast& /*bad_cast_ex*/)
        {
            RootMapLogError("Could not cast ProcessDAI to PlantDAI");
            throw;
        }
    }


    /* __PLANT__PLANT__PLANT__PLANT__PLANT__PLANT__PLANT__PLANT__PLANT__
    ~Plant
    Destructs all the RootTips and Branches. Might be a good idea to consult
    RootMap on this one.
    __PLANT__PLANT__PLANT__PLANT__PLANT__PLANT__PLANT__PLANT__PLANT__ */
    Plant::~Plant()
    {
        PossibleAxis* possible_axis = 0;
        // while there is still some list left,
        while (m_plant_possible_seminal_axes)
        {
            // keep a pointer to the second node
            possible_axis = m_plant_possible_seminal_axes->GetNext();
            // forget the head node
            delete m_plant_possible_seminal_axes;
            // make the head what used to be the second node
            m_plant_possible_seminal_axes = possible_axis;
        }
        m_plant_possible_seminal_axes = 0;

        // while there is still some list left,
        while (m_plant_possible_nodal_axes)
        {
            // keep a pointer to the second node
            possible_axis = m_plant_possible_nodal_axes->GetNext();
            // forget the head node
            delete m_plant_possible_nodal_axes;
            // make the head what used to be the second node
            m_plant_possible_nodal_axes = possible_axis;
        }
        m_plant_possible_seminal_axes = 0;

        // WARNING: DO NOT DO THIS.  Call ForgetRoots()
        //delete m_plant_first_axis;
        //delete m_plant_first_branch;
        ForgetRoots();
    }

    void Plant::Defaults()
    {
        m_plant_type = 0;
        m_plant_seeding_time = 0;
        m_plant_first_axis = 0;
        m_plant_first_branch = 0;
        m_plant_total_branches = 0;
        m_plant_total_tips = 0;
        m_plant_total_root_length = 0.0;
        m_plant_total_root_length_high = 0.0;

        for (long i = negative; i <= positive; ++i)
        {
            for (int j = X; j < Z; ++j) // No_Dimension = X-1
            //      WAS:
            //        for (Dimension j=X ; j<Z ; j+=1) // No_Dimension = X-1
            {
                m_plant_root_extremes[j][i] = 0.0;
            }
        }
        m_plant_is_more_extreme = true;

        m_plant_possible_seminal_axes = 0;
        m_plant_possible_nodal_axes = 0;
#ifdef _CS_DEBUG_PLANT_NEWHEADING_UNITVECTOR_
        do_unit_heading_check = 0;
#endif
    }


    // void Plant::SaveToFile(JSimulationFile *file)
    // {
    // //#pragma message ("Plant::SaveToFile requires completion")
    // // need to save the actual axes, branches and tips
    //     JSFPlantPossibleAxisDescriptor descriptor;
    //     PossibleAxis *axis;
    //
    //     axis = m_plant_possible_seminal_axes;
    //     while (axis)
    //     {
    //         descriptor.start_lag = axis->possibleaxis_start_lag;
    //         descriptor.probability = axis->possibleaxis_probability;
    //         descriptor.axis_type = Seminal;
    //
    //         file->WriteSome((Ptr)&descriptor, sizeof(JSFPlantPossibleAxisDescriptor));
    //
    //         axis = axis->possibleaxis_next;
    //     }
    //
    //     axis = m_plant_possible_nodal_axes;
    //     while (axis)
    //     {
    //         descriptor.start_lag = axis->possibleaxis_start_lag;
    //         descriptor.probability = axis->possibleaxis_probability;
    //         descriptor.axis_type = Nodal;
    //
    //         file->WriteSome((Ptr)&descriptor, sizeof(JSFPlantPossibleAxisDescriptor));
    //
    //         axis = axis->possibleaxis_next;
    //     }
    //
    // }

    //
    //
    // Function:
    //  ForgetRoots
    //
    //
    // Description:
    //  De-allocates all axes, branches and root tips that form the virtual root
    //  system for this plant.
    //
    //
    // Working Notes:
    //  For technical assistance with this function and ForgetAxis and ForgetBranch,
    //  see the Rootmap 2.1 manual (section 5.1, p30).  This is partially recursive
    //  (one level per root order).
    //
    //
    // Modifications:
    //  20030724 RvH -
    //
    void Plant::ForgetRoots()
    {
        //    Axis * next_axis = 0;
        //    Axis * current_axis = m_plant_first_axis;
        //    while (current_axis != 0)
        //    {
        //        next_axis = current_axis->GetNext();
        //        ForgetBranch(current_axis);
        //        // current_axis->m_parentBranch is the next axis
        //        // current_axis->m_child is the first branch marker of the
        //        //      the actual root that grows as the axis
        //
        //        current_axis = next_axis;
        //    }
        ForgetBranch(m_plant_first_axis);

        m_plant_first_axis = 0;
        m_plant_first_branch = 0;
        m_plant_total_branches = 0;
        m_plant_total_tips = 0;
        m_plant_total_root_length = 0.0;
        m_plant_total_root_length_high = 0.0;
    }


    void Plant::ForgetBranch(Branch* b)
    {
        RootMapLogDebug_PlantLowLevel("ForgetBranch id:" << b->m_id);
        if (b == 0) return;

        Branch* next_branch = 0;
        Branch* current_branch = b;

        while (current_branch != 0)
        {
            //
            // must delete the child root tip. Must do so first, before 'b' becomes
            // invalidated by being deleted.
            RootTip* r = current_branch->m_child;

#ifdef PLANT_DEBUG_TIPCOUNT
            if (r != 0)
            {
                plantRootTips.erase(r);
                CS_DEBUG_STREAM << "Plant::ForgetBranch(current_branch=" << hex << current_branch
                    << " current_branch->child=" << r << dec
                    << ") TipCount=" << plantRootTips.size() << endl;
            }
#endif // #ifdef PLANT_DEBUG_TIPCOUNT

            delete r;

            next_branch = current_branch->m_parentBranch;

            ForgetBranch(current_branch->m_childBranch);
            //
            // the ForgetBranch function can delete the branch but can't update
            // the value of the pointer to it from the current_branch.
            current_branch->m_childBranch = 0;

            //
            // As the final manoevre, delete THE current branch
            delete current_branch;

            current_branch = next_branch;
        }
    }

    Branch* Plant::MakeBranch
    (DoubleCoordinate* position,
        double length,
        double start_age,
        RootTip* child,
        Branch* child_branch,
        Branch* parent_branch,
        BoxIndex box,
        long int branch_order,
        WrapDirection wrap_direction,
        AxisType axis_type)
    {
        const size_t idx = m_plant_coordinator.GetVolumeObjectCoordinator()->GetContainingVolumeObjectIndex(*position);

        Branch* new_branch = new Branch(position, length, start_age, child, child_branch, parent_branch, axis_type, branch_order, GetRadius(box, branch_order, idx));

        if (m_raytracerOutputActive)
        {
            // Mark new (not yet raytraced) root segments as such
            m_new_segment_map[new_branch->m_id] = true;
        }

        RootMapLogDebug_PlantLowLevel("MakeBranch id:" << new_branch->m_id);

        IncrementBranchCount(box, position, idx, branch_order, wrap_direction);

#if defined _CS_DEBUG_WINDOW_ && defined _CS_DEBUG_MAKE_BRANCH_
        std::string delimiter = ", ";
        std::string end_of_line = "\r";
        std::string pstr = "MakeBranch : ";
        std::string pstr2 = "";

        sprintEditCellText(position->x, pstr2);
        TCLpstrcat(pstr, pstr2);
        TCLpstrcat(pstr, delimiter);
        sprintEditCellText(position->y, pstr2);
        TCLpstrcat(pstr, pstr2);
        TCLpstrcat(pstr, delimiter);
        sprintEditCellText(position->z, pstr2);
        TCLpstrcat(pstr, pstr2);
        TCLpstrcat(pstr, end_of_line);
        write_(pstr);
#endif

        return (new_branch);
    }

    RootTip* Plant::MakeRootTip
    (DoubleCoordinate* position,
        Vec3d* heading,
        double age,
        BoxIndex box,
        long branch_order,
        WrapDirection wrap,
        AxisType axis_type)
    {
        DoubleCoordinate double_position(position->x, position->y, position->z);
        double initial_branch_angle = 0;

        if (box == 0)
            box = m_plant_coordinator.FindBoxByDoubleCoordinate(position, &wrap);

        const size_t idx = m_plant_coordinator.GetVolumeObjectCoordinator()->GetContainingVolumeObjectIndex(*position);

        if (branch_order > 0)
        {
            initial_branch_angle = saInitialBranchAngle->GetValue(box, branch_order - 1, idx);
        }

        RootTip* new_root_tip = new RootTip(double_position, age, *heading, branch_order, initial_branch_angle, GetRadius(box, branch_order, idx), axis_type);

        if (new_root_tip != 0)
        {
            if (m_raytracerOutputActive && m_new_segment_map.find(new_root_tip->m_id) == m_new_segment_map.end())
            {
                m_new_segment_map[new_root_tip->m_id] = true;
            }

            IncrementTipCount(box, &double_position, idx, branch_order, wrap);
#ifdef PLANT_DEBUG_TIPCOUNT
            //std::set<RootTip *> plantRootTips;
            plantRootTips.insert(new_root_tip);
            CS_DEBUG_STREAM << "Plant::MakeRootTip(RootTip=" << hex << new_root_tip
                << ") TipCount=" << dec << plantRootTips.size() << endl;
#endif // #ifdef PLANT_DEBUG_TIPCOUNT
            RootMapLogDebug_PlantLowLevel("MakeRootTip id:" << new_root_tip->m_id);
        }

        return (new_root_tip);
    }


    Axis* Plant::MakeAxis
    (double start_lag,
        DoubleCoordinate* position,
        Vec3d* start_orientation,
        AxisType type_of_axis)
    {
        Axis* new_axis = 0;

        new_axis = new Axis(start_lag, position, start_orientation, type_of_axis);

        RootMapLogDebug_PlantLowLevel("MakeAxis id:" << new_axis->m_id);

        return (new_axis);
    }

    Axis* Plant::MakeSeminal(const PossibleAxis& possible_seminal)
    {
        // Pseudorandomly select values between -0.5 and 0.5 for x and y
        const double x = Utility::RandomRange(0.5, -0.5);
        const double y = Utility::RandomRange(0.5, -0.5);

        //DoubleCoordinate start_orientation(0.0,0.0,1.0);
        Vec3d start_orientation;
        //axis->GetStartOrientation( &axis_start_orientation );
        start_orientation.x = x;
        start_orientation.y = y;
        start_orientation.z = (x > 0) ? -1.0 : 1.0;

        NewRootTipHeading(&start_orientation,
            RootTipDeflection(saInitialSeminalDeflection->GetValue()),
            RootTipOrientation(0));

        Axis* new_axis = MakeAxis(possible_seminal.GetStartLag(),
            &m_plant_seed_location,
            &start_orientation,
            Seminal);

        return new_axis;
    }

    int Plant::AddAxis(Axis* new_axis)
    {
        /* if "last_axis" is 0, this is the first to be made,
        so we need to latch it onto various data members */
        if (m_plant_first_axis == 0)
        {
            m_plant_first_axis = new_axis;
            m_plant_first_branch = new_axis;
        }
        else // this is not the first axis
        {
            m_plant_first_axis->SetNext(new_axis);
        }

        return 1;
    }

    int Plant::MakeSeminals()
    {
        Axis* return_axis = 0;

        while (m_plant_possible_seminal_axes != 0)
        {
            // Pop the next possible seminal axis off the list
            PossibleAxis* possible_seminal = m_plant_possible_seminal_axes->Pop();

            // If it is the only or last possible axis to be considered, let the list point to nil
            if (possible_seminal == m_plant_possible_seminal_axes) m_plant_possible_seminal_axes = 0;

            // if it survives coming into existence, set it on its way
            if (possible_seminal->GetProbability() > Utility::Random1())
            {
                return_axis = MakeSeminal(*possible_seminal);
                AddAxis(return_axis);
            }

            // erase the evidence. regain memory.
            delete possible_seminal;
        }

        return (0);
    }

    int Plant::MakeNodals()
    {
        //    long int possible_nodal_pair_counter = 0;
        //    long int total_possible_nodals;
        PossibleAxis* possible_nodal;

        while (m_plant_possible_nodal_axes != 0)
        {
            possible_nodal = m_plant_possible_nodal_axes->Pop();
            if (possible_nodal == m_plant_possible_nodal_axes)
            {
                m_plant_possible_nodal_axes = 0;
            }

            if (possible_nodal->GetProbability() > Utility::Random1())
            {
                Vec3d start_orientation1(0, 0, +1);

                NewRootTipHeading(&(start_orientation1),
                    PI / 2,
                    RootTipOrientation(0));
                // set the second of the pair heading in the opposite horizontal direction
                Vec3d start_orientation2(-start_orientation1.x, -start_orientation1.y, start_orientation1.z);

                Axis* new_axis = MakeAxis(possible_nodal->GetStartLag(),
                    &m_plant_origin,
                    &(start_orientation1),
                    Nodal);
                AddAxis(new_axis);

                new_axis = MakeAxis(possible_nodal->GetStartLag(),
                    &m_plant_origin,
                    &(start_orientation2),
                    Nodal);
                AddAxis(new_axis);
            }

            delete possible_nodal;
        }

        return 1;
    }

    Boolean Plant::IsUnstarted()
    {
        return ((m_plant_possible_nodal_axes != 0) || (m_plant_possible_seminal_axes != 0));
    }

    Boolean Plant::Start(ProcessActionDescriptor* action)
    {
        // MSA 11.06.13 Is there a less ugly way of doing this?
        // How do you declare a const reference class member, but not initialise it until later?
        m_volumeObjectListPtr = &(m_plant_coordinator.GetVolumeObjectCoordinator()->GetVolumeObjectList());

        PrepareSharedAttributes(action);

        MakeSeminals();
        MakeNodals();

        return (m_plant_first_axis != 0);
    }

    void Plant::AdjustRootLength
    (const double& length,
        const DoubleCoordinate* old_position,
        const BoxCoordinate* box_coord,
        const BoxIndex& box,
        const size_t& volumeObjectIndex,
        const DoubleCoordinate* new_position,
        const long& branch_order,
        const WrapDirection& wrapping)
    {
        m_plant_total_root_length += length;

        //TODO: This replacement for the "RootLength High" characteristic KLUDGE should
        //  in turn be replaced/fixed by using a PlantSummaryRegistration
        if (branch_order >= 2)
        {
            m_plant_total_root_length_high += length;
        }

        m_plant_coordinator.AdjustRootLength(this, length, old_position, box_coord, box, volumeObjectIndex, new_position, branch_order, wrapping);
    }

    void Plant::IncrementBranchCount(DoubleCoordinate* position, const size_t& volumeObjectIndex, long int branch_order)
    {
        BoxIndex box = 0;
        WrapDirection worder;
        box = m_plant_coordinator.FindBoxByDoubleCoordinate(position, &worder);
        Plant::IncrementBranchCount(box, position, volumeObjectIndex, branch_order, worder);
    }

    void Plant::IncrementBranchCount(BoxIndex box, DoubleCoordinate* position, const size_t& volumeObjectIndex, long int branch_order, WrapDirection wrapping)
    {
        ++m_plant_total_branches;
        m_plant_coordinator.AdjustBranchCount(this, +1, box, position, volumeObjectIndex, branch_order, wrapping);
#ifdef PLANT_DEBUG_BRANCHCOUNT
        CS_DEBUG_STREAM << "Plant::IncrementBranchCount(BoxIndex=" << box
            << ",RootOrder=" << branch_order
            << ",WrapDirection=" << wrapping
            << ") BranchCount=" << m_plant_total_branches << endl;
#endif // #ifdef PLANT_DEBUG_BRANCHCOUNT
    }

    void Plant::DecrementBranchCount(DoubleCoordinate* position, const size_t& volumeObjectIndex, long int branch_order)
    {
        BoxIndex box = 0;
        WrapDirection worder;
        box = m_plant_coordinator.FindBoxByDoubleCoordinate(position, &worder);
        Plant::DecrementBranchCount(box, position, volumeObjectIndex, branch_order, worder);
    }

    void Plant::DecrementBranchCount(BoxIndex box, DoubleCoordinate* position, const size_t& volumeObjectIndex, long int branch_order, WrapDirection wrapping)
    {
        --m_plant_total_branches;
        m_plant_coordinator.AdjustBranchCount(this, -1, box, position, volumeObjectIndex, branch_order, wrapping);
#ifdef PLANT_DEBUG_BRANCHCOUNT
        CS_DEBUG_STREAM << "Plant::DecrementBranchCount(BoxIndex=" << box
            << ",RootOrder=" << branch_order
            << ",WrapDirection=" << wrapping
            << ") BranchCount=" << m_plant_total_branches << endl;
#endif // #ifdef PLANT_DEBUG_BRANCHCOUNT
    }

    void Plant::IncrementTipCount(DoubleCoordinate* position, const size_t& volumeObjectIndex, long int branch_order)
    {
        WrapDirection worder;
        BoxIndex box = m_plant_coordinator.FindBoxByDoubleCoordinate(position, &worder);
        IncrementTipCount(box, position, volumeObjectIndex, branch_order, worder);
    }

    void Plant::IncrementTipCount(BoxIndex box, DoubleCoordinate* position, const size_t& volumeObjectIndex, long int branch_order, WrapDirection wrapping)
    {
        ++m_plant_total_tips;
        m_plant_coordinator.AdjustTipCount(this, +1, box, position, volumeObjectIndex, branch_order, wrapping);
#ifdef PLANT_DEBUG_TIPCOUNT
        CS_DEBUG_STREAM << "Plant::IncrementTipCount(BoxIndex=" << box
            << ",RootOrder=" << branch_order
            << ",WrapDirection=" << wrapping
            << ") TipCount=" << m_plant_total_tips << endl;
#endif // #ifdef PLANT_DEBUG_TIPCOUNT
    }

    void Plant::DecrementTipCount(DoubleCoordinate* position, const size_t& volumeObjectIndex, long int branch_order)
    {
        WrapDirection worder;
        BoxIndex box = m_plant_coordinator.FindBoxByDoubleCoordinate(position, &worder);
        DecrementTipCount(box, position, volumeObjectIndex, branch_order, worder);
    }

    void Plant::DecrementTipCount(BoxIndex box, DoubleCoordinate* position, const size_t& volumeObjectIndex, long int branch_order, WrapDirection wrapping)
    {
        --m_plant_total_tips;
        m_plant_coordinator.AdjustTipCount(this, -1, box, position, volumeObjectIndex, branch_order, wrapping);
#ifdef PLANT_DEBUG_TIPCOUNT
        CS_DEBUG_STREAM << "Plant::DecrementTipCount(BoxIndex=" << box
            << ",RootOrder=" << branch_order
            << ",WrapDirection=" << wrapping
            << ") TipCount=" << m_plant_total_tips << endl;
#endif // #ifdef PLANT_DEBUG_TIPCOUNT
    }

    void Plant::GetSoilRect(DoubleRect* r, ViewDirection direction)
    {
        switch (direction)
        {
        case vFront:
            r->left = m_plant_root_extremes[X][negative];
            r->right = m_plant_root_extremes[X][positive];
            r->top = m_plant_root_extremes[Z][negative];
            r->bottom = m_plant_root_extremes[Z][positive];
            break;

        case vSide:
            r->left = m_plant_root_extremes[Y][negative];
            r->right = m_plant_root_extremes[Y][positive];
            r->top = m_plant_root_extremes[Z][negative];
            r->bottom = m_plant_root_extremes[Z][positive];
            break;

        case vTop:
            r->left = m_plant_root_extremes[X][negative];
            r->right = m_plant_root_extremes[X][positive];
            r->top = m_plant_root_extremes[Y][negative];
            r->bottom = m_plant_root_extremes[Y][positive];
            break;
        }
    }

    bool Plant::AboveGround(DoubleCoordinate* position, Vec3d* heading) const
    {
        bool above_ground = ((position->z < 0) && (heading->z < 0));
        if (above_ground)
        {
            heading->z = -(heading->z);
            heading->Normalize(true);
        }
        return (above_ground);
    }

    Branch* Plant::GrowRootTip
    (RootTip* root_tip,
        long int branch_order,
        double time_step_fraction,
        double time_step,
        AxisType axis_type)
    {
        if (time_step == 0)
        {
            return __nullptr;
        }

        DoubleCoordinate old_position = root_tip->roottip_position;

        const size_t idx = m_plant_coordinator.GetVolumeObjectCoordinator()->GetContainingVolumeObjectIndex(old_position);

        WrapDirection wrap_direction;
        BoxCoordinate box_coord;
        BoxIndex box = m_plant_coordinator.FindBoxByDoubleCoordinate(&old_position, &wrap_direction, &box_coord);

        RootMapLogDebug_PlantLowLevel("GrowRootTip id:" << root_tip->m_id);

        // MSA TODO work out why SA and Characteristic storage of growth rates seem to be parallel, not one and the same

        const double growth_rate = (axis_type == Seminal) ? (saSeminalGrowthRate->GetValue(box, branch_order, idx)) : (saNodalGrowthRate->GetValue(box, branch_order, idx));

#ifdef _CS_DEBUG_
        //double tip_growth_duration = m_plant_coordinator.GetTipGrowthDuration(branch_order, box);
        double tip_growth_duration = saTipGrowthDuration->GetValue(box, branch_order, idx);

        if ((growth_rate_max < 0.01) || (tip_growth_duration < 1000))
        {
            Debugger();
        }
#endif // #ifdef _CS_DEBUG_

        //    growth_increment = m_plant_coordinator.TempAdjustRate(growth_rate_max, box, m_plant_type->GetTemperatureOfZeroGrowth()) * m_plant_coordinator.GetRootConductance(branch_order, box) * time_step_fraction * time_step;
        const double temperature_of_zero_growth = saTemperatureOfZeroGrowth->GetValue();
        const double root_conductance = saRootConductance->GetValue(box, branch_order, idx);
        const double temperature_adjusted_rate = m_plant_coordinator.TempAdjustRate(growth_rate, box, temperature_of_zero_growth);
        const double growth_increment = temperature_adjusted_rate * root_conductance * time_step_fraction * time_step;

        Vec3d the_roottip_heading = root_tip->roottip_heading;

        //VMD 26/1/99
        //The tip growth duration and growth rate max are set to zero in plant coordinator for a scoreboard box if it is dry and has been dry for some time, hence the roots should die.
        //if((tip_growth_duration == 0) && (growth_rate_max == 0))
        //{
        //    root_tip->roottip_dead = true;
        //}

        if (axis_type == Seminal)
        {
            NewRootTipHeading(&the_roottip_heading, RootTipDeflection(saSeminalDeflectionIndex->GetValue(box, branch_order, idx)), RootTipOrientation(saSeminalGeotropismIndex->GetValue(box, branch_order, idx)));
        }
        else if (axis_type == Nodal)
        {
            NewRootTipHeading(&the_roottip_heading, RootTipDeflection(saNodalDeflectionIndex->GetValue(box, branch_order, idx)), RootTipOrientation(saNodalGeotropismIndex->GetValue(box, branch_order, idx)));
        }

        the_roottip_heading.Normalize(true);
        DoubleCoordinate startPoint = old_position;
        DoubleCoordinate endPoint;
        do
        {
            endPoint = GetGrowthDestination(startPoint, the_roottip_heading, growth_increment);
        } while (AboveGround(&endPoint, &the_roottip_heading));

        RmAssert(Distance3D(&startPoint, &endPoint) - growth_increment < 1e-6, "Magnitude of root tip growth vector does not match growth_increment");

        const double interval = saInitialBranchInterval->GetValue(box, branch_order, idx);

        // This method will recursively generate as many "sub-segments" as necessary to deal with obstructions
        return GrowRootTipSegment(root_tip, startPoint, the_roottip_heading, growth_increment, interval, box, branch_order, axis_type, wrap_direction);
    }

    Branch* Plant::GrowRootTipSegment(RootTip* root_tip,
        const DoubleCoordinate& startPoint,
        const Vec3d& unitUndeflectedHeading,
        const double& nominalGrowthIncrement,
        const double& interval,
        BoxIndex& box,
        long& branch_order,
        AxisType& axis_type,
        WrapDirection& wrap_direction)
    {
        Branch* new_branch = __nullptr;

        // Queue of VolumeObject indices, one for each subsegment of this root tip segment
        std::queue<size_t> containingVOIndices;
        std::queue<double> actualSubsegmentLengths;
        const bool doIntersectionChecking = !m_volumeObjectListPtr->empty();

        // MSA 11.06.28 Note that here we examine a point shifted slightly along from startPoint, just in case startPoint is located directly ON a VO surface.
        containingVOIndices.push(m_plant_coordinator.GetVolumeObjectCoordinator()->GetContainingVolumeObjectIndex(startPoint + (2 * NEAR_AS_INTERSECTING) * unitUndeflectedHeading));

        DoubleCoordinate currentStartPoint = startPoint;
        DoubleCoordinate currentEndPoint = startPoint + nominalGrowthIncrement * unitUndeflectedHeading;
        DoubleCoordinate currentIntersectionPoint;

        double remainingGrowthIncrement = nominalGrowthIncrement;

        // Initialise the root tip heading to the non-deflected case (penetrated or did not encounter VolumeObject(s)).
        // This heading will be updated if any deflection is detected.
        root_tip->roottip_heading = unitUndeflectedHeading;
        Vec3d finalDeflectedHeading(unitUndeflectedHeading);

        bool intersectionHasOccurred = true; // Not actually necessarily true here, but used to go into the loop.
        while (intersectionHasOccurred)
        {
            intersectionHasOccurred = false;
            size_t idx = 0;

            if (doIntersectionChecking)
            {
                std::vector<VOIntersectionData> intersectedVOData;
                for (VolumeObjectList::const_iterator iter = m_volumeObjectListPtr->begin(); iter != m_volumeObjectListPtr->end(); ++iter)
                {
                    std::vector<DoubleCoordinate> resultPoints;
                    VolumeObject::InteractionType it = (*iter)->DeflectRootSegment(currentStartPoint, currentIntersectionPoint, currentEndPoint, finalDeflectedHeading, GetRadius(box, branch_order, (*iter)->GetIndex()), branch_order);
                    if (it != VolumeObject::NoInteraction)
                    {
                        resultPoints.push_back(currentStartPoint);
                        resultPoints.push_back(currentIntersectionPoint);
                        resultPoints.push_back(currentEndPoint);
                        intersectedVOData.push_back(VOIntersectionData((*iter), resultPoints, finalDeflectedHeading, it));
                    }
                }

                intersectionHasOccurred = !intersectedVOData.empty();
                std::vector<VOIntersectionData>::iterator iteratorToEarliestIntersected;

                if (intersectionHasOccurred)
                {
                    // Check all intersected VolumeObjects: process the earliest-intersected one only.
                    // MSA 11.05.02 Switched from using distances to squared distances, to avoid unnecessary square root computations
                    double distToClosestIntersectionSquared = std::numeric_limits<double>::max();
                    iteratorToEarliestIntersected = intersectedVOData.end();
                    for (std::vector<VOIntersectionData>::iterator iter = intersectedVOData.begin(); iter != intersectedVOData.end(); ++iter)
                    {
                        const double distSquared = DistanceSquared3D(&iter->points[0], &iter->points[1]);
                        if (distSquared < distToClosestIntersectionSquared)
                        {
                            distToClosestIntersectionSquared = distSquared;
                            iteratorToEarliestIntersected = iter;
                            idx = iter->vo->GetIndex();
                        }
                    }

                    currentStartPoint = iteratorToEarliestIntersected->points[0];
                    currentIntersectionPoint = iteratorToEarliestIntersected->points[1];
                    currentEndPoint = iteratorToEarliestIntersected->points[2];

                    const double thisSubsegmentLength = Distance3D(&currentStartPoint, &currentIntersectionPoint);

                    actualSubsegmentLengths.push(thisSubsegmentLength);

                    // Calculate growth increment after intersection.
                    // Remember that currentIntersectionPoint may not be exactly on the VO surface, as it is displaced slightly when the interaction is a Deflection.
                    // This is to simulate the effect of the root's radius.
                    // In the slight-displacement case, it is possible that the distance between start and intersection points may slightly exceed the remaining growth increment.
                    // When this happens, we just gloss over it and (when actually growing the root tip) let the slightly-displaced intersection point be the end point.
                    double growthIncrementAfterIntersection = Utility::CSMax(0.0, remainingGrowthIncrement - thisSubsegmentLength);

                    // MSA 11.06.21 Note that we don't want to use finalHeading here, because it does not necessarily parallel "currentEndPoint - currentIntersectionPoint".
                    Vec3d pseudoHeading(currentEndPoint.x - currentIntersectionPoint.x, currentEndPoint.y - currentIntersectionPoint.y, currentEndPoint.z - currentIntersectionPoint.z);

                    pseudoHeading.Normalize();
                    currentStartPoint = currentIntersectionPoint;

                    // Set the next subsegment's VO index.
                    if (iteratorToEarliestIntersected->interaction == VolumeObject::PenetrationToInside
                        || iteratorToEarliestIntersected->interaction == VolumeObject::DeflectionToInside)
                    {
                        containingVOIndices.push(iteratorToEarliestIntersected->vo->GetIndex());
                    }
                    else
                    {
                        // MSA 11.06.28 Note that here we examine a point shifted slightly along from startPoint, just in case startPoint is located directly ON a VO surface.
                        containingVOIndices.push(m_plant_coordinator.GetVolumeObjectCoordinator()->GetContainingVolumeObjectIndex(currentStartPoint + (2 * NEAR_AS_INTERSECTING) * pseudoHeading));
                    }

                    // Now, check if the growing conditions for the after-intersection subsegment have changed.
                    // This will only be the case if the root segment has penetrated the VO surface (and is thus in a different Scoreboard box subsection).
                    if (iteratorToEarliestIntersected->interaction == VolumeObject::PenetrationToOutside || iteratorToEarliestIntersected->interaction == VolumeObject::PenetrationToInside)
                    {
                        const VolumeObject* containerVO = __nullptr; // Initialise to the segment-is-outside-all-VOs case

                        // For the PenetrationToOutside case, we need to determine which VO (or none) the root segment is now in.
                        if (iteratorToEarliestIntersected->interaction == VolumeObject::PenetrationToOutside)
                        {
                            // MSA 11.06.28 Note that here we examine a point shifted slightly along from startPoint, just in case startPoint is located directly ON a VO surface.
                            containerVO = m_plant_coordinator.GetVolumeObjectCoordinator()->GetContainingVolumeObject(currentStartPoint + (2 * NEAR_AS_INTERSECTING) * pseudoHeading);
                        }
                        else
                        {
                            // The PenetrationToInside case is simple.
                            containerVO = iteratorToEarliestIntersected->vo;
                        }

                        // Determine the after-intersection subsegment's surroundings.
                        WrapDirection wd;
                        BoxCoordinate box_coord;
                        const BoxIndex boxAfterIntersection = m_plant_coordinator.FindBoxByDoubleCoordinate(&currentStartPoint, &wd, &box_coord);
                        const size_t voIndexAfterIntersection = containerVO == __nullptr ? 0 : containerVO->GetIndex();

                        // Now we must recalculate the increment after intersection.
                        // 1) Grab the values of a couple of variables from the start of this root segment...
                        const double growth_rate = (axis_type == Seminal) ? (saSeminalGrowthRate->GetValue(box, branch_order, idx)) : (saNodalGrowthRate->GetValue(box, branch_order, idx));
                        const double temperature_of_zero_growth = saTemperatureOfZeroGrowth->GetValue();
                        const double root_conductance = saRootConductance->GetValue(box, branch_order, idx);
                        const double temperature_adjusted_rate = m_plant_coordinator.TempAdjustRate(growth_rate, box, temperature_of_zero_growth);
                        // 2) Then substitute them back into the equation to work out the time component.
                        const double timeComponentAfterIntersection = growthIncrementAfterIntersection / (root_conductance * temperature_adjusted_rate);

                        // 3) Now update the variables for the new surroundings...
                        const double growthRateAfterIntersection = (axis_type == Seminal) ? (saSeminalGrowthRate->GetValue(boxAfterIntersection, branch_order, voIndexAfterIntersection))
                            : (saNodalGrowthRate->GetValue(boxAfterIntersection, branch_order, voIndexAfterIntersection));

                        const double rootConductanceAfterIntersection = saRootConductance->GetValue(boxAfterIntersection, branch_order, voIndexAfterIntersection);
                        const double tempAdjustedRateAfterIntersection = m_plant_coordinator.TempAdjustRate(growthRateAfterIntersection, boxAfterIntersection, temperature_of_zero_growth);

                        // 4) And use them to recalculate the after-intersection growth increment.
                        const double recalculatedGrowthIncrementAfterIntersection = tempAdjustedRateAfterIntersection * rootConductanceAfterIntersection * timeComponentAfterIntersection;

                        if (recalculatedGrowthIncrementAfterIntersection != growthIncrementAfterIntersection)
                        {
                            // Note that this won't end up punching a root segment through a curved BoundingCylinder wall, because it will only ever be called
                            // when a root segment has ALREADY punched through a wall :)
                            growthIncrementAfterIntersection = recalculatedGrowthIncrementAfterIntersection;
                            currentEndPoint = currentIntersectionPoint + growthIncrementAfterIntersection * pseudoHeading;
                        }
                    }
                    // Finally, update the root tip heading if and only if it has changed (i.e. the root has deflected).
                    else if (iteratorToEarliestIntersected->interaction == VolumeObject::DeflectionToOutside || iteratorToEarliestIntersected->interaction == VolumeObject::DeflectionToInside)
                    {
                        root_tip->roottip_heading = iteratorToEarliestIntersected->finalHeading;
                        root_tip->roottip_heading.Normalize(true);
                    }

                    remainingGrowthIncrement = growthIncrementAfterIntersection;
                } // if(intersectionHasOccurred)
                else
                {
                    actualSubsegmentLengths.push(remainingGrowthIncrement);
                }
            } // if(doIntersectionChecking)
            else
            {
                actualSubsegmentLengths.push(nominalGrowthIncrement);
            }

            // Must update the length in all cases.
            double length = root_tip->roottip_length;

            // Note that length is increased by remainingGrowthIncrement, which is NOT equal to the sum of distances between all points.
            // This is because if a BoundingCylinder's curved surface (for example) is intersected,
            // there will be a resulting intersection point and deflection end point which form the chord
            // between two points on the cross-section circle - but the actual root segment is considered to lie
            // along the circumference arc between the two points.
            length += remainingGrowthIncrement;

            root_tip->roottip_length = length;

            // If there was an intersection, this is not the last time through this loop.
            // Just add the intersection point as a midpoint.
            if (intersectionHasOccurred)
            {
                root_tip->roottip_midpoints.push_back(currentIntersectionPoint);
            }
            // Otherwise, finally, do the actual root tip growth
            else
            {
                if (length > interval)
                {
                    // If length is sufficient, make a new branch.
                    // MSA 11.06.22 Is it safe to use the possibly-old "box" value here?
                    // It should be, as the code was like this pre-barrier modelling, and
                    // even in those days a root segment would often start in "box" but have its endpoint in another.
                    new_branch = MakeBranch(&currentEndPoint, length, 0, 0, 0, 0, box, branch_order, wrap_direction, axis_type);

                    if (!root_tip->roottip_midpoints.empty())
                    {
                        // If any deflection happened - that is, if there are any midpoints -
                        // they belong to this Branch.
                        new_branch->m_midpoints.assign(root_tip->roottip_midpoints.begin(), root_tip->roottip_midpoints.end());
                        root_tip->roottip_midpoints.clear();
                    }

                    root_tip->roottip_length = 0;
                }

                UpdateExtremes(&currentEndPoint);
                root_tip->roottip_position = currentEndPoint;

                std::queue<double> comparo = actualSubsegmentLengths;
                std::queue<size_t> comparo2 = containingVOIndices;

                // Now, adjust the root length for each subsegment of this root segment.

                const DoubleCoordinate* dc1 = &startPoint;
                const DoubleCoordinate* dc2 = __nullptr;
                std::list<DoubleCoordinate>::const_iterator iter;
                bool usingNBMP = (new_branch != __nullptr) && (!new_branch->m_midpoints.empty());
                bool usingRTMP = (new_branch == __nullptr) && (!root_tip->roottip_midpoints.empty());

                if (usingNBMP)
                {
                    iter = new_branch->m_midpoints.begin();
                    dc2 = &(*iter);
                    ++iter;
                }
                else if (usingRTMP)
                {
                    iter = root_tip->roottip_midpoints.begin();
                    dc2 = &(*iter);
                    ++iter;
                }
                else
                {
                    dc2 = &currentEndPoint;
                }

                while (dc2 != __nullptr)
                {
                    WrapDirection wrapDirection;
                    BoxCoordinate thisBoxCoord;
                    // MSA 11.06.29 We now find the containing box of a point slightly further along the DC1-DC2 line segment,
                    // to avoid edge cases where DC1 is on the boundary of two boxes, and is found by FindBoxByDoubleCoordinate()
                    // to be in the box that we don't want.
                    const double incr = Utility::CSMin(2 * NEAR_AS_INTERSECTING, DistanceSquared3D(dc1, dc2));
                    DoubleCoordinate temp = *dc1;
                    temp.x += incr * (dc2->x - dc1->x);
                    temp.y += incr * (dc2->y - dc1->y);
                    temp.z += incr * (dc2->z - dc1->z);
                    const BoxIndex thisBoxIndex = m_plant_coordinator.FindBoxByDoubleCoordinate(&temp, &wrapDirection, &thisBoxCoord);
                    const size_t idx = containingVOIndices.front();
                    AdjustRootLength(actualSubsegmentLengths.front(), dc1, &thisBoxCoord, thisBoxIndex, idx, dc2, branch_order, wrapDirection);
                    dc1 = dc2;
                    dc2 = __nullptr; // Preset to the no-more-subsegments case
                    if (usingNBMP)
                    {
                        if (iter != (new_branch->m_midpoints.end()))
                        {
                            dc2 = &(*iter);
                            ++iter;
                        }
                        else
                        {
                            // Only want to grow the last subsegment if its length is >0
                            if (remainingGrowthIncrement > 0) dc2 = &currentEndPoint;
                            usingNBMP = false; // Finished using new Branch MidPoints.
                        }
                    }
                    else if (usingRTMP)
                    {
                        if (iter != (root_tip->roottip_midpoints.end()))
                        {
                            dc2 = &(*iter);
                            ++iter;
                        }
                        else
                        {
                            // Only want to grow the last subsegment if its length is >0
                            if (remainingGrowthIncrement > 0)
                            {
                                dc2 = &currentEndPoint;
                            }
                            else
                            {
                                // MSA 11.06.28 The midpoint is no longer necessary, as the root tip length AFTER the midpoint is zero.
                                // We have to erase it now, so it won't be picked up in subsequent calls to this function.
                                root_tip->roottip_midpoints.clear();
                            }
                            usingRTMP = false; // Finished using Root Tip MidPoints.
                        }
                    }

                    containingVOIndices.pop();
                    actualSubsegmentLengths.pop();
                }

                RmAssert(((currentEndPoint.x < 1000) && (currentEndPoint.y < 1000) && (currentEndPoint.z < 1000)), "RootTip position gone berserk");

                // Drawing uses the original startPoint, as the branch/rootTip handles drawing of all its own midpoints.
                if (new_branch != __nullptr)
                {
                    DrawBranch(startPoint, new_branch, root_tip);
                }
                else
                {
                    // There's no new Branch, just an elongated RootTip. Draw it.
                    DrawRootTip(startPoint, root_tip);
                }
            } // if(!intersectionHasOccurred)
        } // while(intersectionHasOccurred)

        return new_branch;
    }

    DoubleCoordinate Plant::GetGrowthDestination(const DoubleCoordinate& startPoint, const Vec3d& heading, const double& growthRate)
    {
        DoubleCoordinate dest;
        dest.x = startPoint.x + growthRate * heading.x;
        dest.y = startPoint.y + growthRate * heading.y;
        dest.z = startPoint.z + growthRate * heading.z;
        return dest;
    }

    void Plant::UpdateExtremes(const DoubleCoordinate* possible_new_extreme)
    {
        if (possible_new_extreme->x < m_plant_root_extremes[X][negative])
        {
            m_plant_root_extremes[X][negative] = possible_new_extreme->x;
            m_plant_is_more_extreme = true;
        }

        if (possible_new_extreme->x > m_plant_root_extremes[X][positive])
        {
            m_plant_root_extremes[X][positive] = possible_new_extreme->x;
            m_plant_is_more_extreme = true;
        }

        if (possible_new_extreme->y < m_plant_root_extremes[Y][negative])
        {
            m_plant_root_extremes[Y][negative] = possible_new_extreme->y;
            m_plant_is_more_extreme = true;
        }

        if (possible_new_extreme->y > m_plant_root_extremes[Y][positive])
        {
            m_plant_root_extremes[Y][positive] = possible_new_extreme->y;
            m_plant_is_more_extreme = true;
        }

        if (possible_new_extreme->z < m_plant_root_extremes[Z][negative])
        {
            m_plant_root_extremes[Z][negative] = possible_new_extreme->z;
            m_plant_is_more_extreme = true;
        }

        if (possible_new_extreme->z > m_plant_root_extremes[Z][positive])
        {
            m_plant_root_extremes[Z][positive] = possible_new_extreme->z;
            m_plant_is_more_extreme = true;
        }
    }

    Branch* Plant::BranchEdit
    (Branch* this_branch,
        Branch* last_branch,
        RootTip* parent,
        long int branch_order,
        double time_step,
        double branch_lag_time,
        AxisType axis_type)
    {
        RootMapLogDebug_PlantLowLevel("BranchEdit branch id:" << this_branch->m_id << ", parent id:" << parent->m_id);
        Branch* return_branch = this_branch;
        BranchEditData bed(this_branch, last_branch, parent, branch_order, time_step, branch_lag_time);

        // Initialise the segment heading correctly
        bed.segment_heading = Vec3d(this_branch->m_position.x - last_branch->m_position.x, this_branch->m_position.y - last_branch->m_position.y, this_branch->m_position.z - last_branch->m_position.z);
        bed.segment_heading.Normalize(true);

        // Find the box in which the branch node lives
        bed.box = m_plant_coordinator.FindBoxByDoubleCoordinate(&(this_branch->m_position), &(bed.wrap));

        const size_t idx = m_plant_coordinator.GetVolumeObjectCoordinator()->GetContainingVolumeObjectIndex(this_branch->m_position);

        if (axis_type == Seminal)
        {
            bed.branch_interval = saSeminalFinalBranchInterval->GetValue(bed.box, branch_order, idx);
        }
        else
        {
            bed.branch_interval = saNodalFinalBranchInterval->GetValue(bed.box, branch_order, idx);
        }

        while (this_branch->m_length >= (2 * bed.branch_interval))
        {
            if (last_branch->m_parentBranch == this_branch)
            {
                WhereAndWhen(this_branch, last_branch, parent, branch_lag_time, time_step, &bed);
                last_branch->m_parentBranch = InsertBranch(this_branch, branch_order, time_step, &bed, axis_type);
                last_branch = last_branch->m_parentBranch;
            }
            else
            {
                WhereAndWhen(this_branch, last_branch, parent, branch_lag_time, time_step, &bed);
                last_branch->m_childBranch = InsertBranch(this_branch, branch_order, time_step, &bed, axis_type);
                last_branch = last_branch->m_childBranch;
            }
        }

        if (this_branch->m_length >= bed.branch_interval)
        {
            WhereAndWhen(this_branch, last_branch, parent, branch_lag_time, time_step, &bed);
            MoveBranch(this_branch, parent, branch_order, time_step, &bed, axis_type);
        }
        else if (last_branch->m_parentBranch == this_branch)
        {
            last_branch->m_parentBranch = RemoveBranch(this_branch, parent, branch_order);
            this_branch = last_branch;
            return_branch = this_branch;
        }
        else
        {
            last_branch->m_childBranch = RemoveBranch(this_branch, parent, branch_order);
            this_branch = last_branch;
            return_branch = this_branch;
        }

        return (return_branch);
    }

    void Plant::WhereAndWhen
    (Branch* this_branch,
        Branch* last_branch,
        RootTip* parent,
        double branch_lag_time,
        double time_step,
        BranchEditData* edit_data)
    {
        RootMapLogDebug_PlantLowLevel("WhereAndWhen branch id:" << this_branch->m_id << ", parent id:" << parent->m_id);
        double length_fraction = edit_data->branch_interval / this_branch->m_length;

        if (last_branch->m_child == parent)
            edit_data->branch_age = ((1 - length_fraction) * (parent->roottip_age - this_branch->m_age) + this_branch->m_age);
        else
            edit_data->branch_age = ((1 - length_fraction) * (last_branch->m_age - this_branch->m_age) + this_branch->m_age);

        edit_data->time_step_fraction = (edit_data->branch_age - branch_lag_time) / time_step;

        edit_data->next_position.x = last_branch->m_position.x + length_fraction * (this_branch->m_position.x - last_branch->m_position.x);
        edit_data->next_position.y = last_branch->m_position.y + length_fraction * (this_branch->m_position.y - last_branch->m_position.y);
        edit_data->next_position.z = last_branch->m_position.z + length_fraction * (this_branch->m_position.z - last_branch->m_position.z);
    }

    Branch* Plant::InsertBranch
    (Branch* at_branch,
        long branch_order,
        double time_step,
        BranchEditData* bed,
        AxisType axis_type)
    {
        RootMapLogDebug_PlantLowLevel("InsertBranch at branch id:" << at_branch->m_id);

        // MSA 11.02.02 There is a problem here. The BranchEditData's "box" is selected based on the position of "this_branch".
        // bed->next_position is a DIFFERENT POINT, and is thus often NOT inside bed->box.

        const BoxIndex box = m_plant_coordinator.FindBoxByDoubleCoordinate(&(bed->next_position));

        Branch* b = MakeBranch(&(bed->next_position), bed->branch_interval, bed->branch_age, 0, 0, at_branch, box, branch_order, bed->wrap, axis_type);
        b->m_child = MakeRootTip(&(b->m_position), &(bed->segment_heading), bed->time_step_fraction * time_step, box, branch_order + 1, bed->wrap, axis_type);
        b->m_childBranch = GrowRootTip(b->m_child, branch_order + 1, bed->time_step_fraction, time_step, axis_type);
        at_branch->m_length -= bed->branch_interval;

        return (b);
    }

    void Plant::MoveBranch
    (Branch* the_branch,
        RootTip* the_parent,
        long branch_order,
        double time_step,
        BranchEditData* bed,
        AxisType axis_type)
    {
        RootMapLogDebug_PlantLowLevel("MoveBranch id:" << the_branch->m_id);
        Vec3d heading(bed->segment_heading);

        DoubleCoordinate good_position(the_branch->m_position.x, the_branch->m_position.y, the_branch->m_position.z);

        // So, we essentially move this Branch to a new position and create a new RootTip (and very probably a new Branch too) at the old position.
        the_branch->m_position = bed->next_position;
        the_branch->m_child = MakeRootTip(&good_position, &heading, (double)(bed->time_step_fraction * time_step), bed->box, (branch_order + 1), bed->wrap, axis_type);
        the_branch->m_childBranch = GrowRootTip(the_branch->m_child, branch_order + 1, bed->time_step_fraction, time_step, axis_type);

        if (the_branch->m_parentBranch != 0)
            the_branch->m_parentBranch->m_length += the_branch->m_length - bed->branch_interval;
        else
            the_parent->roottip_length += the_branch->m_length - bed->branch_interval;

        the_branch->m_length = bed->branch_interval;
        the_branch->m_age = bed->branch_age;
    }

    Branch* Plant::RemoveBranch
    (Branch* the_branch,
        RootTip* the_parent,
        long int branch_order)
    {
        RootMapLogDebug_PlantLowLevel("RemoveBranch id:" << the_branch->m_id);
        if (the_branch->m_parentBranch != 0)
        {
            the_branch->m_parentBranch->m_length += the_branch->m_length;
        }
        else
        {
            the_parent->roottip_length += the_branch->m_length;
        }

        Branch* return_branch = the_branch->m_parentBranch;

        const size_t idx = m_plant_coordinator.GetVolumeObjectCoordinator()->GetContainingVolumeObjectIndex(the_branch->m_position);

        DecrementBranchCount(&(the_branch->m_position), idx, branch_order);

        TCLForgetObject(the_branch);

        return (return_branch);
    }

    void Plant::GrowBranch
    (Branch* this_branch,
        Branch* last_branch,
        RootTip* parent,
        long int branch_order,
        double time_step,
        AxisType axis_type)
    {
        RootMapLogDebug_PlantLowLevel("GrowBranch id:" << this_branch->m_id);
        Branch* corner_branch = last_branch;

        do
        {
            const BoxIndex box = m_plant_coordinator.FindBoxByDoubleCoordinate(&(this_branch->m_position));

            const size_t idx = m_plant_coordinator.GetVolumeObjectCoordinator()->GetContainingVolumeObjectIndex(this_branch->m_position);

            double branch_lag_time = 0; // Initialising to silence compiler warning
            if (axis_type == Seminal)
            {
                branch_lag_time = saSeminalBranchLagTime->GetValue(box, branch_order, idx);
                if (branch_order > 3)
                {
                    branch_lag_time = saSeminalBranchLagTime->GetValue(box, 3, idx);
                }
            }
            else if (axis_type == Nodal)
            {
                branch_lag_time = saNodalBranchLagTime->GetValue(box, branch_order, idx);
                if (branch_order > 3)
                {
                    branch_lag_time = saNodalBranchLagTime->GetValue(box, 3, idx);
                }
            }
            else
            {
                RmAssert(false, "Branch lag time could not be set - unknown AxisType encountered");
            }

            double growth_time_adjusted = m_plant_coordinator.TempAdjustRate(time_step, box, saTemperatureOfZeroGrowth->GetValue());
            this_branch->m_age += growth_time_adjusted;

            if (this_branch->m_childBranch != 0)
            {
                //            if (this_branch->m_child->roottip_age <= m_plant_coordinator.GetTipGrowthDuration(branch_order+1, box))
                if (this_branch->m_child->roottip_age <= saTipGrowthDuration->GetValue(box, branch_order + 1, idx))
                {
                    this_branch->m_child->roottip_age += growth_time_adjusted;
                    GrowBranch(this_branch->m_childBranch, this_branch, this_branch->m_child, branch_order + 1, time_step, axis_type);
                }
            }
            else if (this_branch->m_child != 0)
            {
                this_branch->m_child->roottip_age += growth_time_adjusted;
                this_branch->m_childBranch = GrowRootTip(this_branch->m_child, branch_order + 1, 1, time_step, axis_type);
            }
            else if (this_branch->m_age >= branch_lag_time)
            {
                this_branch = BranchEdit(this_branch, last_branch, parent, branch_order, time_step, branch_lag_time, axis_type);
            }

            last_branch = this_branch;
            if (this_branch == corner_branch)
            {
                this_branch = this_branch->m_childBranch;
            }
            else
            {
                this_branch = this_branch->m_parentBranch;
            }
        } while (this_branch != 0);

        last_branch->m_parentBranch = GrowRootTip(parent, branch_order, 1, time_step, axis_type);
    }

    void Plant::GrowAxis(Axis* axis, double time_step, double adjusted_time)
    {
        RootMapLogDebug_PlantLowLevel("GrowAxis id:" << axis->m_id);
        //<< ", age:" << axis->m_age
        //<< ", orientation:" << ToString(axis_start_orientation).c_str()
        //<< ", position:" << ToString(axis->m_position).c_str();
        axis->m_child->roottip_age += adjusted_time;
        if (axis->m_childBranch == 0)
            axis->m_childBranch = GrowRootTip(axis->m_child, 0, 1, time_step, axis->GetType());
        else
            GrowBranch(axis->m_childBranch, axis, axis->m_child, 0, time_step, axis->GetType());
    }

    void Plant::StartAxis(Axis* axis, double time_step, double adjusted_time)
    {
        double time_step_fraction = (axis->m_age - saGerminationLag->GetValue() - axis->GetStartLag() - m_plant_seeding_time) / adjusted_time;
        Vec3d axis_start_orientation;
        axis->GetStartOrientation(&axis_start_orientation);
        RootMapLogDebug_PlantLowLevel("StartAxis id:" << axis->m_id);
        //<< ", age:" << axis->m_age
        //<< ", orientation:" << ToString(axis_start_orientation).c_str()
        //<< ", position:" << ToString(axis->m_position).c_str();


        axis->m_age -= (saGerminationLag->GetValue() + m_plant_seeding_time + axis->GetStartLag());
        axis->m_child = MakeRootTip(&(axis->m_position), &axis_start_orientation, time_step_fraction * time_step, 0, 0, wraporderNone, axis->GetType());
        axis->m_childBranch = GrowRootTip(axis->m_child, 0, time_step_fraction, time_step, axis->GetType());

        RootMapLogDebug_PlantLowLevel("Started Plant Axis id:" << axis->m_id);
        //<< ", Start Orientation:" << ToString(axis_start_orientation).c_str()
        //<< ", Position:" << ToString(axis->m_position).c_str();
    }

    void Plant::NextTimeStep(const ProcessTime_t& time_step)
    {
        Axis* this_axis = m_plant_first_axis;
        Axis* last_axis = 0;
        DoubleCoordinate start_orientation;// = {0.0, 0.0, 0.0};  Default constructor handles that, now
        BoxIndex box = 0;
        double time_increment = 0;

        while (this_axis != 0)
        {
            try
            {
                if (this_axis->GetType() == Seminal)
                {
                    box = m_plant_coordinator.FindBoxByDoubleCoordinate(&m_plant_seed_location);
                }
                else
                {
                    box = m_plant_coordinator.FindBoxByDoubleCoordinate(&m_plant_origin);
                }
            }
            catch (const std::exception& stdex)
            {
                LOG_ALERT << "Caught std::exception calling PlantCoordinator::FindBoxByDoubleCoordinate() : " << stdex.what();
                RmAssert(false, "Caught std::exception calling PlantCoordinator::FindBoxByDoubleCoordinate()");
            }
            catch (...)
            {
                RmAssert(false, "Caught non-standard exception calling PlantCoordinator::FindBoxByDoubleCoordinate()");
            }

            time_increment = m_plant_coordinator.TempAdjustRate(time_step, box, saTemperatureOfZeroGrowth->GetValue());
            this_axis->m_age += time_increment;

            //        if ((this_axis->m_child != 0) && (this_axis->m_age <= m_plant_coordinator.GetTipGrowthDuration(0, box)))

            const size_t idx = m_plant_coordinator.GetVolumeObjectCoordinator()->GetContainingVolumeObjectIndex(this_axis->m_position);

            if ((this_axis->m_child != 0) && (this_axis->m_age <= saTipGrowthDuration->GetValue(box, 0, idx)))
            {
                try
                {
                    GrowAxis(this_axis, time_step, time_increment);
                }
                catch (const std::exception& stdex)
                {
                    LOG_ALERT << "Caught std::exception calling Plant::GrowAxis() : " << stdex.what();
                    RmAssert(false, "Caught std::exception calling Plant::GrowAxis()");
                }
                //catch (...)
                //{
                //    RmAssert(false, "Caught non-standard exception calling Plant::GrowAxis()");
                //}
            }
            else if ((this_axis->m_child == 0) && (this_axis->m_age >= this_axis->GetStartLag() + saGerminationLag->GetValue() + m_plant_seeding_time))
            {
                try
                {
                    StartAxis(this_axis, time_step, time_increment);
                }
                catch (const std::exception& stdex)
                {
                    LOG_ALERT << "Caught std::exception calling Plant::StartAxis() : " << stdex.what();
                    RmAssert(false, "Caught std::exception calling Plant::StartAxis()");
                }
                catch (...)
                {
                    RmAssert(false, "Caught non-standard exception calling Plant::StartAxis()");
                }
            }

            last_axis = this_axis;
            this_axis = (Axis*)(this_axis->m_parentBranch);
        }
    }

    void Plant::DrawScoreboard(const DoubleRect& area, Scoreboard* scoreboard)
    {
        if (scoreboard->GetScoreboardStratum().isBelowZero())
        {
            DrawRoots(area, scoreboard);
        }
    }

    // The buffer will already be set up as the active drawing port
    void Plant::DrawRoots(const DoubleRect& /* area */, Scoreboard* /* scoreboard */)
    {
        if (m_plant_first_axis == 0) return;

        Axis* this_axis = m_plant_first_axis;
        do
        {
            if (this_axis->m_childBranch != 0)
            {
                DrawBranch(this_axis->m_position, this_axis->m_childBranch, this_axis->m_child);
            }
            else if (this_axis->m_child != 0)
            {
                DrawRootTip(this_axis->m_position, this_axis->m_child);
            }
            this_axis = (Axis*)(this_axis->m_parentBranch);
        } while (this_axis != 0);
    }

    void Plant::DrawBranch(const DoubleCoordinate& starting_point, Branch* this_branch, RootTip* parent)
    {
        DoubleCoordinate coord = starting_point;
        BEGIN_DRAWING
            do
            {
                std::list<DoubleCoordinate>& midpoints = this_branch->m_midpoints;

                DoubleCoordinate& previousPoint = coord; // MSA heap.
                for (std::list<DoubleCoordinate>::const_iterator iter = midpoints.begin(); iter != midpoints.end(); ++iter)
                {
                    // Draw all but the last segment
                    drawing.DrawRootSegment(previousPoint, *iter,
                        this_branch->m_branch_order,
                        this_branch->m_radius,
                        this_branch->m_id);
                    previousPoint = (*iter);
                }
                // Draw the last segment, from the final midpoint (or the start point) to roottip_position
                drawing.DrawRootSegment(previousPoint, this_branch->m_position,
                    this_branch->m_branch_order,
                    this_branch->m_radius,
                    this_branch->m_id);
                coord = this_branch->m_position;

                /* Oh, do let the children draw, Dear.
                If we have a child (ie. a next-order branch), let it draw next */
                if (this_branch->m_childBranch != 0)
                {
                    DrawBranch(coord, this_branch->m_childBranch, this_branch->m_child);
                }
                // otherwise, if we only have a child (next-order rootTip) then just draw that
                else if (this_branch->m_child != 0)
                {
                    DrawRootTip(coord, this_branch->m_child);
                }

                // And when they're all quite finished drawing, we need to retrieve the pen
                // then head off to the next branch in our root-order's line
                this_branch = this_branch->m_parentBranch;
            } while (this_branch != 0); // we get to the end of the branches on this root-order

            // Finally we must be at the end - draw our parent.}
            DrawRootTip(coord, parent);

            END_DRAWING
    }

    void Plant::DrawRootTip(const DoubleCoordinate& origin, RootTip* rootTip)
    {
        if (rootTip->roottip_length <= 0) return; // This is faster than comparing the origin and roottip_position coordinates.

        std::list<DoubleCoordinate>& midpoints = rootTip->roottip_midpoints;

        /**
        *  MSA All root segment drawing instructions output from Plant.cpp now use
        *  the DrawRootSegment() methods. This is to allow buffers to interpret the order
        *  to draw a root segment differently.
        *  ProcessDrawingBuffer will pass on instructions to draw a line, and
        *  ProcessVertexBuffer will pass on instructions to draw either a line
        *  or a cylinder, depending on configuration settings.
        */
        BEGIN_DRAWING

            const DoubleCoordinate* previousPoint = &origin;
        for (std::list<DoubleCoordinate>::const_iterator iter = midpoints.begin(); iter != midpoints.end(); ++iter)
        {
            // Draw all but the last segment
            drawing.DrawRootSegment(*previousPoint, *iter,
                rootTip->roottip_order, rootTip->roottip_radius, rootTip->m_id);
            previousPoint = &(*iter);
        }
        // Draw the last segment, from the final midpoint (or the start point) to roottip_position
        drawing.DrawRootSegment(*previousPoint, rootTip->roottip_position,
            rootTip->roottip_order, rootTip->roottip_radius, rootTip->m_id);

        END_DRAWING
    }

    long int Plant::DoActivateRaytracerOutput()
    {
        m_raytracerOutputActive = true;
        return kNoError;
    }

    long int Plant::DoRaytracerOutput(RaytracerData* raytracerData)
    {
        if (0 == raytracerData || 0 == m_plant_first_axis || m_new_segment_map.empty()) return kNoError;

        // Sanity check.
        RmAssert(m_raytracerOutputActive, "Error: DoActivateRaytracerOutput() has not been called");

        // MSA 09.12.07 There is probably a better way to do this.
        const long pCount = m_plant_coordinator.GetNumPlants();
        long pNum = 0;
        for (; pNum < pCount; ++pNum)
        {
            if (this == m_plant_coordinator.GetPlant(pNum)) { break; } // We've identified the number of this Plant.
            if (pNum >= (pCount - 1))
            {
                RmAssert(false, "PlantCoordinator cannot find this Plant pointer (???)");
                return kInvalidMessage;
            }
        }

        Axis* this_axis = m_plant_first_axis;
        do
        {
            if (this_axis->m_childBranch != 0)
            {
                RaytracerCacheBranch(pNum, this_axis->m_position, this_axis->m_childBranch, this_axis->m_child, raytracerData);
            }
            else if (this_axis->m_child != 0 && m_new_segment_map[this_axis->m_child->m_id])
            {
                // Buffer a raytracer object to represent this root tip
                RaytracerCacheRootTip(pNum, this_axis->m_position, this_axis->m_child, raytracerData);
            }

            this_axis = (Axis*)(this_axis->m_parentBranch);
        } while (this_axis != 0);

        // All extant root segments have now been cached in raytracerData.
        // Clear the decks for new root segments.
        m_new_segment_map.clear();

        return kNoError;
    }

    void Plant::RaytracerCacheRootTip(const long& pNum, const DoubleCoordinate& origin, RootTip* rootTip, RaytracerData* raytracerData)
    {
        if (rootTip->roottip_length <= 0) return; // This is faster than comparing the origin and roottip_position coordinates.

        std::list<DoubleCoordinate>& midpoints = rootTip->roottip_midpoints;

        const DoubleCoordinate* previousPoint = &origin;
        for (std::list<DoubleCoordinate>::const_iterator iter = midpoints.begin(); iter != midpoints.end(); ++iter)
        {
            // Cache all but the last segment
            raytracerData->AddRootSegment(pNum, rootTip->roottip_order, rootTip->roottip_radius, *previousPoint, *iter);
            previousPoint = &(*iter);
        }
        // Cache the last segment, from the final midpoint (or the start point) to roottip_position
        raytracerData->AddRootSegment(pNum, rootTip->roottip_order, rootTip->roottip_radius, *previousPoint, rootTip->roottip_position);
    }

    void Plant::RaytracerCacheBranch
    (const long& pNum,
        const DoubleCoordinate& starting_point,
        Branch* this_branch,
        RootTip* parent,
        RaytracerData* raytracerData)
    {
        // Copy construct a working coordinate
        DoubleCoordinate coord = starting_point;

        do
        {
            if (m_new_segment_map[this_branch->m_id])
            {
                // Buffer a raytracer object to represent this root branch
                if (this_branch->m_length > 0) // This is faster than comparing the origin and roottip_position coordinates.
                {
                    std::list<DoubleCoordinate>& midpoints = this_branch->m_midpoints;

                    const DoubleCoordinate* previousPoint = &coord;
                    for (std::list<DoubleCoordinate>::const_iterator iter = midpoints.begin(); iter != midpoints.end(); ++iter)
                    {
                        // Cache all but the last segment
                        raytracerData->AddRootSegment(pNum, this_branch->m_branch_order, this_branch->m_radius, *previousPoint, *iter);
                        previousPoint = &(*iter);
                    }
                    // Cache the last segment, from the final midpoint (or the start point) to m_position
                    raytracerData->AddRootSegment(pNum, this_branch->m_branch_order, this_branch->m_radius, *previousPoint, this_branch->m_position);
                }
            }
            // Important: must always update the working coord (MSA bug fix)
            coord = this_branch->m_position;

            // If we have a child (ie. a next-order branch), let it cache next
            if (this_branch->m_childBranch != 0)
            {
                RaytracerCacheBranch(pNum, coord, this_branch->m_childBranch, this_branch->m_child, raytracerData);
            }
            // otherwise, if we only have a child (next-order rootTip) then just cache that
            else if (this_branch->m_child != 0 && m_new_segment_map[this_branch->m_child->m_id])
            {
                // Buffer a raytracer object to represent this root tip
                RaytracerCacheRootTip(pNum, coord, this_branch->m_child, raytracerData);
            }

            // head off to the next branch in our root-order's line
            this_branch = this_branch->m_parentBranch;
        } while (this_branch != 0); // we get to the end of the branches on this root-order

        if (m_new_segment_map[parent->m_id])
        {
            // Finally, cache the parent.
            RaytracerCacheRootTip(pNum, coord, parent, raytracerData);
        }
    }


    void Plant::PrepareSharedAttributes(ProcessActionDescriptor* action)
    {
        SharedAttributeSearchHelper helper(action->GetSharedAttributeManager(), m_plant_coordinator.GetSharedAttributeOwner());

        //
        std::string plant_name = GetProcessName();

        //
        // Load up our expected SharedAttributes
        //
        // Per plant and per root order
        saSeminalBranchLagTime = helper.SearchForAttribute("Seminal Branch Lag Time", "Plant,RootOrder", plant_name.c_str());
        saNodalBranchLagTime = helper.SearchForAttribute("Nodal Branch Lag Time", "Plant,RootOrder", plant_name.c_str());
        saSeminalDeflectionIndex = helper.SearchForAttribute("Seminal Deflection Index", "Plant,RootOrder", plant_name.c_str());
        saNodalDeflectionIndex = helper.SearchForAttribute("Nodal Deflection Index", "Plant,RootOrder", plant_name.c_str());
        saSeminalFinalBranchInterval = helper.SearchForAttribute("Seminal Final Branch Interval", "Plant,RootOrder", plant_name.c_str());
        saNodalFinalBranchInterval = helper.SearchForAttribute("Nodal Final Branch Interval", "Plant,RootOrder", plant_name.c_str());
        saSeminalGeotropismIndex = helper.SearchForAttribute("Seminal Geotropism Index", "Plant,RootOrder", plant_name.c_str());
        saNodalGeotropismIndex = helper.SearchForAttribute("Nodal Geotropism Index", "Plant,RootOrder", plant_name.c_str());
        saInitialBranchAngle = helper.SearchForAttribute("Initial Branch Angle", "Plant,RootOrder", plant_name.c_str());
        saInitialBranchInterval = helper.SearchForAttribute("Initial Branch Interval", "Plant,RootOrder", plant_name.c_str());
        saRootConductance = helper.SearchForAttribute("Root Conductance", "Plant,RootOrder", plant_name.c_str());
        saTipGrowthDuration = helper.SearchForAttribute("Tip Growth Duration", "Plant,RootOrder", plant_name.c_str());
        saPlasticityFactorP = helper.SearchForAttribute("Root P Plasticity Factor", "Plant,RootOrder", plant_name.c_str());
        saPlasticityFactorN = helper.SearchForAttribute("Root N Plasticity Factor", "Plant,RootOrder", plant_name.c_str());
        saDownRegulationFactor = helper.SearchForAttribute("Down Regulation Factor", "Plant,RootOrder", plant_name.c_str());
        saSeminalUnitGwthRte = helper.SearchForAttribute("Seminal Unit Growth Rate", "Plant,RootOrder", plant_name.c_str());
        saNodalUnitGwthRte = helper.SearchForAttribute("Nodal Unit Growth Rate", "Plant,RootOrder", plant_name.c_str());
        saGrowthRateMax = helper.SearchForAttribute("Growth Rate Max", "Plant,RootOrder", plant_name.c_str());
        saSeminalGrowthRate = helper.SearchForAttribute("Seminal Growth Rate", "Plant,RootOrder", plant_name.c_str());
        saNodalGrowthRate = helper.SearchForAttribute("Nodal Growth Rate", "Plant,RootOrder", plant_name.c_str());

        // search for root length, per Plant+RootOrder
        saRootLengthPerPlantPerRootOrder = helper.SearchForAttribute("Root Length Wrap None", "Plant,RootOrder", plant_name.c_str());

        saRootRadiusPerPlantPerRootOrder = helper.SearchForAttribute("Root Radius", "Plant,RootOrder", plant_name.c_str());

        // per-PlantType
        saInitialSeminalDeflection = helper.SearchForAttribute("Initial Seminal Deflection", "PlantType");
        saTemperatureOfZeroGrowth = helper.SearchForAttribute("Temperature of Zero Growth", "PlantType");
        saFirstSeminalProbability = helper.SearchForAttribute("First Seminal Probability", "PlantType");

        //
        // per-Plant attributes (no RootOrder). Note that we want only the attribute
        // for THIS plant
        saSeedingTime = helper.SearchForAttribute("Seeding Time", "Plant", plant_name.c_str());
        saSeedingTime->SetValue(m_plant_seeding_time); //Time from zero (start of simulation) in hours when the seeds are sown

        saGerminationLag = helper.SearchForAttribute("Germination Lag", "Plant", plant_name.c_str());
        saLeafEmergence = helper.SearchForAttribute("Leaf Emergence", "Plant", plant_name.c_str());
        saCotyLife = helper.SearchForAttribute("Mean Coty Life", "Plant", plant_name.c_str());
        saWaterUptake = helper.SearchForAttribute("Local Plant Water Uptake", "Plant", plant_name.c_str());
        saPhosphorusUptake = helper.SearchForAttribute("Phosphorus Uptake", "Plant", plant_name.c_str());
        saNitrateUptake = helper.SearchForAttribute("Plant Nitrate Uptake", "Plant", plant_name.c_str());
        saTotalRootLength = helper.SearchForAttribute("Root Length Wrap None", "Plant", plant_name.c_str());
        saPreviousTotalRootLength = helper.SearchForAttribute("Previous Total Root Length", "Plant", plant_name.c_str());
        saPlantIsLegume = helper.SearchForAttribute("Plant Is Legume", "Plant", plant_name.c_str());
        saOrganicExudate = helper.SearchForAttribute("Organic Exudate", "Plant", plant_name.c_str());
        saImaxDeclineCurve = helper.SearchForAttribute("Imax Decline Curve", "Plant", plant_name.c_str());
        saRemNResUnits = helper.SearchForAttribute("Remaining N Resource Units", "Plant", plant_name.c_str());
        saRemPResUnits = helper.SearchForAttribute("Remaining P Resource Units", "Plant", plant_name.c_str());
        saPlantTargetResRatio = helper.SearchForAttribute("Plant Target Resource Ratio", "Plant", plant_name.c_str());
        saPlantActualResRatio = helper.SearchForAttribute("Plant Actual Resource Ratio", "Plant", plant_name.c_str());
        saNRegulationFactor = helper.SearchForAttribute("N Regulation Factor", "Plant", plant_name.c_str());
        saPRegulationFactor = helper.SearchForAttribute("P Regulation Factor", "Plant", plant_name.c_str());


        //These are per plant at the moment, but will probably be changed to per plant and per root order soon
        saPConcMin = helper.SearchForAttribute("P Conc Min", "Plant", plant_name.c_str());
        saNFluxMax = helper.SearchForAttribute("N Flux Max", "Plant", plant_name.c_str());
        saPFluxMax = helper.SearchForAttribute("P Flux Max", "Plant", plant_name.c_str());
        saNUptakeKineticCons = helper.SearchForAttribute("N Uptake Kinetic Const", "Plant", plant_name.c_str());
        saPUptakeKineticCons = helper.SearchForAttribute("P Uptake Kinetic Const", "Plant", plant_name.c_str());
        saNAbsorptionPower = helper.SearchForAttribute("N Absorption Power", "Plant", plant_name.c_str());
        saPAbsorptionPower = helper.SearchForAttribute("P Absorption Power", "Plant", plant_name.c_str());
        saCumulNFix = helper.SearchForAttribute("Cumul Plant N Fixation", "Plant", plant_name.c_str());
    }

    bool Plant::DoesDrawing() const
    {
        return (true);
    }

    bool Plant::DoesRaytracerOutput() const
    {
        return (true);
    }

    DoubleCoordinate Plant::GetOrigin() const
    {
        return m_plant_origin;
    }

    double Plant::GetRadius(const BoxIndex& box_index, const long& branch_order, const size_t& volumeObjectIndex) const
    {
        return saRootRadiusPerPlantPerRootOrder->GetValue(box_index, branch_order, volumeObjectIndex);
    }

    BranchEditData::BranchEditData()
        : this_branch(0)
        , last_branch(0)
        , parent(0)
        , branch_order(0)
        , time_step(0)
        , branch_lag_time(0)
        , segment_heading(0.0, 0.0, 0.0)
        //, next_position( 0.0, 0.0, 0.0 )
        , branch_interval(0)
        , branch_age(0)
        , time_step_fraction(0)
        , return_branch(0)
        , box(0)
        , wrap(wraporderNone)
    {
    }

    BranchEditData::BranchEditData(
        Branch* tb,
        Branch* lb,
        RootTip* pa,
        long int bo,
        double ts,
        double blt)
        : this_branch(tb)
        , last_branch(lb)
        , parent(pa)
        , branch_order(bo)
        , time_step(ts)
        , branch_lag_time(blt)
        , segment_heading(0, 0, 0)
        //, next_position( 0, 0, 0 )
        , branch_interval(0)
        , branch_age(0)
        , time_step_fraction(0)
        , return_branch(0)
        , box(0)
        , wrap(wraporderNone)
    {
    }
} /* namespace rootmap */
