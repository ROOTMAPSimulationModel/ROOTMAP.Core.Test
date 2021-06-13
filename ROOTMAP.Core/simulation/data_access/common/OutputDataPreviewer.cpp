/////////////////////////////////////////////////////////////////////////////
// Name:        OutputDataPreviewer.cpp
// Purpose:     Implementation of the OutputDataPreviewer class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/OutputDataPreviewer.h"

namespace rootmap
{
    OutputDataPreviewer::OutputDataPreviewer(PostOffice* po)
        : m_nextOutputRuleIndex(0)
        , m_po(po)
    {
    }

    OutputDataPreviewer::~OutputDataPreviewer()
    {
        for (std::vector<OutputRuleDAI*>::iterator iter = m_outputRules.begin(); iter != m_outputRules.end(); ++iter)
        {
            delete (*iter);
        }
    }

    void OutputDataPreviewer::AddOutputRule(OutputRuleDAI* or )
    {
        m_outputRules.push_back(or );
    }

    OutputRuleDAI* OutputDataPreviewer::GetNextOutputRule()
    {
        if (m_nextOutputRuleIndex < m_outputRules.size())
        {
            return m_outputRules[m_nextOutputRuleIndex++];
        }
        return __nullptr;
    }
} /* namespace rootmap */

