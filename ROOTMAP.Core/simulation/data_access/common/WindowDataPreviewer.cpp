/////////////////////////////////////////////////////////////////////////////
// Name:        WindowDataPreviewer.cpp
// Purpose:     Implementation of the WindowDataPreviewer class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/WindowDataPreviewer.h"


namespace rootmap
{
    WindowDataPreviewer::WindowDataPreviewer()
        : m_nextTableIndex(0)
        , m_next2DViewIndex(0)
        , m_next3DViewIndex(0)
    {
    }

    WindowDataPreviewer::~WindowDataPreviewer()
    {
        for (std::vector<Table*>::iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
        {
            delete (*iter);
        }
        for (std::vector<TwoDView*>::iterator iter = m_2DViews.begin(); iter != m_2DViews.end(); ++iter)
        {
            delete (*iter);
        }
        for (std::vector<ThreeDView*>::iterator iter = m_3DViews.begin(); iter != m_3DViews.end(); ++iter)
        {
            delete (*iter);
        }
    }

    void WindowDataPreviewer::AddTable(Table* tbl)
    {
        m_tables.push_back(tbl);
    }

    void WindowDataPreviewer::Add2DView(TwoDView* twoD)
    {
        m_2DViews.push_back(twoD);
    }

    void WindowDataPreviewer::Add3DView(ThreeDView* threeD)
    {
        m_3DViews.push_back(threeD);
    }

    WindowDataPreviewer::Window* WindowDataPreviewer::GetNextWDPWindow()
    {
        if (m_nextTableIndex < m_tables.size())
        {
            return m_tables[m_nextTableIndex++];
        }
        if (m_next2DViewIndex < m_2DViews.size())
        {
            return m_2DViews[m_next2DViewIndex++];
        }
        if (m_next3DViewIndex < m_3DViews.size())
        {
            return m_3DViews[m_next3DViewIndex++];
        }
        return __nullptr;
    }

    void WindowDataPreviewer::Table::dummy()
    {
    }

    void WindowDataPreviewer::TwoDView::dummy()
    {
    }

    void WindowDataPreviewer::ThreeDView::dummy()
    {
    }
} /* namespace rootmap */

