#ifndef VolumeObjectDataPreviewer_H
#define VolumeObjectDataPreviewer_H
/////////////////////////////////////////////////////////////////////////////
// Name:        VolumeObjectDataPreviewer.h
// Purpose:     Declaration of the VolumeObjectDataPreviewer class
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/DataPreviewer.h"
#include <vector>
#include "wx/string.h"
#include "simulation/process/modules/VolumeObject.h"
#include "simulation/process/modules/BoundingCylinder.h"
#include "simulation/process/modules/BoundingRectangularPrism.h"

namespace rootmap
{
    class VolumeObjectDataPreviewer : public DataPreviewer
    {
        friend class RmCWVolumeObjectPage;

    public:

        VolumeObjectDataPreviewer();
        virtual ~VolumeObjectDataPreviewer();

        void ResetVolumeObjectIndex();
        const size_t volumeObjectCount() const;
        void ChangesHaveBeenMade();
        const bool DataChanged() const;

        void AddBoundingCylinder(BoundingCylinder* bc);
        void AddBoundingRectangularPrism(BoundingRectangularPrism* brp);
        void RemoveLastBoundingCylinder();
        void RemoveLastBoundingRectangularPrism();
        const bool HasMoreVolumeObjects() const;
        VolumeObject* GetNextVolumeObject();

    protected:

        std::vector<BoundingCylinder*> m_boundingCylinders;
        std::vector<BoundingRectangularPrism*> m_boundingRectangularPrisms;

    private:

        size_t m_nextBoundingCylinderIndex;
        size_t m_nextBoundingRectangularPrismIndex;
        bool m_dataChanged;
    };

    inline void VolumeObjectDataPreviewer::ResetVolumeObjectIndex()
    {
        m_nextBoundingCylinderIndex = 0;
        m_nextBoundingRectangularPrismIndex = 0;
    }

    inline const size_t VolumeObjectDataPreviewer::volumeObjectCount() const
    {
        return (m_boundingCylinders.size() + m_boundingRectangularPrisms.size());
    }

    inline const bool VolumeObjectDataPreviewer::HasMoreVolumeObjects() const
    {
        return (m_nextBoundingCylinderIndex < m_boundingCylinders.size()) || (m_nextBoundingRectangularPrismIndex < m_boundingRectangularPrisms.size());
    }

    inline void VolumeObjectDataPreviewer::RemoveLastBoundingCylinder()
    {
        std::vector<BoundingCylinder*>::const_iterator i = m_boundingCylinders.end() - 1;
        delete (*i);
        m_boundingCylinders.erase(i);
    }

    inline void VolumeObjectDataPreviewer::RemoveLastBoundingRectangularPrism()
    {
        std::vector<BoundingRectangularPrism*>::const_iterator i = m_boundingRectangularPrisms.end() - 1;
        delete (*i);
        m_boundingRectangularPrisms.erase(i);
    }

    inline void VolumeObjectDataPreviewer::ChangesHaveBeenMade()
    {
        m_dataChanged = true;
    }

    inline const bool VolumeObjectDataPreviewer::DataChanged() const
    {
        return m_dataChanged;
    }
} /* namespace rootmap */


#endif // #ifndef VolumeObjectDataPreviewer_H
