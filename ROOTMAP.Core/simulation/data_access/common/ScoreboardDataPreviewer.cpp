/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardDataPreviewer.cpp
// Purpose:     Implementation of the ScoreboardDataPreviewer class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/ScoreboardDataPreviewer.h"
#include "simulation/data_access/interface/ScoreboardDAI.h"
#include "core/scoreboard/ScoreboardStratum.h"
#include "simulation/data_access/tinyxmlconfig/TXDataWriter.h"


namespace rootmap
{
    ScoreboardDataPreviewer::ScoreboardDataPreviewer()
        : DataPreviewer()
    {
    }

    ScoreboardDataPreviewer::~ScoreboardDataPreviewer()
    {
    }

    void ScoreboardDataPreviewer::Populate(ScoreboardDAI& scoreboardData)
    {
        m_XBoundaries = BoundaryValueVector(scoreboardData.getXBoundaryValueVector());
        m_YBoundaries = BoundaryValueVector(scoreboardData.getYBoundaryValueVector());
        m_ZBoundaries = BoundaryValueVector(scoreboardData.getZBoundaryValueVector());
        m_name = scoreboardData.getName();
        m_stratum = scoreboardData.getStratum();
    }
} /* namespace rootmap */

