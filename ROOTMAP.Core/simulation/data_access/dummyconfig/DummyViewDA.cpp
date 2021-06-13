/////////////////////////////////////////////////////////////////////////////
// Name:        DummyViewDA.cpp
// Purpose:     Implementation of the DummyViewDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "DummyViewDA.h"


namespace rootmap
{
    DummyViewDA::DummyViewDA()
    {
    }

    DummyViewDA::~DummyViewDA()
    {
    }

    ViewDirection DummyViewDA::GetViewDirection() const
    {
        return vFront;
    }

    DoubleCoordinate DummyViewDA::GetReferenceIndex() const
    {
        //double value =  2            // 2 scroll units
        //                 / GetScale()   //
        //                 * 6;           //  scroll pixels/unit
        //return (DoubleCoordinate(-value,-value,-value));
        return (DoubleCoordinate(0.0, 0.0, 0.0));
    }

    double DummyViewDA::GetScale() const
    {
        return 6.543;
    }

    double DummyViewDA::GetZoomRatio() const
    {
        return 1.0;
    }

    bool DummyViewDA::DoesRepeat() const
    {
        return false;
    }

    bool DummyViewDA::DoesWrap() const
    {
        return true;
    }

    bool DummyViewDA::DoesBoxes() const
    {
        return true;
    }

    bool DummyViewDA::DoesBoundaries() const
    {
        return true;
    }

    bool DummyViewDA::DoesBoxColours() const
    {
        return true;
    }

    const double DummyViewDA::GetRootRadiusMultiplier() const
    {
        return 1.0;
    }

    bool DummyViewDA::DoesCylinders() const
    {
        return false;
    }

    bool DummyViewDA::DoesSpheres() const
    {
        return false;
    }

    bool DummyViewDA::DoesCones() const
    {
        return false;
    }

    size_t DummyViewDA::GetStacksAndSlices() const
    {
        return 0;
    }

    int DummyViewDA::GetRootColourationMode() const
    {
        return 1;
    }

    float DummyViewDA::GetBaseRootRf() const { return 0.0F; }
    float DummyViewDA::GetBaseRootGf() const { return 0.0F; }
    float DummyViewDA::GetBaseRootBf() const { return 0.0F; }

    ScoreboardFlags DummyViewDA::GetScoreboards() const
    {
        ScoreboardFlags sf(0);
        sf.set(2, true);

        return (sf);
    }

    std::vector<std::string> DummyViewDA::GetProcesses() const
    {
        std::vector<std::string> procs;
        std::string proc = "DummyProcessNothing";
        procs.push_back(proc);

        return procs;
    }

    ViewDAI::CharacteristicColourDataArray DummyViewDA::GetCharacteristicColourInfo() const
    {
        CharacteristicColourData cyanData;
        cyanData.colourRangeMin = 0;
        cyanData.colourRangeMax = colourElementMaxValue;
        cyanData.characteristicRangeMin = 0.0;
        cyanData.characteristicRangeMax = 100.0;
        cyanData.processName = "DummyProcessWith2Characteristics";
        cyanData.characteristicName = "Dummy Characteristic 1";
        cyanData.characteristicIndex = -1;
        cyanData.characteristicID = -1;

        CharacteristicColourData magentaData;
        magentaData.colourRangeMin = 0;
        magentaData.colourRangeMax = colourElementMaxValue;
        magentaData.characteristicRangeMin = 0.0;
        magentaData.characteristicRangeMax = 1.0;
        magentaData.processName = "DummyProcessWith2Characteristics";
        magentaData.characteristicName = "Dummy Characteristic 2";
        magentaData.characteristicIndex = -1;
        magentaData.characteristicID = -1;

        CharacteristicColourData yellowData;
        yellowData.colourRangeMin = 0;
        yellowData.colourRangeMax = colourElementMaxValue;
        yellowData.characteristicRangeMin = 0.0;
        yellowData.characteristicRangeMax = 100.0;
        yellowData.processName = "DummyProcessWith2Characteristics";
        yellowData.characteristicName = "Dummy Characteristic 1";
        yellowData.characteristicIndex = -1;
        yellowData.characteristicID = -1;

        CharacteristicColourDataArray colours;
        colours.push_back(cyanData);
        colours.push_back(magentaData);
        colours.push_back(yellowData);

        return colours;
    }
} /* namespace rootmap */

