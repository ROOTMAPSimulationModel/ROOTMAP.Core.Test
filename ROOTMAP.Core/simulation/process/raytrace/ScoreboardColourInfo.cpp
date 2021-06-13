/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardColourInfo.cpp
// Purpose:     Implementation of the ScoreboardColourInfo class
// Created:     29-06-2009 15:51:18
// Author:      RvH
// $Date$
// $Revision$
// Copyright:   ©2002-2009 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/raytrace/ScoreboardColourInfo.h"


namespace rootmap
{
    ScoreboardColourInfo::ScoreboardColourInfo()
    {
    }

    ScoreboardColourInfo::ScoreboardColourInfo(const CharacteristicColours_t& rhs)
    {
        for (int i = 0; i < colourElementsMax; ++i)
        {
            theData.push_back(rhs[i]);
        }
    }

    void ScoreboardColourInfo::push_back(const CharacteristicColourInfo& cci)
    {
        theData.push_back(cci);
    }

    CharacteristicColourInfo& ScoreboardColourInfo::at(const size_t& index)
    {
        return theData.at(index);
    }

    const size_t ScoreboardColourInfo::size() const
    {
        return theData.size();
    }

    ScoreboardColourInfo::~ScoreboardColourInfo()
    {
    }
} /* namespace rootmap */

