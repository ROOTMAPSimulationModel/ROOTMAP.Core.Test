/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardStratum.cpp
// Purpose:     Implementation of the ClassTemplate class
// Created:     2003
// Author:      RvH
// $Date: 2009-07-10 06:43:25 +0800 (Fri, 10 Jul 2009) $
// $Revision: 69 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/common/SimplestCrudestHash.h"
#include "core/macos_compatibility/CropSimStringResources.h"
#include "core/macos_compatibility/MacResourceManager.h"
#include "core/scoreboard/ScoreboardStratum.h"
#include "core/utility/Utility.h"

#include <sstream>

namespace rootmap
{
    ScoreboardStratum::ScoreboardStratum(const ScoreboardStratum& svt)
        : m_scoreboardVolume(svt.m_scoreboardVolume)
    {
    }

    ScoreboardStratum::ScoreboardStratum(const ScoreboardStratumDataType svt)
        : m_scoreboardVolume(svt)
    {
    }

    ScoreboardStratum::ScoreboardStratum()
        : m_scoreboardVolume(NONE)
    {
    }

#if defined ROOTMAP_SCOREBOARDSTRATUM_IMPLEMENTS_LEGACY_GROUNDVOLUMETYPE_FUNCTIONS
    //
    // The alternative to the crude hash is strcmp. Not so much more difficult to
    // use, more accurate than the crudeHash, and probably as fast as a map.
    //
    short int ScoreboardStratum::getMenuID(const char * menuName, const char * submenuName, Dimension d) const
    {
        short int menu_id = -1;
        ScoreboardStratumDataType menuCrudeHash = crudeHash(menuName);
        ScoreboardStratumDataType submenuCrudeHash = crudeHash(submenuName);

        if (menuCrudeHash == crudeHashTable)
        {
            if (submenuCrudeHash == crudeHashLayers)
            {
                // Table menu, Layers submenu
                menu_id = (short int)(firstTableMenuLayersSubmenuID +
                    (m_scoreboardVolume * NumberOfDimensions) + d);
            }
            else if (submenuCrudeHash == crudeHashPlant)
            {
                // Table menu, Plant submenu
                menu_id = (short int)(firstTableMenuPlantSubmenuID + m_scoreboardVolume);
            }
            else if (submenuName[0] == '\0')
            {
                // Table menu
                menu_id = (short int)(firstTableMenuID + m_scoreboardVolume);
            }
        }
        else if (menuCrudeHash == crudeHashEdit)
        {
            if (submenuCrudeHash == crudeHashLayers)
            {
                // Edit menu, Layers submenu
                menu_id = (short int)(firstEditMenuLayersSubmenuID + m_scoreboardVolume);
            }
        }

        return menu_id;
    }

    //
    //
    ScoreboardStratumDataType ScoreboardStratum::getMenuCommandNumber(const char * menuName, const char * submenuName) const
    {
        ScoreboardStratumDataType cmd = -1;
        ScoreboardStratumDataType menuCrudeHash = crudeHash(menuName);
        ScoreboardStratumDataType submenuCrudeHash = crudeHash(submenuName);

        if (menuCrudeHash == crudeHashTable)
        {
            if (submenuCrudeHash == crudeHashLayers)
            {
                // Table menu, Layers submenu
                cmd = (ScoreboardStratumDataType)(firstTableMenuLayersCommandNumber + m_scoreboardVolume);
            }
            else if (submenuCrudeHash == crudeHashPlant)
            {
                // Table menu, Plant submenu
                cmd = (ScoreboardStratumDataType)(firstTableMenuPlantCommandNumber + m_scoreboardVolume);
            }
            else if (submenuCrudeHash == crudeHashNewTable)
            {
                // Table menu, "New Table" submenu
                cmd = (ScoreboardStratumDataType)(firstTableMenuNewTableCommandNumber + m_scoreboardVolume);
            }
        }
        else if (menuCrudeHash == crudeHashEdit)
        {
            if (submenuCrudeHash == crudeHashLayers)
            {
                // Edit menu, Layers submenu
                cmd = (ScoreboardStratumDataType)(firstEditMenuLayersSubmenuCommandNumber + m_scoreboardVolume);
            }
        }

        return cmd;
    }

    //
    //
    // getItemCommandNumber("Table", "New Table"); replaces gvt2NewTableMenu_GVT_Cmd
    // getItemCommandNumber("Scoreboard Strata", ""); replaces gvt2VolumeTypesPopupMenu_GVT_Cmd
    // getItemCommandNumber("View", "Scoreboard"); replaces gvt2ScoreboardMenu_GVT_Cmd
    ScoreboardStratumDataType ScoreboardStratum::getItemCommandNumber(const char * menuName, const char * submenuName) const
    {
        ScoreboardStratumDataType cmd = -1;
        ScoreboardStratumDataType menuCrudeHash = crudeHash(menuName);
        ScoreboardStratumDataType submenuCrudeHash = crudeHash(submenuName);

        if (menuCrudeHash == crudeHashTable)
        {
            if (submenuCrudeHash == crudeHashNewTable)
            {
                // Table menu, "New Table" submenu
                cmd = (ScoreboardStratumDataType)(firstTableMenuNewTableSubmenuItemCommandNumber + m_scoreboardVolume);
            }
        }
        else if (menuCrudeHash == crudeHashView)
        {
            if (submenuCrudeHash == crudeHashScoreboard)
            {
                // View menu, Scoreboard submenu
                cmd = (ScoreboardStratumDataType)(firstViewMenuScoreboardSubmenuVolumeItemCommandNumber + m_scoreboardVolume);
            }
        }
        else if (menuCrudeHash == crudeHashScoreboard)
        {
            if (submenuCrudeHash == crudeHashZero)
            {
                // "Scoreboard Strata" popup menu
                cmd = (ScoreboardStratumDataType)(firstVolumeTypesPopupMenuVolumeItemCommandNumber + m_scoreboardVolume);
            }
        }

        return cmd;
    }

    ScoreboardStratumDataType ScoreboardStratum::getFirstItemCommandNumber(const char * menuName, const char * submenuName)
    {
        ScoreboardStratum ss = ScoreboardStratum::first();
        return ss.getItemCommandNumber(menuName, submenuName);
    }


    ScoreboardStratumDataType ScoreboardStratum::getLastItemCommandNumber(const char * menuName, const char * submenuName)
    {
        ScoreboardStratum ss = ScoreboardStratum::last();
        return ss.getItemCommandNumber(menuName, submenuName);
    }

    void ScoreboardStratum::getItemCommandNumberRange(const char * menuName, const char * submenuName, ScoreboardStratumDataType & first, ScoreboardStratumDataType & last)
    {
        first = getFirstItemCommandNumber(menuName, submenuName);
        last = getLastItemCommandNumber(menuName, submenuName);
    }
#endif // #if defined ROOTMAP_SCOREBOARDSTRATUM_IMPLEMENTS_LEGACY_GROUNDVOLUMETYPE_FUNCTIONS


    void ScoreboardStratum::toCString(char* s, long max_len) const
    {
        char buff[16];
        sprintf_s(buff, 16, "%d", m_scoreboardVolume);

        strncpy_s(s, 16, buff, max_len);
    }

    std::string ScoreboardStratum::toString() const
    {
        std::ostringstream s;
        s << m_scoreboardVolume;
        return s.str();
    }

    std::string ScoreboardStratum::ToString(const ScoreboardStratum& stratum)
    {
        return (stratum.toString());
    }

    void ScoreboardStratum::fromCString(const char* s)
    {
        m_scoreboardVolume = StringToStratumDataType(s);
    }

    ScoreboardStratum ScoreboardStratum::StringToStratum(const char* s)
    {
        ScoreboardStratum stratum(ScoreboardStratum::NONE);
        stratum.fromCString(s);
        return stratum;
    }

    ScoreboardStratumDataType ScoreboardStratum::StringToStratumDataType(const std::string& s)
    {
        long string_list_id = STRL_GVTDisplay;
        std::string use_string = s;

        if ((s[0] == 'v') && (s[1] == 't'))
        {
            std::copy(s.begin() + 2, s.end(), use_string.begin());
        }

        for (ScoreboardStratumDataType ssval = minScoreboardVolumeIndex; ssval <= numberOfScoreboardStrata; ++ssval)
        {
            std::string fetched_string;
            Utility::CS_GetIndString(fetched_string, string_list_id, ssval);

            if (use_string == fetched_string)
            {
                return ssval;
            }
        }

        return ScoreboardStratum::NONE;
    }

    /*
    */
    Boolean ScoreboardStratum::IsCommandNewTable(long theCmd)
    {
        Boolean isNewTable = false;

        if ((theCmd >= firstTableMenuNewTableSubmenuItemCommandNumber) &&
            (theCmd <= (firstTableMenuNewTableSubmenuItemCommandNumber + numberOfScoreboardStrata)))
        {
            m_scoreboardVolume = theCmd - firstTableMenuNewTableSubmenuItemCommandNumber;
            isNewTable = true;
        }

        return isNewTable;
    }


#if defined ROOTMAP_SCOREBOARDSTRATUMITERATOR
#pragma message(HERE "TODO: Deprecate ScoreboardStratumIterator, better functionality implemented in ScoreboardCoordinator")
    void ScoreboardStratumIterator::doForEach(sviDoForEachFunction f)
    {
        ScoreboardStratum sv = ScoreboardStratum::first();
        while (sv <= ScoreboardStratum::last())
        {
            (*f)(sv);
            ++sv;
        }
    }

    void ScoreboardStratumIterator::doForEach(sviDoForEachFunction1 f, void * info)
    {
        ScoreboardStratum sv = ScoreboardStratum::first();
        while (sv <= ScoreboardStratum::last())
        {
            (*f)(sv, info);
            ++sv;
        }
    }

    void ScoreboardStratumIterator::doForEach(sviDoForEachFunction2 f, void * info1, void * info2)
    {
        ScoreboardStratum sv = ScoreboardStratum::first();
        while (sv <= ScoreboardStratum::last())
        {
            (*f)(sv, info1, info2);
            ++sv;
        }
    }

    void ScoreboardStratumIterator::doForEachExistsIsTrue(sviDoForEachEITFunction f, Simulation * e)
    {
        ScoreboardStratum sv = ScoreboardStratum::first();
        while (sv <= ScoreboardStratum::last())
        {
            ++sv;
        }
    }
#endif // #if defined ROOTMAP_SCOREBOARDSTRATUMITERATOR

    const ScoreboardStratumDataType ScoreboardStratum::numberOfScoreboardStrata = 16;
    const ScoreboardStratumDataType ScoreboardStratum::totalScoreboardStrata = 17;
    const ScoreboardStratumDataType ScoreboardStratum::numberOfAvailableScoreboards = 16;
    const ScoreboardStratumDataType ScoreboardStratum::minScoreboardVolumeIndex = 0;
    const ScoreboardStratumDataType ScoreboardStratum::maxScoreboardVolumeIndex = 15;

#ifdef CS_SCOREBOARDSTRATUM_PROCESSLISTS
    // the number of total process volumes includes ALL, which has
    // was: TOTAL_AVAILABLE_PROCESSLISTS 17
    const ScoreboardStratumDataType ScoreboardStratum::totalProcessVolumes = 17;
    // was: MIN_PROCESSLIST_INDEX 0
    const ScoreboardStratumDataType ScoreboardStratum::minProcessVolumeIndex = 0;
    // was: MAX_PROCESSLIST_INDEX 16
    const ScoreboardStratumDataType ScoreboardStratum::maxProcessVolumeIndex = 16;
#endif // #ifdef CS_SCOREBOARDSTRATUM_PROCESSLISTS

    // NOTE: These aren't really meant to be used. Instead,
    //       use the functions isAboveZero() is isBelowZero()
    // Z_V_T is the vt at the bottom of which zero ground is defined as existing.
    // was: ZERO_AT_BOTTOM_OF_VOLUME vtLitter
    ScoreboardStratumDataType ScoreboardStratum::zeroAtBottom = Litter;
    // was: ZERO_AT_TOP_OF_VOLUME vtSoil
    ScoreboardStratumDataType ScoreboardStratum::zeroAtTop = Soil;

    Boolean ScoreboardStratum::isAboveZero() const
    {
        return (m_scoreboardVolume <= zeroAtBottom);
    }

    Boolean ScoreboardStratum::isBelowZero() const
    {
        return (m_scoreboardVolume >= zeroAtTop);
    }

    const short int ScoreboardStratum::firstTableMenuID = 21;
    const short int ScoreboardStratum::firstEditMenuLayersSubmenuID = 51;
    const short int ScoreboardStratum::firstTableMenuLayersSubmenuID = 83;
    const short int ScoreboardStratum::firstTableMenuPlantSubmenuID = 67;

    const ScoreboardStratumDataType ScoreboardStratum::firstEditMenuLayersSubmenuCommandNumber = 1510;
    const ScoreboardStratumDataType ScoreboardStratum::firstTableMenuNewTableCommandNumber = 2934;
    const ScoreboardStratumDataType ScoreboardStratum::firstTableMenuNewTableSubmenuItemCommandNumber = 2950;
    const ScoreboardStratumDataType ScoreboardStratum::firstTableMenuLayersCommandNumber = 2966;
    const ScoreboardStratumDataType ScoreboardStratum::firstTableMenuPlantCommandNumber = 2982;
    const ScoreboardStratumDataType ScoreboardStratum::firstVolumeTypesPopupMenuVolumeItemCommandNumber = 1024;
    const ScoreboardStratumDataType ScoreboardStratum::firstViewMenuScoreboardSubmenuVolumeItemCommandNumber = 2031;
    // const ScoreboardStratumDataType ScoreboardStratum::first = ;
} /* namespace rootmap */
