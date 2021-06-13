/////////////////////////////////////////////////////////////////////////////
// Name:        DimensionOrder.cpp
// Purpose:     Implementation of the DimensionOrder class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

#include "simulation/scoreboard/DimensionOrder.h"
#include "core/common/Exceptions.h"
#include "core/utility/Utility.h"

namespace rootmap
{
    DimensionOrder::DimensionOrder()
    {
        m_dimensions[0] = NoDimension;
        m_dimensions[1] = NoDimension;
        m_dimensions[2] = NoDimension;
        m_increments[0] = 1;
        m_increments[1] = 1;
        m_increments[2] = 1;
    }

    /*
    DimensionOrder::DimensionOrder(Dimension dimensions[3], signed int increments[3])
    {
        Set(dimensions, increments);
    }
    */

    DimensionOrder::~DimensionOrder()
    {
    }

    /*
    void DimensionOrder::Set(Dimension dimensions[3], signed int increments[3])
    {
        m_dimensions[0] = dimensions[0];
        m_dimensions[1] = dimensions[1];
        m_dimensions[2] = dimensions[2];
        m_increments[0] = m_increments[0];
        m_increments[1] = m_increments[1];
        m_increments[2] = m_increments[2];
    }
    */

    void DimensionOrder::Set(const std::string& spec)
    {
        int idx = 0;
        for (std::string::const_iterator ch = spec.begin();
            ch != spec.end(); ++ch)
        {
            switch (*ch)
            {
            case '+':
                m_increments[idx] = +1;
                break;
            case '-':
                m_increments[idx] = -1;
                break;
            case ',':
                ++idx;
                if (idx > 2)
                {
                    throw StringConversionException("While parsing Dimension Order specification", spec);
                }
                break;
            default:
                m_dimensions[idx] = Utility::StringToDimension(*ch);
                break;
            }
        }
    }

    void DimensionOrder::MatchDimensions
    (
        long* x_ptr, long* y_ptr, long* z_ptr,
        long* * least_hndl, long* * middle_hndl, long* * most_hndl
    )
    {
        // the first character is the least-significant, ie. innermost in a loop
        switch (m_dimensions[0])
        {
        case X: *least_hndl = x_ptr;
            break;
        case Y: *least_hndl = y_ptr;
            break;
        case Z: *least_hndl = z_ptr;
            break;
        default: break;
        }

        // second character : second loop
        switch (m_dimensions[1])
        {
        case X: *middle_hndl = x_ptr;
            break;
        case Y: *middle_hndl = y_ptr;
            break;
        case Z: *middle_hndl = z_ptr;
            break;
        default: break;
        }

        // third character : outer-most, ie. most-significant loop
        switch (m_dimensions[2])
        {
        case X: *most_hndl = x_ptr;
            break;
        case Y: *most_hndl = y_ptr;
            break;
        case Z: *most_hndl = z_ptr;
            break;
        default: break;
        }
    }

    void DimensionOrder::MatchIncrements
    (signed int& least_inc,
        signed int& middle_inc,
        signed int& most_inc)
    {
        least_inc = m_increments[0];
        middle_inc = m_increments[1];
        most_inc = m_increments[2];
    }
} /* namespace rootmap */

