/////////////////////////////////////////////////////////////////////////////
// Name:        PlantSummary.cpp
// Purpose:     Implementation of the PlantSummary class
// Created:     1994
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2002-2008 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/plant/PlantSummary.h"
#include "simulation/process/plant/PlantCoordinator.h"
#include "simulation/common/SprintUtility.h"
#include "core/utility/Utility.h"
#include "core/scoreboard/ScoreboardStratum.h"
#include "core/common/Constants.h"
#include "core/macos_compatibility/MacResourceManager.h"


// #include "core/common/ExtraStuff.h"


namespace rootmap
{
    RootMapLoggerDefinition(PlantSummary);

    PlantSummary::PlantSummary()
        : m_plant(0)
        , m_branch_order(-1)
        , m_volume_object_index(0)
        , m_wrap_order(wraporderNone)
        , m_routine(srNone)
        , m_characteristic_descriptor(0)
    {
        RootMapLoggerInitialisation("rootmap.PlantSummary");
    }


    void PlantSummary::IPlantSummary
    (
        PlantCoordinator* plantcoordinator,
        const ScoreboardStratum& stratum,
        long branching_order,
        const size_t& volume_object_index,
        WrapDirection wrapping_order,
        SummaryRoutine summary_type,
        const std::string& name)
    {
        //
        // Create and populate the CharacteristicDescriptor
        m_characteristic_descriptor = new CharacteristicDescriptor;
        m_characteristic_descriptor->id = plantcoordinator->GetNewSummaryID(GetSummaryTypeArrayIndex(), stratum);
        // 20020819 RvH - the ScoreboardIndex is now set by RegisterCharacteristic
        m_characteristic_descriptor->ScoreboardIndex = -1; // plantcoordinator->GetNewCharacteristicIndex(m_stratum);
        // summary minima and maxima are always as follows
        m_characteristic_descriptor->Minimum = 0.0;
        m_characteristic_descriptor->Maximum = Utility::infinity();
        m_characteristic_descriptor->Default = 0.0;
        //
        m_characteristic_descriptor->Name = name;
        // summaries are visible, unless they are requested by another process via
        // a PlantSummaryRegistration, in which case the PlantCoordinator will explicitly
        // make the summary false after calling this initialiser
        m_characteristic_descriptor->Visible = true;
        // a plant summary cannot be editted in Table Windows
        m_characteristic_descriptor->Edittable = false;
        // the information of a plant summary should not need to be saved
        m_characteristic_descriptor->Savable = false;
        // a plant summary is a vanilla numeric value
        m_characteristic_descriptor->SpecialPerBoxInfo = false;
        //
        m_characteristic_descriptor->m_ScoreboardStratum = stratum;

        //
        // Summary data members
        m_branch_order = branching_order;
        m_volume_object_index = volume_object_index;
        m_wrap_order = wrapping_order;
        m_routine = summary_type;

        // Units (only definable after the routine type is known)
        m_characteristic_descriptor->Units = Utility::NewUnitsString(m_routine);

        LOG_INFO << "New PlantSummary (New) " << *this;
    }


    void PlantSummary::IPlantSummary
    (PlantCoordinator* plantcoordinator,
        PlantSummary* other)
    {
        // Copy the Name string, as "Copy of [old name]"
        std::string copy_name;
        MacResourceManager::GetIndString(copy_name, STRLMiscellaneous, STRLmCopyOf);
        std::string old_name;
        old_name = other->GetCharacteristicName();
        copy_name += old_name;
        std::string new_name = copy_name;

        // get some other important values sorted out
        long new_id = plantcoordinator->GetNewSummaryID(GetSummaryTypeArrayIndex(), GetScoreboardStratum());
        // 20020819 RvH - the ScoreboardIndex is now set by RegisterCharacteristic
        long new_scoreboard_index = -1; // plantcoordinator->GetNewCharacteristicIndex(m_stratum);

        m_characteristic_descriptor = new CharacteristicDescriptor(
            *(other->m_characteristic_descriptor),
            new_id,
            new_scoreboard_index,
            new_name);

        m_plant = other->m_plant;
        m_plants = other->m_plants;

        m_volume_object_index = other->m_volume_object_index;

        m_branch_order = other->m_branch_order;
        m_wrap_order = other->m_wrap_order;
        m_routine = other->m_routine;
        LOG_INFO << "New PlantSummary (Copy) " << *this;
    }


    void PlantSummary::IPlantSummary
    (CharacteristicDescriptor* cd,
        Plant* plant,
        const PlantSet& plants,
        long int rootOrder,
        const size_t& volume_object_index,
        const ScoreboardStratum& /* stratum */,
        WrapDirection wrapping_order,
        SummaryRoutine summary_type
    )
    {
        m_characteristic_descriptor = cd;
        m_plant = plant;
        m_plants = plants;
        m_branch_order = rootOrder;
        m_volume_object_index = volume_object_index;
        m_wrap_order = wrapping_order;
        m_routine = summary_type;

        LOG_INFO << "New PlantSummary (Owned) " << *this;
    }

    std::ostream& operator<<(std::ostream& ostr, const PlantSummary& summary)
    {
        ostr << "CharacteristicDescriptor " << *(summary.m_characteristic_descriptor);
        if (summary.m_plant != __nullptr)
        {
            ostr << " {Plant:" << summary.m_plant->GetProcessName();
        }
        else
        {
            ostr << " {Plants:" << summary.m_plants.size();
        }

        summary.m_volume_object_index;
        ostr << "} {Constrained to VolumeObject:";

        if (summary.m_volume_object_index == 0) ostr << "[none]";
        else ostr << summary.m_volume_object_index;

        ostr << "} {BranchOrder:" << summary.m_branch_order
            << "} {WrapOrder:" << SprintUtility::StringOf(summary.m_wrap_order)
            << "} {SummaryRoutine:"
            << "0x" << Utility::OSTypeToString(summary.m_routine)
            << "}";
        return ostr;
    }

    const SummaryRoutineGroup PlantSummary::GetSummaryTypeArrayIndex() const
    {
        return (Utility::DetermineSummaryTypeArrayIndex(m_routine));
    }

    const std::string& PlantSummary::GetCharacteristicName() const
    {
        return GetCharacteristicDescriptor().Name;
    }

    const std::string& PlantSummary::GetUnitsString() const
    {
        return GetCharacteristicDescriptor().Units;
    }


    void PlantSummary::AddAllPlants()
    {
        // we can now ignore the specific and list of plants
        // because we're now summarising alllll of them
        m_plant = 0;
        m_plants.erase(m_plants.begin(), m_plants.end());
    }

    void PlantSummary::AddPlant(Plant* new_plant)
    {
        // nothing to add ?
        if (new_plant == 0) return;

        // no need to add a plant if we're already summarising them all
        if ((m_plants.empty()) && (m_plant == 0)) return;

        // takes care of when new_plant is already our only plant
        if (m_plant == new_plant) return;

        // if the set is currently empty, need to add both m_plant and new_plant
        if (m_plants.empty())
        {
            m_plants.insert(m_plant);
            m_plants.insert(new_plant);
            m_plant = new_plant;
        }
        else // just add the new_plant if it isn't already there
        {
            PlantSet::iterator fnd = m_plants.find(new_plant);
            if (fnd == m_plants.end())
            {
                m_plants.insert(new_plant);
                m_plant = new_plant;
            }
            else
            {
                return;
            }
        }
    }

    void PlantSummary::RemovePlant(Plant* remove_plant)
    {
        if (m_plants.empty())
        {
            if (m_plant == remove_plant)
            {
                m_plant = 0;
            }
            else
            {
                return;
            }
        }
        else // must be at least two plants in the array
        {
            PlantSet::iterator fnd = m_plants.find(remove_plant);
            if (fnd != m_plants.end())
            {
                m_plants.erase(fnd);
            }
            else
            {
                return;
            }
        }

        // logic to maintain the state of the plant and array members
        if (m_plants.size() > 1)
        {
            // get a fresh plant if the m_plant is stale
            if (m_plant == remove_plant)
            {
                m_plant = (*(m_plants.begin()));
            }
        }
        else if (m_plants.size() == 1)
        {
            m_plant = (*(m_plants.begin()));
            m_plants.erase(m_plants.begin());
        }
    }
} /* namespace rootmap */


