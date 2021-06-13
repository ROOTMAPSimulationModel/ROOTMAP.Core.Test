#ifndef TXCharacteristicDA_H
#define TXCharacteristicDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        TXCharacteristicDA.h
// Purpose:     Declaration of the TXCharacteristicDA class
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
    class TXCharacteristicDA : public CharacteristicDAI
    {
    public:
        virtual CharacteristicIdentifier getIdentifier() const;
        virtual std::string getName() const;
        virtual void setName(std::string newName);
        virtual std::string getUnits() const;
        virtual ScoreboardStratum getStratum() const;
        virtual double getMinimum() const;
        virtual double getMaximum() const;
        virtual double getDefault() const;
        virtual bool isVisible() const;
        virtual bool isEdittable() const;
        virtual bool isSavable() const;
        virtual bool hasSpecialPerBoxInfo() const;

        virtual bool isValid() const;

        virtual ~TXCharacteristicDA();

        TXCharacteristicDA(CharacteristicIdentifier id,
            std::string& name,
            std::string& units,
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
    }; // class TXCharacteristicDA
} /* namespace rootmap */

#endif // #ifndef TXCharacteristicDA_H
