#include "BoostPrecomp.h"

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

//#include "simulation/process/common/ProcessDrawingBuffer.h"
#include "simulation/data_access/interface/PlantDAI.h"
#include "simulation/data_access/interface/ProcessDAI.h"
#include "simulation/common/RaytracerData.h"

#include "core/utility/Utility.h"
#include "core/log/LogHelper.h"
#include "core/macos_compatibility/MacResourceManager.h"
// #include <stdlib.h>
#include "simulation/common/BoostPrecomp.h"


namespace rootmap
{

    RootMapLoggerDefinition(Plant);

    using Utility::CSRestrict;

#if 0
#define RootMapLogDebug_PlantLowLevel(msg) RootMapLogDebug(msg)
#else
#define RootMapLogDebug_PlantLowLevel(msg)
#endif

    class BranchEditData
    {
    public:
        // PARAMETERS PASSED INTO BranchEdit()
        Branch            * this_branch;
        Branch            * last_branch;
        RootTip           * parent;
        long int            branch_order;
        double_cs           time_step;
        double_cs           branch_lag_time;

        // BranchEdit() LOCAL VARIABLES
        DoubleCoordinate    segment_heading;
        DoubleCoordinate    next_position;
        double_cs           branch_interval;
        double_cs           branch_age;
        double_cs           time_step_fraction;
        Branch            * return_branch;
        BoxIndex            box;
        WrapDirection       wrap;

        BranchEditData();
        BranchEditData(    Branch * tb,
            Branch * lb,
            RootTip * pa,
            long int bo,
            double_cs ts,
            double_cs blt);                    
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
    Plant::Plant(PlantCoordinator & coordinator)
        : Process()
        , saBranchLagTime(0)
        , saDeflectionIndex(0)
        , saFinalBranchInterval(0)
        , saGeotropismIndex(0)
        , saGrowthRateMax(0)
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
        , saSeedSize(0)
        , saPlantIsLegume(0)
        , plantDRAInitialised(false)
        , m_soil_scoreboard_Issue15(NULL)
        , plant_type(0)
        , plant_seeding_time(0)
        , plant_first_axis(0)
        , plant_first_branch(0)
        , plant_coordinator(coordinator)
        , plant_origin(0, 0, 0)
        , plant_seed_location(0, 0, 0)
        , plant_total_branches(0)
        , plant_total_tips(0)
        , plant_total_root_length(0.0)
        , plant_total_root_length_high(0.0)
        , plant_is_more_extreme(true)
        , plant_possible_seminal_axes(0)
        , plant_possible_nodal_axes(0)
        , plantPreviousTimestamp(0)
    {
        RootMapLoggerInitialisation("rootmap.Plant");
        Plant::Defaults();
        m_maxmap = m_branch_order_map.max_size()-1;
    }

    void Plant::Initialise(const ProcessDAI & data)
    {
        Process::Initialise(data);

        try
        {
            const PlantDAI & plant_data = dynamic_cast<const PlantDAI &>(data);

            plant_type = plant_coordinator.FindPlantTypeByName(plant_data.getTypeName());
            plant_origin = plant_data.getOrigin();
            plant_seed_location = plant_data.getSeedLocation();
            plant_seeding_time = plant_data.getSeedingTime()*24;  //converting from days to hours
            plant_possible_seminal_axes = plant_data.getPossibleSeminals();
            plant_possible_nodal_axes = plant_data.getPossibleNodals();
            RootMapLogDebug("Initialised Plant " << plant_data.getTypeName());

        }
        catch (std::bad_cast & /*bad_cast_ex*/)
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

        PossibleAxis * possible_axis  = 0;
        // while there is still some list left,
        while (plant_possible_seminal_axes)
        {
            // keep a pointer to the second node
            possible_axis = plant_possible_seminal_axes->GetNext();
            // forget the head node
            delete plant_possible_seminal_axes;
            // make the head what used to be the second node
            plant_possible_seminal_axes = possible_axis;
        }
        plant_possible_seminal_axes = 0;

        // while there is still some list left,
        while (plant_possible_nodal_axes)
        {
            // keep a pointer to the second node
            possible_axis = plant_possible_nodal_axes->GetNext();
            // forget the head node
            delete plant_possible_nodal_axes;
            // make the head what used to be the second node
            plant_possible_nodal_axes = possible_axis;
        }
        plant_possible_seminal_axes = 0;

        // WARNING: DO NOT DO THIS.  Call ForgetRoots()
        //delete plant_first_axis;
        //delete plant_first_branch;
        ForgetRoots();
    }

    void Plant::Defaults()
    {
        plant_type = 0;
        plant_seeding_time = 0;
        plant_first_axis = 0;
        plant_first_branch = 0;
        plant_total_branches = 0;
        plant_total_tips = 0;
        plant_total_root_length = 0.0;
        plant_total_root_length_high = 0.0;

        for (long i=negative ; i <=positive ; i++)
        {
            for (int j=X ; j<Z ; j+=1) // No_Dimension = X-1
                //      WAS:
                //        for (Dimension j=X ; j<Z ; j+=1) // No_Dimension = X-1
            {
                plant_root_extremes[j][i] = 0.0;
            }
        }
        plant_is_more_extreme = true;

        plant_possible_seminal_axes = 0;
        plant_possible_nodal_axes = 0;
#ifdef _CS_DEBUG_PLANT_NEWHEADING_UNITVECTOR_
        do_unit_heading_check=0;
#endif
    }


    // void Plant::SaveToFile(JSimulationFile *file)
    // {
    // //#pragma message ("Plant::SaveToFile requires completion")
    // // need to save the actual axes, branches and tips
    //     JSFPlantPossibleAxisDescriptor descriptor;
    //     PossibleAxis *axis;
    // 
    //     axis = plant_possible_seminal_axes;
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
    //     axis = plant_possible_nodal_axes;
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
        //    Axis * current_axis = plant_first_axis;
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
        ForgetBranch(plant_first_axis);

        plant_first_axis = 0;
        plant_first_branch = 0;
        plant_total_branches = 0;
        plant_total_tips = 0;
        plant_total_root_length = 0.0;
        plant_total_root_length_high = 0.0;
    }


    void Plant::ForgetBranch(Branch * b)
    {
        RootMapLogDebug_PlantLowLevel("ForgetBranch id:" << b->m_id);
        if (b == 0) return;

        Branch * next_branch = 0;
        Branch * current_branch = b;

        while (current_branch != 0)
        {
            // 
            // must delete the child root tip. Must do so first, before 'b' becomes
            // invalidated by being deleted.
            RootTip * r = current_branch->m_child;

#ifdef PLANT_DEBUG_TIPCOUNT
            if ( r != 0 )
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
        (    DoubleCoordinate   * position,
        double_cs                length,
        double_cs                start_age,
        RootTip           * child,
        Branch            * child_branch,
        Branch            * parent_branch,
        BoxIndex       box,
        long int            branch_order,
        WrapDirection        wrap_direction)
    {
        Branch * new_branch = NEW Branch(position, length, start_age, child, child_branch, parent_branch);
        RootMapLogDebug_PlantLowLevel("MakeBranch id:" << new_branch->m_id);

        IncrementBranchCount(box, branch_order, wrap_direction);

#if defined _CS_DEBUG_WINDOW_ && defined _CS_DEBUG_MAKE_BRANCH_
        RmString delimiter=", ";
        RmString end_of_line="\r";
        RmString pstr="MakeBranch : ";
        RmString pstr2="";

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

    RootTip * Plant::MakeRootTip
        (   DoubleCoordinate  * position,
        DoubleCoordinate  * heading,
        double_cs           age,
        BoxIndex            box,
        long                branch_order,
        WrapDirection       wrap)
    {
        DoubleCoordinate double_position(position->x, position->y, position->z);
        double_cs initial_branch_angle=0;

        if (box == 0)
            box = plant_coordinator.FindBoxByDoubleCoordinate(position, &wrap);

        if (branch_order > 0)
        {
            initial_branch_angle = saInitialBranchAngle->GetValue(box, branch_order-1);
        }

        RootTip * new_root_tip = NEW RootTip(double_position, age, *heading, branch_order, initial_branch_angle);

        if (new_root_tip != 0)
        {
            IncrementTipCount(box, branch_order, wrap);
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
        (   double_cs            start_lag,
        DoubleCoordinate*    position,
        DoubleCoordinate*    start_orientation,
        AxisType             type_of_axis)
    {
        Axis * new_axis = 0;

        new_axis = NEW Axis(start_lag, position, start_orientation, type_of_axis);
        RootMapLogDebug_PlantLowLevel("MakeAxis id:" << new_axis->m_id);

        return (new_axis);
    }

    Axis * Plant::MakeSeminal(const PossibleAxis & possible_seminal)
    {
        DoubleCoordinate start_orientation(0.0,0.0,1.0);
        NewRootTipHeading(  &start_orientation,
            RootTipDeflection(saInitialSeminalDeflection->GetValue()),
            RootTipOrientation(0));

        Axis * new_axis = MakeAxis( possible_seminal.GetStartLag(),
            &plant_seed_location,
            &start_orientation,
            Seminal);

        return new_axis;
    }

    int Plant::AddAxis(Axis * new_axis)
    {
        /* if "last_axis" is 0, this is the first to be made,
        so we need to latch it onto various data members */
        if (plant_first_axis == 0)
        {
            plant_first_axis = new_axis;
            plant_first_branch = new_axis;
        }
        else // this is not the first axis
        {
            plant_first_axis->SetNext( new_axis );
        }

        return 1;
    }

    int Plant::MakeSeminals()
    {
        Axis * return_axis = 0;

        while (plant_possible_seminal_axes != 0)
        {
            // Pop the next possible seminal axis off the list
            PossibleAxis * possible_seminal = plant_possible_seminal_axes->Pop();

            // If it is the only or last possible axis to be considered, let the list point to nil
            if (possible_seminal == plant_possible_seminal_axes) plant_possible_seminal_axes = 0;

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
        PossibleAxis * possible_nodal;

        while (plant_possible_nodal_axes != 0)
        {
            possible_nodal = plant_possible_nodal_axes->Pop();
            if (possible_nodal == plant_possible_nodal_axes)
            {
                plant_possible_nodal_axes = 0;
            }

            if (possible_nodal->GetProbability() > Utility::Random1())
            {
                DoubleCoordinate start_orientation1(0, 0, +1);

                NewRootTipHeading(  &(start_orientation1),
                    pi/2,
                    RootTipOrientation(0));
                // set the second of the pair heading in the opposite horizontal direction
                DoubleCoordinate start_orientation2(-start_orientation1.x, -start_orientation1.y, start_orientation1.z);

                Axis * new_axis = MakeAxis(possible_nodal->GetStartLag(),
                    &plant_origin,
                    &(start_orientation1),
                    Nodal);
                AddAxis(new_axis);                

                new_axis = MakeAxis(possible_nodal->GetStartLag(),
                    &plant_origin,
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
        return ((plant_possible_nodal_axes != 0) || (plant_possible_seminal_axes != 0));
    }

    Boolean Plant::Start(ProcessActionDescriptor * action)
    {
        PrepareSharedAttributes(action);

        MakeSeminals();
        MakeNodals();

        return (plant_first_axis != 0);
    }

    void Plant::AdjustRootLength
        (   double_cs length,
        DoubleCoordinate * old_position,
        BoxCoordinate * box_coord,
        BoxIndex box,
        DoubleCoordinate * new_position,
        long branch_order,
        WrapDirection wrapping)
    {
        plant_total_root_length += length;

        //TODO: This replacement for the "RootLength High" characteristic KLUDGE should
        //  in turn be replaced/fixed by using a PlantSummaryRegistration
        if ( branch_order >= 2 )
        {
            plant_total_root_length_high += length;
        }

        plant_coordinator.AdjustRootLength(this, length, old_position, box_coord, box, new_position, branch_order, wrapping);
    }

    void Plant::IncrementBranchCount(DoubleCoordinate *position, long int branch_order)
    {
        BoxIndex box = 0;
        WrapDirection worder;
        box = plant_coordinator.FindBoxByDoubleCoordinate(position, &worder);
        Plant::IncrementBranchCount(box, branch_order, worder);
    }

    void Plant::IncrementBranchCount(BoxIndex box, long int branch_order, WrapDirection wrapping)
    {
        plant_total_branches++;
        plant_coordinator.AdjustBranchCount(this, +1, box, branch_order, wrapping);
#ifdef PLANT_DEBUG_BRANCHCOUNT
        CS_DEBUG_STREAM << "Plant::IncrementBranchCount(BoxIndex=" << box
            << ",RootOrder=" << branch_order
            << ",WrapDirection=" << wrapping
            << ") BranchCount=" << plant_total_branches << endl;
#endif // #ifdef PLANT_DEBUG_BRANCHCOUNT
    }

    void Plant::DecrementBranchCount(DoubleCoordinate *position, long int branch_order)
    {
        BoxIndex box = 0;
        WrapDirection worder;
        box = plant_coordinator.FindBoxByDoubleCoordinate(position, &worder);
        Plant::DecrementBranchCount(box, branch_order, worder);
    }

    void Plant::DecrementBranchCount(BoxIndex box, long int branch_order, WrapDirection wrapping)
    {
        plant_total_branches--;
        plant_coordinator.AdjustBranchCount(this, -1, box, branch_order, wrapping);
#ifdef PLANT_DEBUG_BRANCHCOUNT
        CS_DEBUG_STREAM << "Plant::DecrementBranchCount(BoxIndex=" << box
            << ",RootOrder=" << branch_order
            << ",WrapDirection=" << wrapping
            << ") BranchCount=" << plant_total_branches << endl;
#endif // #ifdef PLANT_DEBUG_BRANCHCOUNT
    }

    void Plant::IncrementTipCount(DoubleCoordinate *position, long int branch_order)
    {
        WrapDirection worder;
        BoxIndex box = plant_coordinator.FindBoxByDoubleCoordinate(position, &worder);
        IncrementTipCount(box, branch_order, worder);
    }

    void Plant::IncrementTipCount(BoxIndex box, long int branch_order, WrapDirection wrapping)
    {
        plant_total_tips++;
        plant_coordinator.AdjustTipCount(this, +1, box, branch_order, wrapping);
#ifdef PLANT_DEBUG_TIPCOUNT
        CS_DEBUG_STREAM << "Plant::IncrementTipCount(BoxIndex=" << box
            << ",RootOrder=" << branch_order
            << ",WrapDirection=" << wrapping
            << ") TipCount=" << plant_total_tips << endl;
#endif // #ifdef PLANT_DEBUG_TIPCOUNT
    }

    void Plant::DecrementTipCount(DoubleCoordinate *position, long int branch_order)
    {
        WrapDirection worder;
        BoxIndex box = plant_coordinator.FindBoxByDoubleCoordinate(position, &worder);
        DecrementTipCount(box, branch_order, worder);
    }

    void Plant::DecrementTipCount(BoxIndex box, long int branch_order, WrapDirection wrapping)
    {
        plant_total_tips--;
        plant_coordinator.AdjustTipCount(this, -1, box, branch_order, wrapping);
#ifdef PLANT_DEBUG_TIPCOUNT
        CS_DEBUG_STREAM << "Plant::DecrementTipCount(BoxIndex=" << box
            << ",RootOrder=" << branch_order
            << ",WrapDirection=" << wrapping
            << ") TipCount=" << plant_total_tips << endl;
#endif // #ifdef PLANT_DEBUG_TIPCOUNT
    }

    void Plant::GetSoilRect(DoubleRect* r, ViewDirection direction)
    {
        switch (direction)
        {
        case vFront:
            r->left = plant_root_extremes[X][negative];
            r->right = plant_root_extremes[X][positive];
            r->top = plant_root_extremes[Z][negative];
            r->bottom = plant_root_extremes[Z][positive];
            break;

        case vSide:
            r->left = plant_root_extremes[Y][negative];
            r->right = plant_root_extremes[Y][positive];
            r->top = plant_root_extremes[Z][negative];
            r->bottom = plant_root_extremes[Z][positive];
            break;

        case vTop:
            r->left = plant_root_extremes[X][negative];
            r->right = plant_root_extremes[X][positive];
            r->top = plant_root_extremes[Y][negative];
            r->bottom = plant_root_extremes[Y][positive];
            break;
        }
    }

    // prototype, for the compiler's sake.
    Boolean AboveGround(DoubleCoordinate *position, DoubleCoordinate *heading);
    // implementation
    Boolean AboveGround(DoubleCoordinate *position, DoubleCoordinate *heading)
    {
        Boolean above_ground = ((position->z < 0) && (heading->z < 0));
        if (above_ground)
        {
            heading->z = -(heading->z);
            ScaleRootTipHeading(heading);
        }

        return (above_ground);
    }

    Branch * Plant::GrowRootTip
        (   RootTip* root_tip,
        long int branch_order,
        double_cs time_step_fraction,
        double_cs time_step)
    {

        DoubleCoordinate old_position = root_tip->roottip_position;
        double_cs length = root_tip->roottip_length;

        WrapDirection wrap_direction;
        BoxCoordinate box_coord;
        BoxIndex box = plant_coordinator.FindBoxByDoubleCoordinate(&old_position, &wrap_direction, &box_coord);

        RootMapLogDebug_PlantLowLevel("GrowRootTip id:" << root_tip->m_id);
        //<< ", old_position:" << ToString(old_position).c_str()
        //<< ", heading:" << ToString(root_tip->roottip_heading).c_str()
        //<< ", length:" << length
        //<< ", boxindex:" << box;

        //     RmString plant_name = GetName();
        //     RmString search_name1 = "Plant 1";
        //     RmString search_name2 = "Plant 2";
        //     RmString search_name3 = "Plant 3";
        //     RmString search_name4 = "Plant 4";
        // 
        //    if (EqualString(*plant_name, search_name1, true, false)) // case-sens, diacritical-insense
        //    {
        //        plant_number = 1;
        //        growth_rate_max = saGrowthRateMax->GetValue1(branch_order, box, plant_number);
        //    }
        //    if (EqualString(*plant_name, search_name2, true, false)) // case-sens, diacritical-insense
        //    {
        //        plant_number = 2;
        //        growth_rate_max = saGrowthRateMax->GetValue2(branch_order, box, plant_number);
        //    }
        //    if (EqualString(*plant_name, search_name3, true, false)) // case-sens, diacritical-insense
        //    {
        //        plant_number = 3;
        //        growth_rate_max = saGrowthRateMax->GetValue3(branch_order, box, plant_number);
        //    }
        //    if (EqualString(*plant_name, search_name4, true, false)) // case-sens, diacritical-insense
        //    {
        //        plant_number = 4;
        //        growth_rate_max = saGrowthRateMax->GetValue4(branch_order, box, plant_number);
        //    }    

        // WAS:
        //growth_rate_max = plant_coordinator.GetGrowthRateMax(branch_order, box);
        double_cs growth_rate_max = saGrowthRateMax->GetValue(box, branch_order);

#ifdef _CS_DEBUG_    
        //double_cs tip_growth_duration = plant_coordinator.GetTipGrowthDuration(branch_order, box);
        double_cs tip_growth_duration = saTipGrowthDuration->GetValue(box, branch_order);

        if ( ( growth_rate_max < 0.01 ) || ( tip_growth_duration < 1000 ) )
        {
            Debugger();
        }
#endif // #ifdef _CS_DEBUG_    

        //    growth_increment = plant_coordinator.TempAdjustRate(growth_rate_max, box, plant_type->GetTemperatureOfZeroGrowth()) * plant_coordinator.GetRootConductance(branch_order, box) * time_step_fraction * time_step;
        double_cs temperature_of_zero_growth = saTemperatureOfZeroGrowth->GetValue();
        double_cs root_conductance = saRootConductance->GetValue(box, branch_order);
        double_cs temperature_adjusted_rate = plant_coordinator.TempAdjustRate(growth_rate_max, box, temperature_of_zero_growth);
        double_cs growth_increment = temperature_adjusted_rate * root_conductance * time_step_fraction * time_step;

        DoubleCoordinate the_roottip_heading = (root_tip->roottip_heading);

        //VMD 26/1/99
        //The tip gowth duration and growth rate max are set to zero in plant coordinator for a scoreboard box if it is dry and has been dry for some time, hence the roots should die.
        //if((tip_growth_duration == 0) && (growth_rate_max == 0))
        //{
        //    root_tip->roottip_dead = 1;
        //}

        NewRootTipHeading(  &the_roottip_heading,
            RootTipDeflection(saDeflectionIndex->GetValue(box, branch_order)),
            RootTipOrientation(saGeotropismIndex->GetValue(box, branch_order)));


        Branch * new_branch = 0;
        //if (root_tip->roottip_dead == 1)
        //{
        //    //the root tip is dead, it cannot grow.
        //    //VMD 25/1/99
        //    position.x = old_position.x;
        //    position.y = old_position.y;
        //    position.z = old_position.z;
        //    
        //    growth_increment = 0;
        //    
        //    while (!IsBelowGround(&position, &the_roottip_heading));
        //
        //    length += growth_increment;
        //    
        //
        //    UpdateExtremes(&position);
        //    root_tip->roottip_position = position;
        //    root_tip->roottip_heading = the_roottip_heading;
        //    root_tip->roottip_length = length;
        //
        //    AdjustRootLength(growth_increment, &old_position, &box_coord, box, &position, branch_order, wrap_direction);
        //}
        //else 
        {
            DoubleCoordinate position;
            do
            {
                position.x = old_position.x + growth_increment*the_roottip_heading.x;
                position.y = old_position.y + growth_increment*the_roottip_heading.y;
                position.z = old_position.z + growth_increment*the_roottip_heading.z;
            }
            while (AboveGround(&position, &the_roottip_heading));

            length += growth_increment;

            double_cs interval = saInitialBranchInterval->GetValue(box, branch_order);

            if (length > interval)
            {
                new_branch = MakeBranch(&position, length, 0, 0, 0, 0, box, branch_order, wrap_direction);
                length = 0;
            }

            UpdateExtremes(&position);
            root_tip->roottip_position = position;
            root_tip->roottip_heading = the_roottip_heading;
            root_tip->roottip_length = length;

            AdjustRootLength(growth_increment, &old_position, &box_coord, box, &position, branch_order, wrap_direction);
            RmAssert( ((position.x<1000)&&(position.y<1000)&&(position.z<1000)), "RootTip position gone berserk");

            /**
            *  MSA All root segment drawing instructions output from Plant.cpp now use
            *  the DrawRoot() methods. This is to allow buffers to interpret the order
            *  to draw a root segment differently. 
            *  ProcessDrawingBuffer will pass on instructions to draw a line, and 
            *  ProcessVertexBuffer will pass on instructions to draw either a line
            *  or a cylinder, depending on configuration settings.
            */
            BEGIN_DRAWING
                drawing.DrawRoot(old_position, position, branch_order, root_tip->m_id);
            END_DRAWING
                /**
                *  MSA record this id/order pair for future reference,
                *  after asserting that there is room left in the std::maps. 
                */
                RmAssert(m_branch_order_map.size()<m_maxmap && m_new_segment_map.size()<m_maxmap, 
                "Map of (plant element ID, branch order) pairs has exceeded maximum size possible on this system");
            if(0!=new_branch)
            {
                m_branch_order_map[new_branch->m_id] = branch_order;
                // Mark new (not yet raytraced) root segments as such
                m_new_segment_map[new_branch->m_id]=true;
            }
            m_branch_order_map[root_tip->m_id] = branch_order;
            m_new_segment_map[root_tip->m_id]=true;
        }

        return (new_branch);
    }

    void Plant::UpdateExtremes(DoubleCoordinate* possible_new_extreme)
    {
        if (possible_new_extreme->x < plant_root_extremes[X][negative])
        {
            plant_root_extremes[X][negative] = possible_new_extreme->x;
            plant_is_more_extreme = true;
        }

        if (possible_new_extreme->x > plant_root_extremes[X][positive])
        {
            plant_root_extremes[X][positive] = possible_new_extreme->x;
            plant_is_more_extreme = true;
        }

        if (possible_new_extreme->y < plant_root_extremes[Y][negative])
        {
            plant_root_extremes[Y][negative] = possible_new_extreme->y;
            plant_is_more_extreme = true;
        }

        if (possible_new_extreme->y > plant_root_extremes[Y][positive])
        {
            plant_root_extremes[Y][positive] = possible_new_extreme->y;
            plant_is_more_extreme = true;
        }

        if (possible_new_extreme->z < plant_root_extremes[Z][negative])
        {
            plant_root_extremes[Z][negative] = possible_new_extreme->z;
            plant_is_more_extreme = true;
        }

        if (possible_new_extreme->z > plant_root_extremes[Z][positive])
        {
            plant_root_extremes[Z][positive] = possible_new_extreme->z;
            plant_is_more_extreme = true;
        }
    }

    Branch* Plant::BranchEdit
        (   Branch*        this_branch,
        Branch*        last_branch,
        RootTip*        parent,
        long int        branch_order,
        double_cs        time_step,
        double_cs        branch_lag_time)
    {
        RootMapLogDebug_PlantLowLevel("BranchEdit branch id:" << this_branch->m_id << ", parent id:" << parent->m_id);
        Branch* return_branch = this_branch;
        BranchEditData bed(this_branch, last_branch, parent, branch_order, time_step, branch_lag_time);

        // Initialise the segment heading correctly
        bed.segment_heading.set( this_branch->m_position.x - last_branch->m_position.x, this_branch->m_position.y - last_branch->m_position.y, this_branch->m_position.z - last_branch->m_position.z);
        ScaleRootTipHeading(&(bed.segment_heading));

        // Find the box in which the branch node lives
        bed.box = plant_coordinator.FindBoxByDoubleCoordinate(&(this_branch->m_position), &(bed.wrap));

        //bed.branch_interval = plant_coordinator.GetFinalBranchInterval(branch_order, bed.box);
        bed.branch_interval = saFinalBranchInterval->GetValue(bed.box, branch_order);

        while (this_branch->m_length >= (2*bed.branch_interval))
        {
            if (last_branch->m_parentBranch == this_branch)
            {
                WhereAndWhen(this_branch, last_branch, parent, branch_lag_time, time_step, &bed);
                last_branch->m_parentBranch = InsertBranch(this_branch, branch_order, time_step, &bed);    
                last_branch = last_branch->m_parentBranch;
            }
            else
            {
                WhereAndWhen(this_branch, last_branch, parent, branch_lag_time, time_step, &bed);
                last_branch->m_childBranch = InsertBranch(this_branch, branch_order, time_step, &bed);    
                last_branch = last_branch->m_childBranch;
            }
        }

        if (this_branch->m_length >= bed.branch_interval)
        {
            WhereAndWhen(this_branch, last_branch, parent, branch_lag_time, time_step, &bed);
            MoveBranch(this_branch, parent, branch_order, time_step, &bed);
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
        (    Branch*    this_branch,
        Branch*    last_branch,
        RootTip*    parent,
        double_cs    branch_lag_time,
        double_cs    time_step,
        BranchEditData* edit_data)
    {
        RootMapLogDebug_PlantLowLevel("WhereAndWhen branch id:" << this_branch->m_id << ", parent id:" << parent->m_id);
        double_cs length_fraction = edit_data->branch_interval/this_branch->m_length;

        if (last_branch->m_child == parent)
            edit_data->branch_age = ((1 - length_fraction) * (parent->roottip_age - this_branch->m_age) + this_branch->m_age);
        else
            edit_data->branch_age = ((1 - length_fraction) * (last_branch->m_age - this_branch->m_age) + this_branch->m_age);

        edit_data->time_step_fraction = (edit_data->branch_age - branch_lag_time)/time_step;

        edit_data->next_position.x = last_branch->m_position.x + length_fraction * (this_branch->m_position.x - last_branch->m_position.x);
        edit_data->next_position.y = last_branch->m_position.y + length_fraction * (this_branch->m_position.y - last_branch->m_position.y);
        edit_data->next_position.z = last_branch->m_position.z + length_fraction * (this_branch->m_position.z - last_branch->m_position.z);
    }

    Branch* Plant::InsertBranch
        (   Branch *         at_branch,
        long             branch_order,
        double_cs        time_step,
        BranchEditData * bed)
    {
        RootMapLogDebug_PlantLowLevel("InsertBranch at branch id:" << at_branch->m_id);
        Branch * b = 0;

        b = MakeBranch(&(bed->next_position), bed->branch_interval, bed->branch_age, 0, 0, at_branch, bed->box, branch_order, bed->wrap);
        b->m_child = MakeRootTip(&(b->m_position), &(bed->segment_heading), bed->time_step_fraction*time_step, bed->box, branch_order+1, bed->wrap);
        b->m_childBranch = GrowRootTip(b->m_child, branch_order+1, bed->time_step_fraction, time_step);
        at_branch->m_length -= bed->branch_interval;

        return (b);
    }

    void Plant::MoveBranch
        (   Branch*            the_branch,
        RootTip*            the_parent,
        long            branch_order,
        double_cs            time_step,
        BranchEditData*    bed)
    {
        RootMapLogDebug_PlantLowLevel("MoveBranch id:" << the_branch->m_id);
        DoubleCoordinate heading( bed->segment_heading );
        DoubleCoordinate good_position(the_branch->m_position.x, the_branch->m_position.y, the_branch->m_position.z);

        the_branch->m_position = bed->next_position;
        the_branch->m_child = MakeRootTip(&good_position, &heading, (double_cs)(bed->time_step_fraction*time_step), bed->box, (branch_order+1), bed->wrap);
        the_branch->m_childBranch = GrowRootTip(the_branch->m_child, branch_order+1, bed->time_step_fraction, time_step);

        if (the_branch->m_parentBranch != 0)
            the_branch->m_parentBranch->m_length += the_branch->m_length - bed->branch_interval;
        else
            the_parent->roottip_length += the_branch->m_length - bed->branch_interval;

        the_branch->m_length = bed->branch_interval;
        the_branch->m_age = bed->branch_age;
    }

    Branch* Plant::RemoveBranch
        (   Branch* the_branch,
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

        DecrementBranchCount(&(the_branch->m_position), branch_order);

        TCLForgetObject(the_branch);

        return (return_branch);
    }

    void Plant::GrowBranch
        (    Branch*        this_branch,
        Branch*        last_branch,
        RootTip*        parent,
        long int        branch_order,
        double_cs        time_step)
    {
        RootMapLogDebug_PlantLowLevel("GrowBranch id:" << this_branch->m_id);
        Branch * corner_branch = last_branch;

        do
        {
            BoxIndex box = plant_coordinator.FindBoxByDoubleCoordinate(&(this_branch->m_position));

            //        double_cs branch_lag_time = plant_coordinator.GetBranchLagTime(branch_order, box);
            double_cs branch_lag_time = saBranchLagTime->GetValue(box, branch_order);

            double_cs growth_time_adjusted = plant_coordinator.TempAdjustRate(time_step, box, saTemperatureOfZeroGrowth->GetValue());
            this_branch->m_age += growth_time_adjusted;

            if (this_branch->m_childBranch != 0)
            {
                //            if (this_branch->m_child->roottip_age <= plant_coordinator.GetTipGrowthDuration(branch_order+1, box))
                if (this_branch->m_child->roottip_age <= saTipGrowthDuration->GetValue(box, branch_order+1))
                {
                    this_branch->m_child->roottip_age += growth_time_adjusted;
                    GrowBranch(this_branch->m_childBranch, this_branch, this_branch->m_child, branch_order+1, time_step);
                }
            }
            else if (this_branch->m_child != 0)
            {
                this_branch->m_child->roottip_age += growth_time_adjusted;
                this_branch->m_childBranch = GrowRootTip(this_branch->m_child, branch_order+1, 1, time_step);
            }
            else if (this_branch->m_age >= branch_lag_time)
            {
                this_branch = BranchEdit(this_branch, last_branch, parent, branch_order, time_step, branch_lag_time);
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
        }
        while (this_branch != 0);

        last_branch->m_parentBranch = GrowRootTip(parent, branch_order, 1, time_step);
    }

    void Plant::GrowAxis(Axis* axis, double_cs time_step, double_cs adjusted_time)
    {
        RootMapLogDebug_PlantLowLevel("GrowAxis id:" << axis->m_id);
        //<< ", age:" << axis->m_age
        //<< ", orientation:" << ToString(axis_start_orientation).c_str()
        //<< ", position:" << ToString(axis->m_position).c_str();
        axis->m_child->roottip_age += adjusted_time;
        if (axis->m_childBranch == 0)
            axis->m_childBranch = GrowRootTip(axis->m_child, 0, 1, time_step);
        else
            GrowBranch(axis->m_childBranch, axis, axis->m_child, 0, time_step);
    }

    void Plant::StartAxis(Axis* axis, double_cs time_step, double_cs adjusted_time)
    {
        double_cs time_step_fraction = (axis->m_age - saGerminationLag->GetValue() - axis->GetStartLag()-plant_seeding_time)/adjusted_time;
        DoubleCoordinate axis_start_orientation;
        axis->GetStartOrientation( &axis_start_orientation );
        RootMapLogDebug_PlantLowLevel("StartAxis id:" << axis->m_id);
        //<< ", age:" << axis->m_age
        //<< ", orientation:" << ToString(axis_start_orientation).c_str()
        //<< ", position:" << ToString(axis->m_position).c_str();


        axis->m_age -= (saGerminationLag->GetValue()+plant_seeding_time+axis->GetStartLag());
        axis->m_child = MakeRootTip(&(axis->m_position), &axis_start_orientation, time_step_fraction*time_step, 0, 0, wraporderNone);
        axis->m_childBranch = GrowRootTip(axis->m_child, 0, time_step_fraction, time_step);

        RootMapLogDebug_PlantLowLevel("Started Plant Axis id:" << axis->m_id);
        //<< ", Start Orientation:" << ToString(axis_start_orientation).c_str()
        //<< ", Position:" << ToString(axis->m_position).c_str();
    }

    void Plant::NextTimeStep(double_cs time_step)
    {
        Axis* this_axis = plant_first_axis;
        Axis* last_axis = 0;
        DoubleCoordinate start_orientation;// = {0.0, 0.0, 0.0};  Default constructor handles that, now
        BoxIndex box = 0;
        double_cs time_increment;

        while (this_axis != 0)
        {
            try
            {
                if (this_axis->GetType() == Seminal)
                {
                    box = plant_coordinator.FindBoxByDoubleCoordinate(&plant_seed_location);
                }
                else
                {
                    box = plant_coordinator.FindBoxByDoubleCoordinate(&plant_origin);
                }
            }
            catch (const std::exception & stdex)
            {
                LOG_ALERT << "Caught std::exception calling PlantCoordinator::FindBoxByDoubleCoordinate() : " << stdex.what();
                RmAssert(false, "Caught std::exception calling PlantCoordinator::FindBoxByDoubleCoordinate()");
            }
            catch (...)
            {
                RmAssert(false, "Caught non-standard exception calling PlantCoordinator::FindBoxByDoubleCoordinate()");
            }

            time_increment = plant_coordinator.TempAdjustRate(time_step, box, saTemperatureOfZeroGrowth->GetValue());
            this_axis->m_age += time_increment;

            //        if ((this_axis->m_child != 0) && (this_axis->m_age <= plant_coordinator.GetTipGrowthDuration(0, box)))
            if ((this_axis->m_child != 0) && (this_axis->m_age <= saTipGrowthDuration->GetValue(box, 0)))
            {
                try
                {
                    GrowAxis(this_axis, time_step, time_increment);
                }
                catch (const std::exception & stdex)
                {
                    LOG_ALERT << "Caught std::exception calling Plant::GrowAxis() : " << stdex.what();
                    RmAssert(false, "Caught std::exception calling Plant::GrowAxis()");
                }
                //catch (...)
                //{
                //    RmAssert(false, "Caught non-standard exception calling Plant::GrowAxis()");
                //}
            }
            else if (this_axis->m_age >= (this_axis->GetStartLag() + saGerminationLag->GetValue()+plant_seeding_time))
            {
                try
                {
                    StartAxis(this_axis, time_step, time_increment);
                }
                catch (const std::exception & stdex)
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

    // The buffer will already be set up as the active drawing port
    void Plant::DrawRoots(const DoubleRect & /* area */, Scoreboard * /* scoreboard */)
    {
        if (plant_first_axis == 0) return;

        BEGIN_DRAWING
            Axis * this_axis = plant_first_axis;
        do
        {
            //drawing.MovePenTo(this_axis->m_position);          // MSA no longer necessary

            if (this_axis->m_childBranch != 0)
            {
                DrawBranch(this_axis->m_position, this_axis->m_childBranch, this_axis->m_child, drawing);
            }
            else if (this_axis->m_child != 0)
            {
                //drawing.DrawLineTo(this_axis->m_child->roottip_position);         // MSA superseded by:
                drawing.DrawRoot(this_axis->m_position, this_axis->m_child->roottip_position, 
                    m_branch_order_map[this_axis->m_child->m_id], this_axis->m_child->m_id);
            }

            this_axis = (Axis*)(this_axis->m_parentBranch);
        }
        while (this_axis != 0);
        END_DRAWING
    }

    void Plant::DrawBranch(const DoubleCoordinate & starting_point, Branch * this_branch,
        RootTip * parent, ProcessDrawing & /* drawing */)
    {
        DoubleCoordinate coord = starting_point;
        BEGIN_DRAWING
            do
            {
                // Draw to ourself first. This also ensures that the pen is in the correct place
                //drawing.DrawLineTo(this_branch->m_position);         // MSA superseded by:
                drawing.DrawRoot(coord, this_branch->m_position, 
                    m_branch_order_map[this_branch->m_id], 
                    this_branch->m_id);
                coord = this_branch->m_position;

                /* Oh, do let the children draw, Dear.
                If we have a child (ie. a next-order branch), let it draw next */
                if (this_branch->m_childBranch != 0)
                {
                    DrawBranch(coord, this_branch->m_childBranch, this_branch->m_child, drawing);
                }
                // otherwise, if we only have a child (next-order rootTip) then just draw that
                else if (this_branch->m_child != 0)
                {
                    // drawing.DrawLineTo(this_branch->m_child->roottip_position);        // MSA superseded by:
                    drawing.DrawRoot(coord, this_branch->m_child->roottip_position, 
                        m_branch_order_map[this_branch->m_child->m_id], 
                        this_branch->m_child->m_id);
                }

                // And when they're all quite finished drawing, we need to retrieve the pen
                //drawing.MovePenTo(this_branch->m_position);        // MSA no longer necessary
                //Éthen head off to the next branch in our root-order's line
                this_branch = this_branch->m_parentBranch;
            }
            while (this_branch != 0); // we get to the end of the branches on this root-order

            //
            // Finally we must be at the end - draw our parent.}
            //drawing.DrawLineTo(parent->roottip_position);         // MSA superseded by:
            drawing.DrawRoot(coord, parent->roottip_position, 
                m_branch_order_map[parent->m_id], 
                parent->m_id);
            END_DRAWING
    }

    void Plant::PrepareSharedAttributes(ProcessActionDescriptor * action)
    {
        SharedAttributeSearchHelper helper(action->GetSharedAttributeManager(), plant_coordinator.GetSharedAttributeOwner());

        //
        RmString plant_name = GetProcessName();

        //
        // Load up our expected SharedAttributes
        //
        // Per-RootOrder (no plant/planttype or any other)
        saBranchLagTime         = helper.SearchForAttribute("Branch Lag Time",          "RootOrder"); 
        saDeflectionIndex       = helper.SearchForAttribute("Deflection Index",         "RootOrder");
        saFinalBranchInterval   = helper.SearchForAttribute("Final Branch Interval",    "RootOrder");
        saGeotropismIndex       = helper.SearchForAttribute("Geotropism Index",         "RootOrder");
        saInitialBranchAngle    = helper.SearchForAttribute("Initial Branch Angle",     "RootOrder");
        saInitialBranchInterval = helper.SearchForAttribute("Initial Branch Interval",  "RootOrder");
        saRootConductance       = helper.SearchForAttribute("Root Conductance",         "RootOrder");
        saTipGrowthDuration     = helper.SearchForAttribute("Tip Growth Duration",      "RootOrder");

        saGrowthRateMax         = helper.SearchForAttribute("Growth Rate Max",          "Plant,RootOrder");
        // MSA 09.10.28 Bug reproduction testing
        //saGrowthRateMax = helper.SearchForAttribute("Growth Rate Max", "Plant,RootOrder",plant_name.c_str());

        //
        // per-PlantType
        saInitialSeminalDeflection  = helper.SearchForAttribute("Initial Seminal Deflection","PlantType");
        saGerminationLag            = helper.SearchForAttribute("Germination Lag",           "Plant");
        saTemperatureOfZeroGrowth   = helper.SearchForAttribute("Temperature of Zero Growth","PlantType");
        saFirstSeminalProbability   = helper.SearchForAttribute("First Seminal Probability", "PlantType");
        saSeedingTime                = helper.SearchForAttribute("Seeding Time",           "Plant");
        saSeedingTime->SetValue(plant_seeding_time);

        //
        // per-Plant attributes (no RootOrder). Note that we want only the attribute
        // for THIS plant
        saWaterUptake       = helper.SearchForAttribute("Local Plant Water Uptake",     "Plant",plant_name.c_str());
        saPhosphorusUptake  = helper.SearchForAttribute("Phosphorus Uptake","Plant",plant_name.c_str());
        saNitrateUptake     = helper.SearchForAttribute("Plant Nitrate Uptake","Plant",plant_name.c_str());
        saTotalRootLength   = helper.SearchForAttribute("Root Length Wrap None","Plant",plant_name.c_str());
        saPreviousTotalRootLength = helper.SearchForAttribute("Previous Total Root Length","Plant",plant_name.c_str());
        saSeedSize = helper.SearchForAttribute("Seed Size","Plant",plant_name.c_str());
        saPlantIsLegume = helper.SearchForAttribute("Plant Is Legume","Plant",plant_name.c_str());

    }

    bool Plant::DoesDrawing() const
    {
        return (true);
    }

    bool Plant::DoesRaytracerOutput() const
    {
        return (true);
    }


    DoubleCoordinate Plant::GetOrigin()
    {
        return plant_origin;
    }

    long int Plant::DoRaytracerOutput(RaytracerData * raytracerData)
    {
        if (0==raytracerData || 0==plant_first_axis  || m_new_segment_map.empty()) return kNoError;

        Axis * this_axis = plant_first_axis;
        do
        {
            if (this_axis->m_childBranch != 0)
            {
                RaytracerCacheBranch(this_axis->m_position, this_axis->m_childBranch, this_axis->m_child, raytracerData);
            }
            else if (this_axis->m_child != 0 && m_new_segment_map[this_axis->m_child->m_id])
            {        
                // Buffer a raytracer object to represent this root segment
                unsigned long order = m_branch_order_map[this_axis->m_child->m_id];
                raytracerData->AddRootSegment(this_axis->m_position, this_axis->m_child->roottip_position, order);
            }

            this_axis = (Axis*)(this_axis->m_parentBranch);
        }
        while (this_axis != 0);

        // All extant root segments have now been cached in m_raytracerData.
        // Clear the decks for new root segments.
        m_new_segment_map.clear();

        return kNoError;
    }

    void Plant::RaytracerCacheBranch
        (   const DoubleCoordinate & starting_point,
        Branch * this_branch,
        RootTip * parent,
        RaytracerData * raytracerData)
    {
        // Copy construct a working coordinate
        DoubleCoordinate coord = starting_point;
        unsigned long order;

        do
        {
            if(m_new_segment_map[this_branch->m_id])
            {
                // Buffer a raytracer object to represent this root segment
                order = m_branch_order_map[this_branch->m_id];
                raytracerData->AddRootSegment(coord, this_branch->m_position, order);
            }            
            // Important: must always update the working coord (MSA bug fix)
            coord = this_branch->m_position;

            // If we have a child (ie. a next-order branch), let it cache next 
            if (this_branch->m_childBranch != 0)
            {
                RaytracerCacheBranch(coord, this_branch->m_childBranch, this_branch->m_child, raytracerData);
            }
            // otherwise, if we only have a child (next-order rootTip) then just cache that
            else if (this_branch->m_child != 0 && m_new_segment_map[this_branch->m_child->m_id])
            {        
                // Buffer a raytracer object to represent this root segment
                order = m_branch_order_map[this_branch->m_id];
                raytracerData->AddRootSegment(coord, this_branch->m_child->roottip_position, order);
            }

            // head off to the next branch in our root-order's line
            this_branch = this_branch->m_parentBranch;
        }
        while (this_branch != 0); // we get to the end of the branches on this root-order

        if(m_new_segment_map[parent->m_id])
        {
            // Finally, cache the parent.
            order = m_branch_order_map[parent->m_id];
            raytracerData->AddRootSegment(coord, parent->roottip_position, order);
        }
    }

    void Plant::DrawScoreboard(const DoubleRect & area, Scoreboard * scoreboard)
    {
        if ( scoreboard->GetScoreboardStratum().isBelowZero() )
        {
            DrawRoots(area, scoreboard);
        }
    }




    //#define Get_BoxIndex_Volume(box_index) scoreboard->GetBoxVolume(box_index)
    //#define Use_Layers  LHorizonArray *x_layers = scoreboardcoordinator->GetLayerList(vSide, pVolumeType); \
    //                    LHorizonArray *y_layers = scoreboardcoordinator->GetLayerList(vFront, pVolumeType); \
    //                    LHorizonArray *z_layers = scoreboardcoordinator->GetLayerList(vTop, pVolumeType)
#define Get_Left_Of_X_Layer(layer)              Get_Layer_Left(x)
#define Get_Front_Of_Y_Layer(layer)             Get_Layer_Front(y)
#define Get_Top_Of_Z_Layer(layer)               Get_Layer_Top(z)
#define Get_Right_Of_X_Layer(layer)             Get_Layer_Right(x)
#define Get_Back_Of_Y_Layer(layer)              Get_Layer_Back(y)
#define Get_Bottom_Of_Z_Layer(layer)            Get_Layer_Bottom(z)




    BranchEditData::BranchEditData()
        : this_branch( 0 )
        , last_branch( 0 )
        , parent( 0 )
        , branch_order( 0 )
        , time_step( 0 )
        , branch_lag_time( 0 )
        , segment_heading( 0.0, 0.0, 0.0 )
        //, next_position( 0.0, 0.0, 0.0 )
        , branch_interval( 0 )
        , branch_age( 0 )
        , time_step_fraction( 0 )
        , return_branch( 0 )
        , box( 0 )
        , wrap( wraporderNone )
    {
    }

    BranchEditData::BranchEditData(    
        Branch * tb,
        Branch * lb,
        RootTip * pa,
        long int bo,
        double_cs ts,
        double_cs blt)
        : this_branch( tb )
        , last_branch( lb )
        , parent( pa )
        , branch_order( bo )
        , time_step( ts )
        , branch_lag_time( blt )
        , segment_heading( 0, 0, 0 )
        //, next_position( 0, 0, 0 )
        , branch_interval( 0 )
        , branch_age( 0 )
        , time_step_fraction( 0 )
        , return_branch( 0 )
        , box( 0 )
        , wrap( wraporderNone )
    {
    }


} // namespace rootmap

