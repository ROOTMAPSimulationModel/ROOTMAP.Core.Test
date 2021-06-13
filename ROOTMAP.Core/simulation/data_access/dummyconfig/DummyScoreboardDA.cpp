/////////////////////////////////////////////////////////////////////////////
// Name:        DummyScoreboardDA.cpp
// Purpose:     Definition of the DummyScoreboardDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-10 06:43:25 +0800 (Fri, 10 Jul 2009) $
// $Revision: 69 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/dummyconfig/DummyScoreboardDA.h"


namespace rootmap
{
    std::string DummyScoreboardDA::getName() const
    {
        return "Soil";
    }


    ScoreboardStratum DummyScoreboardDA::getStratum() const
    {
        return ScoreboardStratum(ScoreboardStratum::Soil);
    }


    BoundaryValueVector DummyScoreboardDA::getXBoundaryValueVector() const
    {
        BoundaryValueVector xboundaries;
        xboundaries.push_back(0.0);
        xboundaries.push_back(5.0);
        xboundaries.push_back(8.0);
        xboundaries.push_back(10.0);
        xboundaries.push_back(12.0);
        xboundaries.push_back(15.0);
        xboundaries.push_back(20.0);

        return xboundaries;
    }


    BoundaryValueVector DummyScoreboardDA::getYBoundaryValueVector() const
    {
        BoundaryValueVector yboundaries;
        yboundaries.push_back(0.0);
        yboundaries.push_back(2.0);
        yboundaries.push_back(5.0);
        yboundaries.push_back(10.0);
        yboundaries.push_back(15.0);
        yboundaries.push_back(18.0);
        yboundaries.push_back(20.0);

        return yboundaries;
    }


    BoundaryValueVector DummyScoreboardDA::getZBoundaryValueVector() const
    {
        BoundaryValueVector zboundaries;
        zboundaries.push_back(0.0);
        zboundaries.push_back(2.0);
        zboundaries.push_back(5.0);
        zboundaries.push_back(9.0);
        zboundaries.push_back(14.0);
        zboundaries.push_back(20.0);
        zboundaries.push_back(27.0);
        zboundaries.push_back(35.0);
        zboundaries.push_back(44.0);
        zboundaries.push_back(54.0);
        zboundaries.push_back(65.0);

        return zboundaries;
    }


    DummyScoreboardDA::~DummyScoreboardDA()
    {
    }


    DummyScoreboardDA::DummyScoreboardDA()
    {
    }
} /* namespace rootmap */

