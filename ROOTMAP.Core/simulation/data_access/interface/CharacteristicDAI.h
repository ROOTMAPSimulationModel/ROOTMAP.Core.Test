#ifndef CharacteristicDAI_H
#define CharacteristicDAI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        CharacteristicDAI.h
// Purpose:     Declaration of the CharacteristicDAI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

#include "core/scoreboard/ScoreboardStratum.h"
#include "core/common/Types.h"
#include "core/common/Structures.h"
#include "simulation/common/Types.h"

namespace rootmap
{
    class CharacteristicDAI
    {
    public:
        virtual CharacteristicIdentifier getIdentifier() const = 0;
        virtual std::string getName() const = 0;
        virtual std::string getUnits() const = 0;
        virtual ScoreboardStratum getStratum() const = 0;
        virtual double getMinimum() const = 0;
        virtual double getMaximum() const = 0;
        virtual double getDefault() const = 0;
        virtual bool isVisible() const = 0;
        virtual bool isEdittable() const = 0;
        virtual bool isSavable() const = 0;
        virtual bool hasSpecialPerBoxInfo() const = 0;

        virtual ~CharacteristicDAI()
        {
        }

    protected:
        CharacteristicDAI()
        {
        }
    }; // class CharacteristicDAI
} /* namespace rootmap */

#endif // #ifndef CharacteristicDAI_H
