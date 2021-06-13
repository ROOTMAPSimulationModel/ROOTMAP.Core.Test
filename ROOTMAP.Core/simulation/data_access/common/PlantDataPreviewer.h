#ifndef PlantDataPreviewer_H
#define PlantDataPreviewer_H
/////////////////////////////////////////////////////////////////////////////
// Name:        PlantDataPreviewer.h
// Purpose:     Declaration of the PlantDataPreviewer class
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/DataPreviewer.h"
#include "simulation/data_access/interface/PlantDAI.h"
#include "core/common/DoubleCoordinates.h"

namespace rootmap
{
    class PlantType;

    struct TogglablePlantDAI
    {
        ~TogglablePlantDAI()
        {
            delete plantDAI;
            plantDAI = __nullptr;
        }

    public:
        PlantDAI* plantDAI;
        bool inUse = true;
    };

    class PlantDataPreviewer : public DataPreviewer
    {
        friend class RmCWPlantPage;

    public:

        PlantDataPreviewer();

        virtual ~PlantDataPreviewer();

        // These methods take ownership of a TogglablePlantDAIs and PlantTypes constructed on the heap.

        virtual void AddPlant(TogglablePlantDAI* plantData);

        virtual void AddPlantType(PlantType* plantType);

        void ResetPlantIndex();
        const size_t plantCount() const;
        void ChangesHaveBeenMade();
        const bool DataChanged() const;
        void RemoveLastPlant();
        const bool HasMorePlants() const;
        TogglablePlantDAI* GetNextPlant();
        void ReplacePlant(TogglablePlantDAI* oldPlant, TogglablePlantDAI* newPlant);

    protected:
        std::string m_typeName;
        DoubleCoordinate m_origin;
        DoubleCoordinate m_seedLocation;
        ProcessTime_t m_seedingTime;
        std::vector<TogglablePlantDAI*> m_plants;
        std::vector<PlantType*> m_plantTypes;

        PossibleAxis* m_possibleSeminals;
        PossibleAxis* m_possibleNodals;
    private:
        size_t m_nextPlantIndex;
        bool m_dataChanged;
    };

    inline void PlantDataPreviewer::ResetPlantIndex()
    {
        m_nextPlantIndex = 0;
    }

    inline const size_t PlantDataPreviewer::plantCount() const
    {
        return (m_plants.size());
    }

    inline const bool PlantDataPreviewer::HasMorePlants() const
    {
        return (m_nextPlantIndex < m_plants.size());
    }

    inline void PlantDataPreviewer::RemoveLastPlant()
    {
        std::vector<TogglablePlantDAI*>::const_iterator i = m_plants.end() - 1;
        delete (*i);
        m_plants.erase(i);
    }

    inline void PlantDataPreviewer::ChangesHaveBeenMade()
    {
        m_dataChanged = true;
    }

    inline const bool PlantDataPreviewer::DataChanged() const
    {
        return m_dataChanged;
    }
} /* namespace rootmap */

#endif // #ifndef PlantDataPreviewer_H
