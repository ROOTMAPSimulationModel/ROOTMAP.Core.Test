/////////////////////////////////////////////////////////////////////////////
// Name:        VolumeObjectDataPreviewer.cpp
// Purpose:     Implementation of the VolumeObjectDataPreviewer class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/VolumeObjectDataPreviewer.h"

namespace rootmap
{
    VolumeObjectDataPreviewer::VolumeObjectDataPreviewer()
        : m_nextBoundingCylinderIndex(0)
        , m_nextBoundingRectangularPrismIndex(0)
    {
    }

    VolumeObjectDataPreviewer::~VolumeObjectDataPreviewer()
    {
        for (std::vector<BoundingCylinder*>::iterator iter = m_boundingCylinders.begin(); iter != m_boundingCylinders.end(); ++iter)
        {
            delete (*iter);
        }
        for (std::vector<BoundingRectangularPrism*>::iterator iter = m_boundingRectangularPrisms.begin(); iter != m_boundingRectangularPrisms.end(); ++iter)
        {
            delete (*iter);
        }
    }

    void VolumeObjectDataPreviewer::AddBoundingCylinder(BoundingCylinder* bc)
    {
        m_boundingCylinders.push_back(bc);
    }

    void VolumeObjectDataPreviewer::AddBoundingRectangularPrism(BoundingRectangularPrism* brp)
    {
        m_boundingRectangularPrisms.push_back(brp);
    }

    VolumeObject* VolumeObjectDataPreviewer::GetNextVolumeObject()
    {
        if (m_nextBoundingCylinderIndex < m_boundingCylinders.size())
        {
            return m_boundingCylinders[m_nextBoundingCylinderIndex++];
        }
        if (m_nextBoundingRectangularPrismIndex < m_boundingRectangularPrisms.size())
        {
            return m_boundingRectangularPrisms[m_nextBoundingRectangularPrismIndex++];
        }
        return __nullptr;
    }
} /* namespace rootmap */

