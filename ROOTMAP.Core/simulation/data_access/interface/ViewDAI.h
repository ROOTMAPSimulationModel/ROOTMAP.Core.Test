#ifndef ViewDAI_H
#define ViewDAI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ViewDAI.h
// Purpose:     Declaration of the ViewDAI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

#include "core/common/DoubleCoordinates.h"

#include "simulation/common/Types.h"
#include "simulation/scoreboard/ScoreboardFlags.h"

#include <vector>

namespace rootmap
{
    class Process;

    class ViewDAI
    {
    public:

        /**
         * @return
         */
        virtual ViewDirection GetViewDirection() const = 0;

        /**
         * @return
         */
        virtual DoubleCoordinate GetReferenceIndex() const = 0;

        /**
         * @return
         */
        virtual double GetScale() const = 0;

        /**
         * @return
         */
        virtual bool DoesRepeat() const = 0;

        /**
         * @return
         */
        virtual bool DoesWrap() const = 0;

        /**
         * @return
         */
        virtual bool DoesBoxes() const = 0;

        /**
         * @return
         */
        virtual bool DoesBoundaries() const = 0;

        /**
         * @return
         */
        virtual bool DoesBoxColours() const = 0;

        /** MSA new stuff
         * @return
         */
        virtual const double GetRootRadiusMultiplier() const = 0;

        /**
         * @return
         */
        virtual bool DoesCylinders() const = 0;

        /**
         * @return
         */
        virtual bool DoesSpheres() const = 0;

        /**
         * @return
         */
        virtual bool DoesCones() const = 0;

        /**
         * @return
         */
        virtual size_t GetStacksAndSlices() const = 0;

        /**
         * @return
         */
        virtual int GetRootColourationMode() const = 0;

        /** MSA new stuff
         * @return
         */
        virtual float GetBaseRootRf() const = 0;
        virtual float GetBaseRootGf() const = 0;
        virtual float GetBaseRootBf() const = 0;

        /**
         * @return
         */
        virtual ScoreboardFlags GetScoreboards() const = 0;

        /**
         * @return
         */
        virtual std::vector<std::string> GetProcesses() const = 0;

        /**
         * It will be interesting to see how this embedded struct fares
         *
         * The purpose here is that the characteristicIndex and characteristicID
         * of the normally-used CharacteristicColourInfo are not available -
         * they are both dynamically determined during simulation initialisation
         *
         * Thus, the values stored to enable persistence are the process and
         * characteristic name, both of which should be unique. Either the
         * Engine or ViewInformation will need to be responsible for retrieving
         * the index and id given those.
         */
        struct CharacteristicColourData
        {
            ColourElementValue_t colourRangeMin;
            ColourElementValue_t colourRangeMax;
            double characteristicRangeMin;
            double characteristicRangeMax;

            // these two are populated by derived ViewDAI classes
            std::string processName;
            std::string characteristicName;

            // these two are populated by the Engine
            int characteristicIndex;
            int characteristicID;

            CharacteristicColourData(ColourElementValue_t colourMin,
                ColourElementValue_t colourMax,
                double characteristicMin,
                double characteristicMax,
                const std::string& pName,
                const std::string& cName);
            CharacteristicColourData();
        };

        typedef std::vector<CharacteristicColourData> CharacteristicColourDataArray;
        /**
         * @return
         */
        virtual CharacteristicColourDataArray GetCharacteristicColourInfo() const = 0;

        /**
         * Virtual destructor, like all good base class destructors should be
         */
        virtual ~ViewDAI()
        {
        }

    protected:
        ViewDAI()
        {
        }
    }; // class ViewDAI

    inline ViewDAI::CharacteristicColourData::CharacteristicColourData
    (ColourElementValue_t colourMin,
        ColourElementValue_t colourMax,
        double characteristicMin,
        double characteristicMax,
        const std::string& pName,
        const std::string& cName)
        : colourRangeMin(colourMin)
        , colourRangeMax(colourMax)
        , characteristicRangeMin(characteristicMin)
        , characteristicRangeMax(characteristicMax)
        , processName(pName)
        , characteristicName(cName)
        , characteristicIndex(-1)
        , characteristicID(-1)
    {
    }

    inline ViewDAI::CharacteristicColourData::CharacteristicColourData()
        : colourRangeMin(0)
        , colourRangeMax(0)
        , characteristicRangeMin(0.0)
        , characteristicRangeMax(0.0)
        , processName("")
        , characteristicName("")
        , characteristicIndex(-1)
        , characteristicID(-1)
    {
    }
} /* namespace rootmap */

#endif // #ifndef ViewDAI_H
