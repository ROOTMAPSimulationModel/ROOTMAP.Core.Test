#ifndef PlantSummary_H
#define PlantSummary_H
/////////////////////////////////////////////////////////////////////////////
// Name:        PlantSummary.h
// Purpose:     Declaration of the PlantSummary class
// Created:     1994
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2002-2008 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/plant/Plant.h"
#include "simulation/process/plant/PlantFwd.h"
#include "simulation/process/common/CharacteristicDescriptor.h"
#include "core/log/Logger.h"

namespace rootmap
{
    class PlantCoordinator;
    class ScoreboardStratum;
    class Plant;

    class PlantSummary
    {
    public:
        PlantSummary();
        /**
         * PlantSummary initialiser used during the construction of
         * PlantSummaries in response to a PlantSummaryRegistration
         */
        void IPlantSummary(PlantCoordinator* plantcoordinator,
            const ScoreboardStratum& stratum,
            long branching_order,
            const size_t& volume_object_index,
            WrapDirection wrapping_order,
            SummaryRoutine summary_type,
            const std::string& name);

        /**
         * PlantSummary Initialiser for when "Duplicate"d from another summary
         */
        void IPlantSummary(PlantCoordinator* plantcoordinator,
            PlantSummary* other);

        /**
         * used by PlantSummarySupplier (a SharedAttributeSupplier)
         *
         * Takes ownership of the CharacteristicDescriptor
         * Copies the PlantSet contents
         */
        void IPlantSummary(CharacteristicDescriptor* cd,
            Plant* plant,
            const PlantSet& plants,
            long branching_order,
            const size_t& volume_object_index,
            const ScoreboardStratum& stratum,
            WrapDirection wrapping_order,
            SummaryRoutine summary_type
        );

        // ////////////////////////////////////
        // CharacteristicDescriptor fascade
        // ////////////////////////////////////
        const long GetCharacteristicID() const;
        const ScoreboardStratum& GetScoreboardStratum() const;
        const long GetCharacteristicIndex() const;
        const bool IsVisible() const;

        const std::string& GetCharacteristicName() const;
        const std::string& GetUnitsString() const;

        CharacteristicDescriptor* GetCharacteristicDescriptor();
        const CharacteristicDescriptor& GetCharacteristicDescriptor() const;

        void SetCharacteristicIndex(long new_characteristic_index);
        void SetVisible(bool visibility);

        // Plant
        const long GetNumPlants() const;
        const size_t GetVolumeObjectIndex() const;
        const long GetBranchOrder() const;
        const WrapDirection GetWrapOrder() const;
        const bool IsWrapOrderX() const;
        const bool IsWrapOrderY() const;
        const SummaryRoutine GetSummaryType() const;
        const SummaryRoutineGroup GetSummaryTypeArrayIndex() const;

        void AddAllPlants();
        void AddPlant(Plant* new_plant);
        void RemovePlant(Plant* remove_plant);
        const bool IncludesPlant(Plant* check_plant) const;


        /**
         * Before summarising for a particular summary, we need to make sure the
         * branch order and wrapping direction are suitable. The formula
         *     ((Rqstd_X + !(Rqrd_X)) && (Rqstd_Y + !(Rqrd_Y)))
         * has been determined as logical for matching Wrapping Order.
         * The truth tables for correct Wrapping matching are analysed in the file
         *   "(Miscellaneous Support Stuff):Programming Notes:Plants:SummaryWrapOrderTruthTables.ss"
         */
        const bool DoSummarise(Plant* plant, WrapDirection wrap_dir, const size_t& vo_index, long branch_order) const;


    private:
        RootMapLoggerDeclaration();

        ///
        ///
        CharacteristicDescriptor* m_characteristic_descriptor;


        ///
        /// the m_plant and m_plants members work together as :
        ///     m_plant     m_plants.size()     means
        ///         0           0               summarising all plants
        ///     non-zero        0               summarising just the one plant
        ///        n/a       non-zero           summarising a bunch of plants
        ///
        /// when summarising a bunch of plants the m_plant is used as an internal
        /// cache
        Plant* m_plant;

        ///
        /// the specific plants to summarise. Otherwise, either all or just one
        /// plant is being summarised (see m_plant description)
        PlantSet m_plants;

        ///
        ///
        long m_branch_order;

        ///
        ///
        size_t m_volume_object_index;

        ///
        ///
        WrapDirection m_wrap_order;

        ///
        ///
        SummaryRoutine m_routine;

        //     MenuHandle      m_menu;

        friend std::ostream& operator<<(std::ostream& ostr, const PlantSummary& summary);
    };

    std::ostream& operator<<(std::ostream& ostr, const PlantSummary& summary);


    // inline void PlantSummary::GetTopicName(std::string & topicName)
    // {
    //     GetCharacteristicName(topicName);
    // }

#if !defined(_DEBUG)
    inline const long PlantSummary::GetCharacteristicID() const
    {
        return m_characteristic_descriptor->id;
    }

    inline const ScoreboardStratum& PlantSummary::GetScoreboardStratum() const
    {
        return m_characteristic_descriptor->GetScoreboardStratum();
    }

    inline const long PlantSummary::GetNumPlants() const
    {
        return (m_plants.size());
    }

    inline const size_t PlantSummary::GetVolumeObjectIndex() const
    {
        return (m_volume_object_index);
    }

    inline const long PlantSummary::GetBranchOrder() const
    {
        return (m_branch_order);
    }

    inline const WrapDirection PlantSummary::GetWrapOrder() const
    {
        return (m_wrap_order);
    }

    inline const bool PlantSummary::IsWrapOrderX() const
    {
        return ((m_wrap_order & wraporderX) != 0);
    }

    inline const bool PlantSummary::IsWrapOrderY() const
    {
        return ((m_wrap_order & wraporderY) != 0);
    }

    inline const SummaryRoutine PlantSummary::GetSummaryType() const
    {
        return (m_routine);
    }

    inline const long PlantSummary::GetCharacteristicIndex() const
    {
        return m_characteristic_descriptor->ScoreboardIndex;
    }

    inline void PlantSummary::SetCharacteristicIndex(long new_characteristic_index)
    {
        m_characteristic_descriptor->ScoreboardIndex = new_characteristic_index;
    }

    inline void PlantSummary::SetVisible(bool visibility)
    {
        m_characteristic_descriptor->Visible = visibility;
    }

    inline const bool PlantSummary::IsVisible() const
    {
        return (m_characteristic_descriptor->Visible);
    }

    inline CharacteristicDescriptor* PlantSummary::GetCharacteristicDescriptor()
    {
        return m_characteristic_descriptor;
    }

    inline const CharacteristicDescriptor& PlantSummary::GetCharacteristicDescriptor() const
    {
        return (*m_characteristic_descriptor);
    }

#else

    inline const long PlantSummary::GetCharacteristicID() const
    {
        return m_characteristic_descriptor->id;
    }

    inline const ScoreboardStratum & PlantSummary::GetScoreboardStratum() const
    {
        return m_characteristic_descriptor->GetScoreboardStratum();
    }

    inline const long PlantSummary::GetNumPlants() const
    {
        return (m_plants.size());
    }

    inline const size_t PlantSummary::GetVolumeObjectIndex() const
    {
        return m_volume_object_index;
    }

    inline const long PlantSummary::GetBranchOrder() const
    {
        //if (m_branch_order<0)
        //{
        // MSA 09.10.07 Not entirely sure what's going on here - presume debugging purposes
        // int no_op = m_branch_order;
        //}
        return (m_branch_order);
    }

    inline const WrapDirection PlantSummary::GetWrapOrder() const
    {
        return (m_wrap_order);
    }

    inline const bool PlantSummary::IsWrapOrderX() const
    {
        return ((m_wrap_order&wraporderX) != 0);
    }

    inline const bool PlantSummary::IsWrapOrderY() const
    {
        return ((m_wrap_order&wraporderY) != 0);
    }

    inline const SummaryRoutine PlantSummary::GetSummaryType() const
    {
        return (m_routine);
    }

    inline const long PlantSummary::GetCharacteristicIndex() const
    {
        return m_characteristic_descriptor->ScoreboardIndex;
    }

    inline void PlantSummary::SetCharacteristicIndex(long new_characteristic_index)
    {
        m_characteristic_descriptor->ScoreboardIndex = new_characteristic_index;
    }

    inline void PlantSummary::SetVisible(bool visibility)
    {
        m_characteristic_descriptor->Visible = visibility;
    }

    inline const bool PlantSummary::IsVisible() const
    {
        return (m_characteristic_descriptor->Visible);
    }

    inline CharacteristicDescriptor * PlantSummary::GetCharacteristicDescriptor()
    {
        return m_characteristic_descriptor;
    }

    inline const CharacteristicDescriptor & PlantSummary::GetCharacteristicDescriptor() const
    {
        return (*m_characteristic_descriptor);
    }

#endif // #if !defined(_DEBUG)

    inline const bool PlantSummary::IncludesPlant(Plant* check_plant) const
    {
        if (check_plant == m_plant) return true;
        if (m_plants.empty() && (__nullptr == m_plant)) return true;
        return m_plants.find(check_plant) != m_plants.end();
    }

    inline const bool PlantSummary::DoSummarise(Plant* plant, WrapDirection /* wrap_dir */, const size_t& vo_index, long branch_order) const
    {
        if (!IncludesPlant(plant)) return false;
        if (vo_index >= 0 && vo_index != m_volume_object_index) return false;
        if ((m_branch_order >= 0) && (branch_order != m_branch_order)) return false;
        // NOTE:
        // Wrapping should cause decisions to be made based on position, not just
        // if //(wrap requested==wrap required)
        return true;
    }

    typedef std::vector<PlantSummary *> PlantSummaryArray;
    typedef PlantSummaryArray::iterator PlantSummaryArrayIterator;
} /* namespace rootmap */

#endif // #ifndef PlantSummary_H
