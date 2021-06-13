#ifndef DummyViewDA_H
#define DummyViewDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DummyViewDA.h
// Purpose:     Declaration of the DummyViewDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/common/DoubleCoordinates.h"
#include "core/common/Types.h"
#include "simulation/data_access/interface/ViewDAI.h"

namespace rootmap
{
    class DummyViewDA
        : public ViewDAI
    {
    public:
        DummyViewDA();
        ~DummyViewDA();

        /**
         * @return
         */
        ViewDirection GetViewDirection() const;

        /**
         * @return
         */
        DoubleCoordinate GetReferenceIndex() const;

        /**
         * @return
         */
        double GetScale() const;

        /**
         * @return
         */
        double GetZoomRatio() const;

        /**
         * @return
         */
        bool DoesRepeat() const;

        /**
         * @return
         */
        bool DoesWrap() const;

        /**
         * @return
         */
        bool DoesBoxes() const;

        /**
         * @return
         */
        bool DoesBoundaries() const;

        /**
         * @return
         */
        bool DoesBoxColours() const;

        /**
         * @return
         */
        const double GetRootRadiusMultiplier() const;

        /**
         * @return
         */
        bool DoesCylinders() const;

        /**
         * @return
         */
        bool DoesSpheres() const;

        /**
         * @return
         */
        bool DoesCones() const;

        /**
         * @return
         */
        size_t GetStacksAndSlices() const;

        /**
         * @return
         */
        int GetRootColourationMode() const;

        /**
         * @return
         */
        float GetBaseRootRf() const;
        float GetBaseRootGf() const;
        float GetBaseRootBf() const;

        /**
         * @return
         */
        ScoreboardFlags GetScoreboards() const;

        /**
         * @return
         */
        std::vector<std::string> GetProcesses() const;

        /**
         * @return
         */
        CharacteristicColourDataArray GetCharacteristicColourInfo() const;
    private:
    }; // class DummyViewDA
} /* namespace rootmap */

#endif // #ifndef DummyViewDA_H
