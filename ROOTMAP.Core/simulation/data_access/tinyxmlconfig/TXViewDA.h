#ifndef TXViewDA_H
#define TXViewDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        TXViewDA.h
// Purpose:     Declaration of the TXViewDA class
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
    class TXViewDA
        : public ViewDAI
    {
    public:
        TXViewDA(ViewDirection viewDir,
            const DoubleCoordinate& refIndex,
            double scale,
            bool doesRepeat,
            bool doesWrap,
            bool doesBoxes,
            bool doesBoundaries,
            bool doesBoxColours,
            //MSA new stuff
            const double rootRadiusMultiplier,
            bool doesCylinders,
            bool doesSpheres,
            bool doesCones,
            const size_t GLStacksAndSlices,
            const int rootColouration,
            const float baseRootRf,
            const float baseRootGf,
            const float baseRootBf,
            const ScoreboardFlags& scoreboardFlags,
            const std::vector<std::string>& processNames,
            const CharacteristicColourDataArray& characteristicColourData);
        ~TXViewDA();

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

        /** MSA new stuff

         * @return
         */
        const double GetRootRadiusMultiplier() const;

        /**
         * @return
         */
        bool DoesCylinders() const;

        /** MSA new stuff
         * @return
         */
        bool DoesSpheres() const;

        /** MSA new stuff
         * @return
         */
        bool DoesCones() const;

        /** MSA new stuff
         * @return
         */
        int GetRootColourationMode() const;

        /** MSA new stuff
         * @return
         */
        size_t GetStacksAndSlices() const;

        /** MSA new stuff
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

        ///
        ///
        ViewDirection m_viewDirection;

        ///
        ///
        DoubleCoordinate m_referenceIndex;

        ///
        ///
        double m_scale;

        ///
        ///
        bool m_doesRepeat;

        ///
        ///
        bool m_doesWrap;

        ///
        ///
        bool m_doesBoxes;

        ///
        ///
        bool m_doesBoundaries;

        ///
        ///
        bool m_doesBoxColours;

        ///
        /// MSA new config variables
        double m_rootRadiusMultiplier;

        ///
        ///
        bool m_doesCylinders;

        ///
        ///
        bool m_doesCones;

        ///
        ///
        bool m_doesSpheres;

        ///
        ///
        size_t m_stacksAndSlices;

        ///
        ///
        int m_rootColourationMode;

        ///
        ///
        float m_baseRootRf;
        float m_baseRootGf;
        float m_baseRootBf;

        ///
        ///
        ScoreboardFlags m_scoreboardFlags;

        ///
        ///
        std::vector<std::string> m_processNames;

        ///
        ///
        CharacteristicColourDataArray m_colourData;
    }; // class TXViewDA
} /* namespace rootmap */

#endif // #ifndef TXViewDA_H
