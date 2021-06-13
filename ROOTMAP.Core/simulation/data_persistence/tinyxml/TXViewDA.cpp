#include "BoostPrecomp.h"

/////////////////////////////////////////////////////////////////////////////
// Name:        TXViewDA.cpp
// Purpose:     Implementation of the TXViewDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "TXViewDA.h"
#include "simulation/common/BoostPrecomp.h"

namespace rootmap
{

    TXViewDA:: TXViewDA
    (   ViewDirection viewDir,
        const DoubleCoordinate & refIndex,
        double_cs scale,
        bool doesRepeat,
        bool doesWrap,
        bool doesBoxes,
        bool doesBoxColours,
        const ScoreboardFlags & scoreboardFlags,
        const std::vector<RmString> & processNames,
        const CharacteristicColourDataArray & characteristicColourData)
    : m_viewDirection(viewDir)
    , m_referenceIndex(refIndex)
    , m_scale(scale)
    , m_doesRepeat(doesRepeat)
    , m_doesWrap(doesWrap)
    , m_doesBoundaries(true)
    , m_doesBoxes(doesBoxes)
    , m_doesBoxColours(doesBoxColours)
    , m_scoreboardFlags(scoreboardFlags)
    , m_processNames(processNames)
    , m_colourData(characteristicColourData)
    {
    }

    TXViewDA::~TXViewDA()
    {
    }

    ViewDirection TXViewDA::GetViewDirection() const
    {
        return m_viewDirection;
    }

    DoubleCoordinate TXViewDA::GetReferenceIndex() const
    {
        return m_referenceIndex;
    }

    double_cs TXViewDA::GetScale() const
    {
        return m_scale;
    }

    bool TXViewDA::DoesRepeat() const
    {
        return m_doesRepeat;
    }

    bool TXViewDA::DoesWrap() const
    {
        return m_doesWrap;
    }

    bool TXViewDA::DoesBoxes() const
    {
        return m_doesBoxes;
    }

    bool TXViewDA::DoesBoundaries() const
    {
        return m_doesBoundaries;
    }

    bool TXViewDA::DoesBoxColours() const
    {
        return m_doesBoxColours;
    }

    ScoreboardFlags TXViewDA::GetScoreboards() const
    {
        return (m_scoreboardFlags);
    }

    std::vector<RmString> TXViewDA::GetProcesses() const
    {
        return m_processNames;
    }

    ViewDAI::CharacteristicColourDataArray TXViewDA::GetCharacteristicColourInfo() const
    {
        return m_colourData;
    }
} // namespace rootmap
