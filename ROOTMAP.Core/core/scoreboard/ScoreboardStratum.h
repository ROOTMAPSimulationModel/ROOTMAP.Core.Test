#ifndef CS_ScoreboardStratum_HH
#define CS_ScoreboardStratum_HH
/////////////////////////////////////////////////////////////////////////////
// Name:        filename.h
// Purpose:     
// Created:     DD/MM/YYYY HH:MM:SS
// Author:      RvH
// $Date: 2009-07-10 06:43:25 +0800 (Fri, 10 Jul 2009) $
// $Revision: 69 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
//
// for iterating through Scoreboard Volumes, try :
//
// for ( ScoreboardStratum svt = ScoreboardStratum::first() ;
//       svt <= ScoreboardStratum::last() ; svt++ )
//        {
//     }
//
//  OR
//
// void ScoreboardCoordinator::IterateOverScoreboards(IScoreboardIterationUser * user);
//
// see void SimulationEngine::InitialiseScoreboardCharacteristicDefaults()
// for an example
//
//
//

#include "core/common/Types.h"


namespace rootmap
{
    //#define CS_SCOREBOARDSTRATUM_PROCESSLISTS
    //#define ROOTMAP_SCOREBOARDSTRATUM_IMPLEMENTS_LEGACY_GROUNDVOLUMETYPE_FUNCTIONS

    class Simulation;

    typedef signed long int ScoreboardStratumDataType;

    class ScoreboardStratum
    {
    public:
        //
        // constructors: copy, conversion and default
        ScoreboardStratum(const ScoreboardStratum& svt);
        explicit ScoreboardStratum(const ScoreboardStratumDataType svt);
        ScoreboardStratum();

        //
        // Synonyms for isAboveGround and isBelowGround?
        // Zero currently defines the "Ground" level
        Boolean isAboveZero() const;
        Boolean isBelowZero() const;

        // was: TOP_VOLUME_TYPE 0
        static ScoreboardStratumDataType first();
        // was: BOTTOM_VOLUME_TYPE vt15
        static ScoreboardStratumDataType last();

#if defined ROOTMAP_SCOREBOARDSTRATUM_IMPLEMENTS_LEGACY_GROUNDVOLUMETYPE_FUNCTIONS
        //
        // returns a menu or submenu ID, based on the names given, returning -1 if
        // no menu matches the description. Currently useful variations are
        // getMenuID("Edit", "Layers");
        // getMenuID("Table", "");
        // getMenuID("Table", "Layers", X);
        // getMenuID("Table", "Layers", Y);
        // getMenuID("Table", "Layers", Z);
        // getMenuID("Table", "Plant");
        short int getMenuID(const char * menuName, const char * submenuName, Dimension d = X) const;

        //
        // Returns the command number of a submenu, based on the names given.
        // Returns -1 if no menu matches the description. Currently useful
        // variations are
        // getMenuCommandNumber("Edit", "Layers");      replaces gvt2EditMenu_Layers_Cmd
        // getMenuCommandNumber("Table", "New Table");  replaces gvt2TableMenu_NewTable_Cmd
        // getMenuCommandNumber("Table", "Layers");     replaces gvt2TableMenu_Layers_Cmd
        // getMenuCommandNumber("Table", "Plant");      replaces gvt2TableMenu_Plant_Cmd
        ScoreboardStratumDataType getMenuCommandNumber(const char * menuName, const char * submenuName) const;

        //
        // Returns the command number of a stratum-based menu item on a submenu,
        // based on the names given.
        // Returns -1 if no menu matches the description. Currently useful
        // variations are
        // getItemCommandNumber("Table", "New Table");      replaces gvt2NewTableMenu_GVT_Cmd
        // getItemCommandNumber("Scoreboard Strata", "");   replaces gvt2VolumeTypesPopupMenu_GVT_Cmd
        // getItemCommandNumber("View", "Scoreboard");      replaces gvt2ScoreboardMenu_GVT_Cmd
        ScoreboardStratumDataType getItemCommandNumber(const char * menuName, const char * submenuName) const;


        //
        // Utility functions to access the first and last command numbers for a
        // range
        static ScoreboardStratumDataType getFirstItemCommandNumber(const char * menuName, const char * submenuName);
        static ScoreboardStratumDataType getLastItemCommandNumber(const char * menuName, const char * submenuName);
        static void getItemCommandNumberRange(const char * menuName, const char * submenuName, ScoreboardStratumDataType & first, ScoreboardStratumDataType & last);
#endif // #if defined ROOTMAP_SCOREBOARDSTRATUM_IMPLEMENTS_LEGACY_GROUNDVOLUMETYPE_FUNCTIONS

        //
        //
        void toCString(char* s, long max_len = 255) const;
        std::string toString() const;

        static std::string ToString(const ScoreboardStratum& stratum);

        void fromCString(const char* s);
        void fromCString(const std::string& s);

        static ScoreboardStratum StringToStratum(const char* s);
        static ScoreboardStratumDataType StringToStratumDataType(const std::string& s);


        Boolean IsCommandNewTable(ScoreboardStratumDataType theCmd);

        // accessed by syntax:
        //
        //      ScoreboardStratum::Air
        //
        // Should only be used for initialising.
        enum
        {
            Air = 0,
            Litter = 1,
            Soil = 2,
            Drainage = 3,

            // 4 thru 15 not used

            ALL = 16,

            NONE = -1
        };

        /// 
        /// NUM_VOLUME_TYPES is ACTUALLY only the number of actual, real scoreboards available.
        /// vtALL and vtNONE are abstract Volume Types.
        /// was: NUM_VOLUME_TYPES 16
        static const ScoreboardStratumDataType numberOfScoreboardStrata;

        /// 
        /// TOTAL_VOLUMES is used for defining arrays which need to include vtALL
        /// was: TOTAL_VOLUMES 17
        static const ScoreboardStratumDataType totalScoreboardStrata;

        /// 
        /// was: TOTAL_AVAILABLE_SCOREBOARDS 16
        static const ScoreboardStratumDataType numberOfAvailableScoreboards;

        /// 
        /// was: MIN_SCOREBOARD_VOLUME_INDEX 0
        static const ScoreboardStratumDataType minScoreboardVolumeIndex;

        /// 
        /// was: MAX_SCOREBOARD_VOLUME_INDEX 15
        static const ScoreboardStratumDataType maxScoreboardVolumeIndex;

#ifdef CS_SCOREBOARDSTRATUM_PROCESSLISTS
        // the number of total process volumes includes ALL, which has
        // was: TOTAL_AVAILABLE_PROCESSLISTS 17
        static const ScoreboardStratumDataType totalProcessVolumes;
        // was: MIN_PROCESSLIST_INDEX 0
        static const ScoreboardStratumDataType minProcessVolumeIndex;
        // was: MAX_PROCESSLIST_INDEX 16
        static const ScoreboardStratumDataType maxProcessVolumeIndex;
#endif // #ifdef CS_SCOREBOARDSTRATUM_PROCESSLISTS

        // OPERATOR OVERLOADING
        // prefix and postfix increment and decrement
        ScoreboardStratumDataType operator++();
        ScoreboardStratumDataType operator++(int);
        ScoreboardStratumDataType operator--();
        ScoreboardStratumDataType operator--(int);

        // assignment
        const ScoreboardStratum& operator=(long svt);
        const ScoreboardStratum& operator=(const ScoreboardStratum& svt);

        // extended assignment
        const ScoreboardStratum& operator+=(const ScoreboardStratum& svt);
        const ScoreboardStratum& operator-=(const ScoreboardStratum& svt);
        const ScoreboardStratum& operator+=(long svt);
        const ScoreboardStratum& operator-=(long svt);

        // logical
        Boolean operator==(const ScoreboardStratum& svt) const;
        Boolean operator!=(const ScoreboardStratum& svt) const;
        Boolean operator<(const ScoreboardStratum& svt) const;
        Boolean operator>(const ScoreboardStratum& svt) const;
        Boolean operator<=(const ScoreboardStratum& svt) const;
        Boolean operator>=(const ScoreboardStratum& svt) const;

        Boolean operator==(long svt) const;
        Boolean operator!=(long svt) const;
        Boolean operator<(long svt) const;
        Boolean operator>(long svt) const;
        Boolean operator<=(long svt) const;
        Boolean operator>=(long svt) const;

        ScoreboardStratumDataType value() const { return m_scoreboardVolume; }

        // static constants    
        // NOTE: These aren't really meant to be used. Instead,
        //       use the functions isAboveZero() is isBelowZero()
        // Z_V_T is the vt at the bottom of which zero ground is defined as existing.
        // was: ZERO_AT_BOTTOM_OF_VOLUME vtLitter
        static ScoreboardStratumDataType zeroAtBottom;
        // was: ZERO_AT_TOP_OF_VOLUME vtSoil
        static ScoreboardStratumDataType zeroAtTop;

    private:
        ScoreboardStratumDataType m_scoreboardVolume;

        static const ScoreboardStratumDataType firstEditMenuLayersSubmenuCommandNumber;
        static const ScoreboardStratumDataType firstTableMenuNewTableCommandNumber;
        static const ScoreboardStratumDataType firstTableMenuNewTableSubmenuItemCommandNumber;
        static const ScoreboardStratumDataType firstTableMenuLayersCommandNumber;
        static const ScoreboardStratumDataType firstTableMenuPlantCommandNumber;
        //    static const ScoreboardStratumDataType firstTableMenuPlantSubmenuItemCommandNumber;
        static const ScoreboardStratumDataType firstVolumeTypesPopupMenuVolumeItemCommandNumber;
        static const ScoreboardStratumDataType firstViewMenuScoreboardSubmenuVolumeItemCommandNumber;

        static const short int firstEditMenuLayersSubmenuID;
        static const short int firstTableMenuLayersSubmenuID;
        static const short int firstTableMenuPlantSubmenuID;
        static const short int firstTableMenuID;
    };

    const ScoreboardStratum StratumALL(ScoreboardStratum::ALL);
    const ScoreboardStratum StratumNONE(ScoreboardStratum::NONE);

    inline ScoreboardStratumDataType ScoreboardStratum::first()
    {
        return minScoreboardVolumeIndex;
    }

    inline ScoreboardStratumDataType ScoreboardStratum::last()
    {
        return maxScoreboardVolumeIndex;
    }

    // prefix increment
    inline ScoreboardStratumDataType ScoreboardStratum::operator++()
    {
        return (++m_scoreboardVolume);
    }

    // postfix increment
    inline ScoreboardStratumDataType ScoreboardStratum::operator++(int)
    {
        ScoreboardStratumDataType previous = m_scoreboardVolume++;
        return previous;
    }

    // prefix decrement
    inline ScoreboardStratumDataType ScoreboardStratum::operator--()
    {
        return (--m_scoreboardVolume);
    }

    // postfix increment
    inline ScoreboardStratumDataType ScoreboardStratum::operator--(int)
    {
        ScoreboardStratumDataType previous = m_scoreboardVolume--;
        return previous;
    }

    inline const ScoreboardStratum& ScoreboardStratum::operator=(const ScoreboardStratum& rhs)
    {
        m_scoreboardVolume = rhs.m_scoreboardVolume;
        return *this;
    }

    inline const ScoreboardStratum& ScoreboardStratum::operator+=(const ScoreboardStratum& rhs)
    {
        m_scoreboardVolume += rhs.m_scoreboardVolume;
        return *this;
    }

    inline const ScoreboardStratum& ScoreboardStratum::operator-=(const ScoreboardStratum& rhs)
    {
        m_scoreboardVolume -= rhs.m_scoreboardVolume;
        return *this;
    }

    inline Boolean ScoreboardStratum::operator==(const ScoreboardStratum& rhs) const
    {
        return (m_scoreboardVolume == rhs.m_scoreboardVolume);
    }

    inline Boolean ScoreboardStratum::operator!=(const ScoreboardStratum& rhs) const
    {
        return (m_scoreboardVolume != rhs.m_scoreboardVolume);
    }

    inline Boolean ScoreboardStratum::operator<(const ScoreboardStratum& rhs) const
    {
        return (m_scoreboardVolume < rhs.m_scoreboardVolume);
    }

    inline Boolean ScoreboardStratum::operator>(const ScoreboardStratum& rhs) const
    {
        return (m_scoreboardVolume > rhs.m_scoreboardVolume);
    }

    inline Boolean ScoreboardStratum::operator<=(const ScoreboardStratum& rhs) const
    {
        return (m_scoreboardVolume <= rhs.m_scoreboardVolume);
    }

    inline Boolean ScoreboardStratum::operator>=(const ScoreboardStratum& rhs) const
    {
        return (m_scoreboardVolume >= rhs.m_scoreboardVolume);
    }

    inline const ScoreboardStratum& ScoreboardStratum::operator=(long rhs)
    {
        m_scoreboardVolume = rhs;
        return *this;
    }

    inline const ScoreboardStratum& ScoreboardStratum::operator+=(long rhs)
    {
        m_scoreboardVolume += rhs;
        return *this;
    }

    inline const ScoreboardStratum& ScoreboardStratum::operator-=(long rhs)
    {
        m_scoreboardVolume -= rhs;
        return *this;
    }

    inline Boolean ScoreboardStratum::operator==(long rhs) const
    {
        return (m_scoreboardVolume == rhs);
    }

    inline Boolean ScoreboardStratum::operator!=(long rhs) const
    {
        return (m_scoreboardVolume != rhs);
    }

    inline Boolean ScoreboardStratum::operator<(long rhs) const
    {
        return (m_scoreboardVolume < rhs);
    }

    inline Boolean ScoreboardStratum::operator>(long rhs) const
    {
        return (m_scoreboardVolume > rhs);
    }

    inline Boolean ScoreboardStratum::operator<=(long rhs) const
    {
        return (m_scoreboardVolume <= rhs);
    }

    inline Boolean ScoreboardStratum::operator>=(long rhs) const
    {
        return (m_scoreboardVolume >= rhs);
    }


#if defined ROOTMAP_SCOREBOARDSTRATUMITERATOR
    // Simulation.cp should have been changed to use these, by now.
    typedef void(*sviDoForEachFunction)(const ScoreboardStratum & sv);
    typedef void(*sviDoForEachFunction1)(const ScoreboardStratum & sv, void * info);
    typedef void(*sviDoForEachFunction2)(const ScoreboardStratum & sv, void * info1, void * info2);
    typedef void(*sviDoForEachEITFunction)(const ScoreboardStratum & sv, Boolean scoreboardExists);

    class ScoreboardStratumIterator
    {
    public:
        static void doForEach(sviDoForEachFunction f);
        static void doForEach(sviDoForEachFunction1 f, void * info);
        static void doForEach(sviDoForEachFunction2 f, void * info1, void * info2);
        static void doForEachExistsIsTrue(sviDoForEachEITFunction f, Simulation * e);
    };
#endif // #if defined ROOTMAP_SCOREBOARDSTRATUMITERATOR
} /* namespace rootmap */


#endif // #ifndef CS_ScoreboardStratum_HH
