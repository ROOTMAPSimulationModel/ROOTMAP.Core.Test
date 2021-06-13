#ifndef DummyCharacteristicDA_H
#define DummyCharacteristicDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DummyCharacteristicDA.h
// Purpose:     Declaration of the DummyCharacteristicDA class
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
#include "simulation/data_access/interface/CharacteristicDAI.h"

namespace rootmap
{
    class DummyCharacteristicDA : public CharacteristicDAI
    {
    public:
        virtual CharacteristicIdentifier getIdentifier() const;
        virtual std::string getName() const;
        virtual std::string getUnits() const;
        virtual ScoreboardStratum getStratum() const;
        virtual double getMinimum() const;
        virtual double getMaximum() const;
        virtual double getDefault() const;
        virtual bool isVisible() const;
        virtual bool isEdittable() const;
        virtual bool isSavable() const;
        virtual bool hasSpecialPerBoxInfo() const;

        virtual ~DummyCharacteristicDA();

        DummyCharacteristicDA(CharacteristicIdentifier id,
            const std::string& name,
            const std::string& units,
            ScoreboardStratum stratum,
            double minimum,
            double maximum,
            double default_,
            bool isVisible,
            bool isEdittable,
            bool isSavable,
            bool hasSpecialPerBoxInfo);

    private:
        CharacteristicIdentifier m_id;
        std::string m_name;
        std::string m_units;
        ScoreboardStratum m_stratum;
        double m_minimum;
        double m_maximum;
        double m_default;
        bool m_isVisible;
        bool m_isEdittable;
        bool m_isSavable;
        bool m_hasSpecialPerBoxInfo;
    }; // class DummyCharacteristicDA
} /* namespace rootmap */

#endif // #ifndef DummyCharacteristicDA_H
