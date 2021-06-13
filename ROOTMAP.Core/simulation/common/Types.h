#ifndef SimulationTypes_H
#define SimulationTypes_H
#include "core/macos_compatibility/macos_compatibility.h"

#include "core/common/Types.h"
#include <time.h>

#include <set>
#include <vector>

// MSA 09.12.16 Horrible workaround required due to legacy Windows foolishness 
// See http://social.msdn.microsoft.com/forums/en-US/vclanguage/thread/d986a370-d856-4f9e-9f14-53f3b18ab63e
#undef max

#include <limits>

#include "wx/string.h"

namespace rootmap
{
    /// type definition for 
    typedef time_t ProcessTime_t;

    /// type definition for 
    typedef OSType MessageType_t;

    ///
    typedef long int SignedIdentifier;

    ///
    typedef unsigned long int UnsignedIdentifier;

    /// type definition for 
    typedef SignedIdentifier CharacteristicIdentifier;

    /// type definition for 
    typedef SignedIdentifier ProcessIdentifier;

    /// type definition for 
    typedef SignedIdentifier PlantIdentifier;

    /// type definition for 
    typedef SignedIdentifier PlantSummaryIdentifier;

    /// type definition for 
    typedef SignedIdentifier PlantTypeIdentifier;

    /// type definition for identifiers used by Branch, Axis, RootTip
    typedef UnsignedIdentifier PlantElementIdentifier;

    /// Enumerated values representing simulation phase
    enum SimulationPhase
    {
        simNONE = 0,
        simInitialising,
        simInitialised,
        simStarting,
        simRunning,
        simStopRequested,
        simPauseRequested,
        simPaused,
        simStopping,
        simIdling
    };

    /// enumerated values for c,m,y. These are the primary additive colours
    /// which we use to represent characteristic values
    enum CMYSelect
    {
        Cyan = 0,
        Magenta = 1,
        Yellow = 2,
        Undisplayed = 3 // Is this a "typedef NewBool { true, false, undefined };" kind of atrocity?
    };

    /// unsigned char is what wxColour uses
    typedef unsigned char ColourElementValue_t;
    // 
    const ColourElementValue_t colourElementMaxValue = std::numeric_limits<ColourElementValue_t>::max();
    const int colourElementsMax = 3;

    struct CharacteristicColourInfo
    {
        // For scoreboard access.
        // Used only by VScoreboardColourBoxes, not useful for file data
        long int characteristicIndex;
        // For correct recollection of the characteristic between save & open
        long int characteristicID;

        // floor and ceiling for characteristic values
        double characteristicRangeMin;
        double characteristicRangeMax;

        // when the characteristic displayed is <=characteristicRangeMin,
        // this is the amount of this colour part displayed.
        ColourElementValue_t colourRangeMin;
        // ditto except >=characteristicRangeMax
        ColourElementValue_t colourRangeMax;
        // MSA 11.04.20
        // This is, unfortunately, the most efficient arrangement of members.
        // Still results in 6 bytes padding and a struct size of 40 bytes.
    };

    typedef CharacteristicColourInfo CharacteristicColours_t[colourElementsMax];

    wxString& operator<<(CMYSelect cmysel, wxString& str);
    std::string& operator+=(CMYSelect cmysel, std::string& str);

    enum PopulateProcessStringFlags
    {
        ppsf_All = 0,
        ppsf_DrawingOnly = 1
    };


    class Process;
    typedef std::vector<Process *> ProcessArray;

    typedef std::set<ProcessIdentifier> ProcessIdentifierSet;
} /* namespace rootmap */

#endif // #ifndef SimulationTypes_H
