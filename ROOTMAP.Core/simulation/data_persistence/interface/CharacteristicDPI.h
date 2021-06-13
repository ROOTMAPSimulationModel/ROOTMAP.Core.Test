#ifndef CharacteristicDPI_H
#define CharacteristicDPI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        CharacteristicDPI.h
// Purpose:     Declaration of the CharacteristicDPI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

//#include "core/scoreboard/ScoreboardStratum.h"
#include "core/common/Types.h"
#include "core/common/Structures.h"
#include "simulation/common/Types.h"

namespace rootmap
{
    class ScoreboardStratum;

    class CharacteristicDPI
    {
    public:
        virtual void storeIdentifier(const CharacteristicIdentifier& id) = 0;
        virtual void storeName(const std::string& name) = 0;
        virtual void storeUnits(const std::string& units) = 0;
        virtual void storeStratum(const ScoreboardStratum& stratum) = 0;
        virtual void storeMinimum(double minimum) = 0;
        virtual void storeMaximum(double maximum) = 0;
        virtual void storeDefault(double default_) = 0;
        virtual void storeVisible(bool is_visible) = 0;
        virtual void storeEdittable(bool is_edittable) = 0;
        virtual void storeSavable(bool is_savable) = 0;
        virtual void storeSpecialPerBoxInfo(bool is_special) = 0;

        virtual ~CharacteristicDPI()
        {
        }

    protected:
        CharacteristicDPI()
        {
        }
    }; // class CharacteristicDPI
} /* namespace rootmap */

#endif // #ifndef CharacteristicDPI_H
