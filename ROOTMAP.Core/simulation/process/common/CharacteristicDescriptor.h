#ifndef CharacteristicDescriptor_H
#define CharacteristicDescriptor_H
/////////////////////////////////////////////////////////////////////////////
// Name:        CharacteristicDescriptor.h
// Purpose:     Declaration of the CharacteristicDescriptor struct/class
// Created:     14-07-2002
// Author:      RvH
// $Date: 2008-11-06 00:38:15 +0900 (Thu, 06 Nov 2008) $
// $Revision: 24 $
// Copyright:   ©2002-2008 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

//#define CS_USING_CHARACTERISTICDESCRIPTOR_CLASS
#ifndef CS_USING_CHARACTERISTICDESCRIPTOR_CLASS
#define CS_USING_CHARACTERISTICDESCRIPTOR_STRUCT
#endif // #ifndef CS_USING_CHARACTERISTICDESCRIPTOR_CLASS

#include "core/common/Types.h"
#include "core/scoreboard/ScoreboardStratum.h"
#include "core/scoreboard/ScoreboardBox.h"

namespace rootmap
{
    class CharacteristicDAI;

#ifndef CS_USING_CHARACTERISTICDESCRIPTOR_CLASS
    //DONE: CharacteristicDescriptor needs a scoreboardstratum so as to
    //      be able to know which scoreboard to register with
    struct CharacteristicDescriptor
    {
        long id;

        CharacteristicIndex ScoreboardIndex;

        double Minimum;
        double Maximum;
        double Default;

        std::string Name;
        std::string Units;

        bool Visible;
        bool Edittable;
        bool Savable;
        bool SpecialPerBoxInfo;

        ScoreboardStratum m_ScoreboardStratum;

        // default constructor
        CharacteristicDescriptor();
        // copy of a plant summary
        CharacteristicDescriptor(const CharacteristicDescriptor& rhs,
            long newID,
            long newScoreboardIndex,
            const std::string& newName
        );

        CharacteristicDescriptor(const CharacteristicDAI& data);

        // destructor
        virtual ~CharacteristicDescriptor();

        const ScoreboardStratum& GetScoreboardStratum();

        static const CharacteristicDescriptor Null;

        bool operator==(const CharacteristicDescriptor& rhs) const;
        bool operator!=(const CharacteristicDescriptor& rhs) const;
    private:
        // the copy constructor and assignment operator are hidden, because you
        // can't copy a CharacteristicDescriptor.  Two characteristics should not
        // share the same Name, id and ScoreboardIndex
        CharacteristicDescriptor(const CharacteristicDescriptor& rhs);
        //    operator=(const CharacteristicDescriptor & rhs);

    public:
        // advanced placement of future interface
        // (currently only used by SharedAttributeXIHHelper)
        void SetID(long value);
        void SetScoreboardIndex(long value);
        void SetMinimum(double value);
        void SetMaximum(double value);
        void SetDefault(double value);
        void SetName(const std::string& value);
        void SetUnits(const std::string& value);
        void SetName(const char* s);
        void SetUnits(const char* s);
        void SetVisible(bool value);
        void SetEdittable(bool value);
        void SetSavable(bool value);
        void SetSpecialPerBoxInfo(bool value);
        void SetScoreboardStratum(const ScoreboardStratum& ss);

        friend std::ostream& operator<<(std::ostream& ostr, const CharacteristicDescriptor& summary);
    };

    std::ostream& operator<<(std::ostream& ostr, const CharacteristicDescriptor& summary);

    inline void CharacteristicDescriptor::SetID(long value)
    {
        id = value;
    }

    inline void CharacteristicDescriptor::SetScoreboardIndex(long value)
    {
        ScoreboardIndex = value;
    }

    inline void CharacteristicDescriptor::SetMinimum(double value)
    {
        Minimum = value;
    }

    inline void CharacteristicDescriptor::SetMaximum(double value)
    {
        Maximum = value;
    }

    inline void CharacteristicDescriptor::SetDefault(double value)
    {
        Default = value;
    }

    inline void CharacteristicDescriptor::SetName(const std::string& value)
    {
        Name = value;
    }

    inline void CharacteristicDescriptor::SetUnits(const std::string& value)
    {
        Units = value;
    }

    inline void CharacteristicDescriptor::SetVisible(bool value)
    {
        Visible = value;
    }

    inline void CharacteristicDescriptor::SetEdittable(bool value)
    {
        Edittable = value;
    }

    inline void CharacteristicDescriptor::SetSavable(bool value)
    {
        Savable = value;
    }

    inline void CharacteristicDescriptor::SetSpecialPerBoxInfo(bool value)
    {
        SpecialPerBoxInfo = value;
    }

    inline void CharacteristicDescriptor::SetScoreboardStratum(const ScoreboardStratum& ss)
    {
        m_ScoreboardStratum = ss;
    }


#else // Then CS_USING_CHARACTERISTICDESCRIPTOR_CLASS is defined

    class CharacteristicDescriptor
    {
    public:
        CharacteristicDescriptor(long id,
            long scoreboardindex,
            double minimum,
            double maximum,
            double default_,
            const std::string & name,
            const std::string & units,
            bool visible,
            bool editable,
            bool savable,
            bool specialperboxinfo
        );
        virtual ~CharacteristicDescriptor();

        // accessors
        long GetID();
        long GetScoreboardIndex();
        const ScoreboardStratum & GetScoreboardStratum();
        double GetMinimum();
        double GetMaximum();
        double GetDefault();
        const std::string & GetName();
        const std::string & GetUnits();
        void GetName(Str255 namestr);
        void GetUnits(Str255 unitstr);

        bool IsVisible();
        bool IsEdittable();
        bool IsSavable();
        bool IsSpecialPerBoxInfo();

        // mutators
        //
        // Shouldn't need these, as the only constructor available should do so
        //
#if 0
        void SetID(long value);
        void SetScoreboardIndex(long value);
        void SetMinimum(double value);
        void SetMaximum(double value);
        void SetDefault(double value);
        void Set(const std::string & value);
        void Set(const std::string & value);
        void SetVisible(bool value);
        void SetEdittable(bool value);
        void SetSavable(bool value);
        void SetSpecialPerBoxInfo(bool value);
#endif

    private:
        long        m_ID;

        long        m_ScoreboardIndex;
        ScoreboardStratum   m_ScoreboardStratum;


        double   m_Minimum;
        double   m_Maximum;
        double   m_Default;

        std::string    m_Name;
        std::string    m_Units;

        bool        m_Visible;
        bool        m_Edittable;
        bool        m_Savable;
        bool        m_SpecialPerBoxInfo;
    };

    inline CharacteristicDescriptor::CharacteristicDescriptor
    (long id,
        long scoreboardindex,
        double minimum,
        double maximum,
        double default_,
        const std::string & name,
        const std::string & units,
        bool visible,
        bool editable,
        bool savable,
        bool specialperboxinfo
    )
        : id(id)
        , ScoreboardIndex(scoreboardindex)
        , Minimum(minimum)
        , Maximum(maximum)
        , Default(default_)
        , Name(name)
        , Units(units)
        , Visible(visible)
        , Edittable(editable)
        , Savable(savable)
        , SpecialPerBoxInfo(specialperboxinfo)
    {
    }

    inline CharacteristicDescriptor::~CharacteristicDescriptor()
    {
        TCLForgetHandle(Name);
        TCLForgetHandle(Units);
    }

    // accessors
    inline long CharacteristicDescriptor::GetID()
    {
        return id;
    }

    inline long CharacteristicDescriptor::GetScoreboardIndex()
    {
        return ScoreboardIndex;
    }

    inline double CharacteristicDescriptor::GetMinimum()
    {
        return Minimum;
    }

    inline double CharacteristicDescriptor::GetMaximum()
    {
        return Maximum;
    }

    inline double CharacteristicDescriptor::GetDefault()
    {
        return Default;
    }

    inline const std::string & CharacteristicDescriptor::GetName()
    {
        return Name;
    }

    inline const std::string & CharacteristicDescriptor::GetUnits()
    {
        return Units;
    }

    inline void CharacteristicDescriptor::GetName(std::string & namestr)
    {
        namestr = GetName();
    }

    inline void CharacteristicDescriptor::GetUnits(std::string & unitstr)
    {
        unitstr = GetUnits();
    }

    inline bool CharacteristicDescriptor::IsVisible()
    {
        return Visible;
    }

    inline bool CharacteristicDescriptor::IsEdittable()
    {
        return Edittable;
    }

    inline bool CharacteristicDescriptor::IsSavable()
    {
        return Savable;
    }

    inline bool CharacteristicDescriptor::IsSpecialPerBoxInfo()
    {
        return SpecialPerBoxInfo;
    }

#endif // #ifdef CS_USING_CHARACTERISTICDESCRIPTOR_CLASS

    inline const ScoreboardStratum& CharacteristicDescriptor::GetScoreboardStratum()
    {
        return m_ScoreboardStratum;
    }
} /* namespace rootmap */

#endif    // CharacteristicDescriptor_H
