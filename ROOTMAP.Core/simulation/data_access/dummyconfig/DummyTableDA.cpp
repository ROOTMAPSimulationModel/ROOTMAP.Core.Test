/////////////////////////////////////////////////////////////////////////////
// Name:        DummyTableDA.cpp
// Purpose:     Implementation of the DummyTableDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-10 06:43:25 +0800 (Fri, 10 Jul 2009) $
// $Revision: 69 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/dummyconfig/DummyTableDA.h"


namespace rootmap
{
    DummyTableDA::DummyTableDA()
    {
    }

    DummyTableDA::~DummyTableDA()
    {
    }

    std::string DummyTableDA::getProcessName() const
    {
        return "DummyProcessWith2Characteristics";
    }

    std::string DummyTableDA::getCharacteristicName() const
    {
        return "Dummy Characteristic 2";
    }

    ViewDirection DummyTableDA::getViewDirection() const
    {
        return vFront;
    }

    long DummyTableDA::getLayer() const
    {
        return 1;
    }

    ScoreboardStratum DummyTableDA::getScoreboardStratum() const
    {
        return ScoreboardStratum(ScoreboardStratum::Soil);
    }
} /* namespace rootmap */

