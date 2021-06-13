/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessDataPreviewer.cpp
// Purpose:     Implementation of the ProcessDataPreviewer class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/ProcessDataPreviewer.h"

namespace rootmap
{
    ProcessDataPreviewer::ProcessDataPreviewer(const std::string& processName)
        : m_processName(processName)
    {
    }

    ProcessDataPreviewer::~ProcessDataPreviewer()
    {
    }

    bool sort(const ProcessDataPreviewer::CharacteristicSummary* lhs, const ProcessDataPreviewer::CharacteristicSummary* rhs)
    {
        return ProcessDataPreviewer::CharacteristicSummary::lessthanAlphabetic(lhs->m_name, rhs->m_name);
    }

    void ProcessDataPreviewer::AddCharacteristicSummary(const std::string& name, const double& min, const double& value, const double& max)
    {
        m_characteristicSummaries.push_back(new CharacteristicSummary(name, min, value, max));

        std::sort(m_characteristicSummaries.begin(), m_characteristicSummaries.end(), sort);
    }
} /* namespace rootmap */

