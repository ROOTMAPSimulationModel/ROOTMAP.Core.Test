/////////////////////////////////////////////////////////////////////////////
// Name:        DummyOutputRuleDA.pp
// Purpose:     Definition of the DummyOutputRuleDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-10 06:43:25 +0800 (Fri, 10 Jul 2009) $
// $Revision: 69 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/dummyconfig/DummyOutputRuleDA.h"
#include "simulation/process/interprocess_communication/PostOffice.h"
#include "core/common/RmAssert.h"


namespace rootmap
{
    std::string DummyOutputRuleDA::GetProcess() const
    {
        return "Dummy Process";
    }


    std::string DummyOutputRuleDA::GetType() const
    {
        return "ScoreboardData";
    }


    std::string DummyOutputRuleDA::GetName() const
    {
        return "Dummy Characteristic";
    }


    ScoreboardStratum DummyOutputRuleDA::GetStratum() const
    {
        return ScoreboardStratum(ScoreboardStratum::Soil);
    }


    std::string DummyOutputRuleDA::GetFileName() const
    {
        return "DummyCharacteristic_%Y%m%d-%H%M%S.txt";
    }

    std::string DummyOutputRuleDA::GetDirectory() const
    {
        return "C:\\Temp";
    }


    std::string DummyOutputRuleDA::GetSpecification1() const
    {
        return "X,-Y,Z";
    }


    std::string DummyOutputRuleDA::GetSpecification2() const
    {
        return "";
    }

    const OutputRuleDAI::BaseWhen& DummyOutputRuleDA::GetWhen() const
    {
        return m_when;
    }

    std::string DummyOutputRuleDA::GetReopenStrategy() const
    {
        return "append";
    }


    DummyOutputRuleDA::~DummyOutputRuleDA()
    {
    }

    DummyOutputRuleDA::DummyOutputRuleDA()
    {
        m_when.m_count = 10;
        m_when.m_interval = PostOffice::CalculateTimeSpan(0, 0, 7, 0, 0, 0);
        m_when.m_initialTime = PostOffice::CalculateTimeSpan(0, 0, 0, 12, 0, 0);
    }
} /* namespace rootmap */


