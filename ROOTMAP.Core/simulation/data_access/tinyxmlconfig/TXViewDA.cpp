/////////////////////////////////////////////////////////////////////////////
// Name:        TXViewDA.cpp
// Purpose:     Implementation of the TXViewDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "TXViewDA.h"


namespace rootmap
{
    TXViewDA::TXViewDA
    (ViewDirection viewDir,
        const DoubleCoordinate& refIndex,
        double scale,
        bool doesRepeat,
        bool doesWrap,
        bool doesBoxes,
        bool doesBoundaries,
        bool doesBoxColours,
        const double rootRadiusMultiplier,
        bool doesCylinders,
        bool doesSpheres,
        bool doesCones,
        const size_t GLStacksAndSlices,
        const int rootColourationMode,
        const float baseRootRf,
        const float baseRootGf,
        const float baseRootBf,
        const ScoreboardFlags& scoreboardFlags,
        const std::vector<std::string>& processNames,
        const CharacteristicColourDataArray& characteristicColourData)
        : m_viewDirection(viewDir)
        , m_referenceIndex(refIndex)
        , m_scale(scale)
        , m_doesRepeat(doesRepeat)
        , m_doesWrap(doesWrap)
        , m_doesBoxes(doesBoxes)
        , m_doesBoundaries(doesBoundaries)
        , m_doesBoxColours(doesBoxColours)
        , m_rootRadiusMultiplier(rootRadiusMultiplier)
        , m_doesCylinders(doesCylinders)
        , m_doesSpheres(doesSpheres)
        , m_doesCones(doesCones)
        , m_stacksAndSlices(GLStacksAndSlices)
        , m_rootColourationMode(rootColourationMode)
        , m_baseRootRf(baseRootRf)
        , m_baseRootGf(baseRootGf)
        , m_baseRootBf(baseRootBf)
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

    double TXViewDA::GetScale() const
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

    // MSA new stuff
    // TODO inline?

    const double TXViewDA::GetRootRadiusMultiplier() const
    {
        return m_rootRadiusMultiplier;
    }

    bool TXViewDA::DoesCylinders() const
    {
        return m_doesCylinders;
    }

    bool TXViewDA::DoesCones() const
    {
        return m_doesCones;
    }

    bool TXViewDA::DoesSpheres() const
    {
        return m_doesSpheres;
    }

    size_t TXViewDA::GetStacksAndSlices() const
    {
        return m_stacksAndSlices;
    }

    int TXViewDA::GetRootColourationMode() const
    {
        return m_rootColourationMode;
    }

    float TXViewDA::GetBaseRootRf() const
    {
        return m_baseRootRf;
    }

    float TXViewDA::GetBaseRootGf() const
    {
        return m_baseRootGf;
    }

    float TXViewDA::GetBaseRootBf() const
    {
        return m_baseRootBf;
    }

    ScoreboardFlags TXViewDA::GetScoreboards() const
    {
        return (m_scoreboardFlags);
    }

    std::vector<std::string> TXViewDA::GetProcesses() const
    {
        return m_processNames;
    }

    ViewDAI::CharacteristicColourDataArray TXViewDA::GetCharacteristicColourInfo() const
    {
        return m_colourData;
    }
} /* namespace rootmap */

