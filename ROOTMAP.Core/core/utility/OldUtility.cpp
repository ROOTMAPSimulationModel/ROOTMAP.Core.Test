/////////////////////////////////////////////////////////////////////////////
// Name:        ClassTemplate.cpp
// Purpose:     Implementation of the ClassTemplate class
// Created:     1993 (approximately)
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/utility/Utility.h"

#include "core/common/Constants.h"
#include "core/common/Exceptions.h"
#include "core/common/RmAssert.h"

#include "core/macos_compatibility/MacResourceManager.h"

#include <cmath>
#include <limits>
#include <sstream>
#include <iomanip>

extern "C" 
{
    #define SWAP_2(x) ( (((x) & 0xff) << 8) | ((unsigned short)(x) >> 8) )
    #define SWAP_4(x) ( ((x) << 24) | \
             (((x) << 8) & 0x00ff0000) | \
             (((x) >> 8) & 0x0000ff00) | \
             ((x) >> 24) )
    #define FIX_SHORT(x) (*(unsigned short *)&(x) = SWAP_2(*(unsigned short *)&(x)))
    #define FIX_INT(x)   (*(unsigned int *)&(x)   = SWAP_4(*(unsigned int *)&(x)))
}

namespace rootmap
{

using namespace MacResourceManager;

namespace Utility
{
/* Max and Min macros */ 
// #defines Maxi, Mini, isEven, isOdd, CS_HiWord, CS_LoWord in Utilities.h

// merges two short ints into one long int
long MergeWords(short hi, short lo)
{
/* Long Form :
    long merger;
    merger = (hi<<16) | lo;
    return merger;
*/
    return ((hi << 16) | lo);
}

/* isInfinite
checks if the number is within range of the predefined "CS_infinity" constant.
By "within range", we mean within a magnitude below, also. This function used to
check either way, however this is now not so, since we defined infinity as the
official <limits> version. */
Boolean isInfinite (double_cs check_number)
{
    return (check_number == std::numeric_limits<double_cs>::infinity());
}

/* CalculateNewScale
Used by VPanorama and VWindow to calculate new view scales when windows are resized.

It is not so important for VWindow, since it is only used to calculate the rect to splat in 
UpdateErase(), which is a purely aesthetic routine.

This uses the ratio of the diagonals. It could just as easily go by the lengths of the sides, which
would not be so much fuss accounting for differences between maths libraries. */ 
double_cs CalculateNewScale(    double_cs oldScale,
                    short int oldH, short int oldV,
                    short int newH, short int newV)
{
    double_cs oldDiagonal, newDiagonal;
    oldDiagonal = sqrt( (double_cs)((oldV*oldV) + (oldH*oldH)) );
    newDiagonal = sqrt( (double_cs)((newV*newV) + (newH*newH)) );
    return (double_cs)(oldScale * (newDiagonal/oldDiagonal));
}


typedef struct {
    unsigned short short1;
    unsigned short short2;
} two_unsigned_shorts;
typedef struct {
    signed short short1;
    signed short short2;
} two_signed_shorts;
typedef union {
    void* void_ptr;
    Ptr byte_ptr;
    signed long sl;
    unsigned long ul;
    two_signed_shorts tss;
    two_unsigned_shorts tus;
    char chars[4];
} four_byte_union;

inline int Random()
{
    return (std::rand());
}

double_cs Random1()
{
    return (((double_cs)Random())/((double_cs)RAND_MAX));
}

double_cs RandomRange(double_cs range_max, double_cs range_min)
{
    return (((double_cs)Random()/(double_cs)RAND_MAX) * range_max + range_min);
}

signed long RandomRange(signed long range_max, signed long range_min)
{
    return (std::floor(RandomRange((double_cs)range_max, (double_cs)range_min)));
}

/* CopySFReply */ 
//void CopySFReply (SFReply fromSF, SFReply *toSF)
//{
//    toSF->good = fromSF.good;
//    toSF->copy = fromSF.copy;
//    toSF->fType = fromSF.fType;
//    toSF->vRefNum = fromSF.vRefNum;
//    toSF->version = fromSF.version;
//    ThinkClassLibrary::TCLpstrcpy(toSF->fName, fromSF.fName);
//}

RmString CS_GetString(long string_resource_id)
{
    RootMapLoggerInitialisation("rootmap.Utility");
    try
    {
        // Get the requested string into memory
        return(MacResourceManager::GetString(string_resource_id));
    }
    catch (const MacResourceManagerException & mrme)
    {
        LOG_ERROR << "MacResourceManager failed to GetString: " << mrme.what();
        return RmString("");
    }
}

bool CS_GetIndString(RmString & pstring, long strListID, long str_index)
{
    RootMapLoggerInitialisation("rootmap.Utility");
    try
    {
        // Get the requested string into memory
        MacResourceManager::GetIndString(pstring, strListID, str_index);
    }
    catch (const MacResourceManagerException & mrme)
    {
        LOG_ERROR << "MacResourceManager failed to GetIndString: " << mrme.what();
        return false;
    }
    
    return true;
}


Dimension StringToDimension(char dim_string)
{
    switch (dim_string)
    {
        case 'X' : case 'x': return (X);
        case 'Y' : case 'y': return (Y);
        case 'Z' : case 'z': return (Z);
        default :
        {
            RmString dimstr(1,dim_string);
            throw StringConversionException("Unrecognised dimension character",dimstr);
        }
    }
}

/* StringToDouble
98.09.12 renamed from sscanDouble */
double_cs StringToDouble (const char * c_string)
{
    double_cs the_double=0.0;
    
    if (strlen(c_string) > 0)
    {
        /* Compare the input string with five possible 'infinite' strings,
        ignoring case and diacritical marks. */
        if (strcmp(c_string, "inf") == 0)
        {
            the_double = infinity();
        }
        else if (strcmp(c_string, "INF") == 0)
        {
            the_double = infinity();
        }
        else if (strcmp(c_string, "infinite") == 0)
        {
            the_double = infinity();
        }
        else if (strcmp(c_string, "infinity") == 0)
        {
            the_double = infinity();
        }
        else if (strcmp(c_string, "°") == 0)
        {
            the_double = infinity();
        }
        else if (strcmp(c_string, "i") == 0)
        {
            the_double = infinity();
        }
        else
        {
            // the_double = atof(c_string);
            the_double = strtod(c_string, 0);
        }
    }
    return (the_double);
} //StringToDouble


// if the string is one of :
//  "yes","true","1"   (case insensitive)
// then this function returns true.
//  "true"  (case insensitive)
//  a non-zero integer
//
bool StringToBool (const RmString & s, bool throwOnFail)
{
    RmString lowercasestr;
    for ( RmString::const_iterator liter = s.begin();
          liter!=s.end() ; ++liter )
    {
        char c = tolower(*liter);
        lowercasestr.append(1,c);
    }

    if ( (lowercasestr == "true") ||
         (lowercasestr == "yes" ) ||
         (lowercasestr == "t"   ) ||
         (lowercasestr == "y"   ) ||
         (lowercasestr == "1"   )
       )
    {
        return true;
    }

    // if the user is really interested in being sure the input was valid,
    // perform some extra checking to ensure the non-true value was actually
    // ok
    if (throwOnFail)
    {
        if ( (lowercasestr == "false") ||
             (lowercasestr == "no"   ) ||
             (lowercasestr == "f"    ) ||
             (lowercasestr == "n"    ) ||
             (lowercasestr == "0"    )
           )
        {
            return false;
        }
        else
        {
            throw StringConversionException("Invalid string passed to StringToBool",s);
        }
    }
    
    return (false);
}

// feel free to optimise this, it was coded while exhausted
// It actually looks optimal enough to me - i don't think it would be worth
// making a loop, for the having to calculate the shift amount and mask during
// every iteration
OSType StringToOSType(const RmString & s)
{
    if (s.size() < sizeof(OSType))
    {
        wxLogError(wxT("String not long enough for conversion to OSType: \"%s\""),s.c_str());
    }

    OSType byte0 = (s[0] << 24) & 0xFF000000;
    OSType byte1 = (s[1] << 16) & 0x00FF0000;
    OSType byte2 = (s[2] << 8)  & 0x0000FF00;
    OSType byte3 = (s[3])       & 0x000000FF;

    OSType value = byte0 | byte1 | byte2 | byte3;

    return value;
}

ViewDirection StringToViewDirection(const RmString & s)
{
    ViewDirection viewdir = NoViewDirection;

    if (StringEndsWith(s,"Front"))
    {
        viewdir = vFront;
    }
    else if (StringEndsWith(s,"Side"))
    {
        viewdir = vSide;
    }
    else if (StringEndsWith(s,"Top"))
    {
        viewdir = vTop;
    }

    return viewdir;
}

bool StringEndsWith(const std::string & s, const char * ends_with)
{
    size_t slen = s.length();
    size_t withlen = strlen(ends_with);

    // expected substring is larger than given string
    if (withlen>slen) { return false; }

    size_t pos = s.rfind(ends_with);
    return ( pos == (slen-withlen));
}

bool StringBeginsWith(const std::string & s, const char * begins_with)
{
    return (s.find(begins_with) == 0);
}


bool StringContains(const std::string & s, const char * contains)
{
    return (s.find(contains) != std::string::npos);
}

bool _IsWhiteSpace(char c);
bool _IsWhiteSpace(char c)
{
    return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
}

void TrimRight(char * s)
{
    size_t len = strlen(s);
    char * c = s;

    while ( ( _IsWhiteSpace(*c) ) && ( c != 0 ) )
    {
        c++;
        len--;
    }
    
    if ( c != s )
    {
        memmove(s, c, len+1);
    }
}


void TrimLeft(char * s)
{
    size_t len = strlen(s);
    char * c = s+len-1;
    
    while ( ( _IsWhiteSpace(*c) ) && ( c >= s ) )
    {
        *c = 0;
        c--;
    }
}


void Trim(char * s)
{
    TrimLeft(s);
    TrimRight(s);
}

wxString TrimAll(const wxString & s)
{
    wxString s_="";

    for ( wxString::const_iterator iter = s.begin();
          iter!=s.end(); ++iter )
    {
        if (!_IsWhiteSpace(*iter))
        {
            s_.append(1,(*iter));
        }
    }
    return s_;
}


void OSTypeToString(OSType ost, RmString & s)
{
    s = OSTypeToString(ost);
}

RmString OSTypeToString(OSType ost)
{
    RmString s;
    four_byte_union raw;
    raw.ul = ost;
#if defined WIN32
    for ( int i=3 ; i>=0 ; i-- )
    {
        s.append(1,raw.chars[i]);
    }
#else // #if defined WIN32
#pragma error Need to re-implement for BigEndian!
    for ( int i=0 ; i<4 ; i++ )
    {
        s.append(1,raw.chars[i]);
    }
#endif

    return s;
}

RmString ToString(bool b, BooleanStringRepresentation bsr)
{
    switch (bsr)
    {
        case bsrYesNo:
            return ((b) ? "Yes" : "No");
            break;
        case bsryesno:
            return ((b) ? "yes" : "no");
            break;
        case bsrTrueFalse:
            return ((b) ? "True" : "False");
            break;
        case bsrtruefalse:
            return ((b) ? "true" : "false");
            break;
        case bsr10:
            return ((b) ? "1" : "0");
            break;
        default:
            break;
    }

    return "false";
}

RmString ToString(long l)
{
    std::ostringstream s;
    s << l;
    return s.str();
}


RmString ToString(unsigned long ul)
{
    std::ostringstream s;
    s << ul;
    return s.str();
}


RmString ToString(double_cs d, int precision, bool isFixed)
{
    std::ostringstream s;
    if (isFixed)
    {
        s << std::fixed;
    }
    s << std::setprecision(precision) << d;
    return s.str();
}


bool NewBooleanState(BooleanStateChange stateChange, bool oldState)
{
    bool new_boolean_state = oldState;

    switch (stateChange)
    {
        case On:
            new_boolean_state = true;
            break;
        case Off:
            new_boolean_state = false;
            break;
        case Toggle:
            new_boolean_state = (!oldState);
            break;
        default:
            RmAssert(false, "Invalid value specified for BooleanStateChange");
            break;
    }

    return new_boolean_state;
}


RmString NewUnitsString(SummaryRoutine sr)
{
    long index = STRLxuNull;
    RmString ss("");

    switch (sr)
    {
        case srRootLength:
            index=STRLxuCentimetres;
            break;
        case srRootLengthDensity:
            index=STRLxuCentimetresPerMl;
            break;
        case srRootTipCount:
            index=STRLxuTipCount;
            break;
        case srRootBranchCount:
            index=STRLxuBranchCount;
            break;
        case srRootVolume:
            index=STRLxuCubicCentimetres;
            break;
        case srRootSurfaceArea:
            index=STRLxuSquareCentimetres;
            break;
        default:
            index=STRLxuNull;
            break;
    }

    CS_GetIndString(ss, STRLUnitStrings, index);
    return (ss);
}

SummaryRoutineGroup DetermineSummaryTypeArrayIndex(SummaryRoutine sr)
{
    SummaryRoutineGroup index = summary_type_root_length_dependent;

    switch (sr)
    {
        case srRootLength:
            index=summary_type_root_length_dependent;
            break;
        case srRootLengthDensity:
            index=summary_type_root_length_dependent;
            break;
        case srRootTipCount:
            index=summary_type_tip_count_dependent;
            break;
        case srRootTipCountDensity:
            index=summary_type_tip_count_dependent;
            break;
        case srRootBranchCount:
            index=summary_type_branch_count_dependent;
            break;
        case srRootBranchCountDensity:
            index=summary_type_branch_count_dependent;
            break;
        case srRootVolume:
            index=summary_type_root_length_dependent;
            break;
        case srRootSurfaceArea:
            index=summary_type_root_length_dependent;
            break;
        default:
            /* srNone */
            index=summary_type_root_length_dependent;
            break;
    }

    return (index);
}


/* ProcessName2ClassName

Each of the 'Prcs' resources are named for human consumption.
The standard treatment of these names is to strip the spaces from
between any words, then add a 'P' to the beginning, then use the
resulting string as the class name for the process, if required.
*/
void ProcessName2ClassName(const RmString & processname, RmString & classname)
{
    if ( !(processname == "PlantCoordinator") )
    {
        classname = "P";
    }

    classname.append(processname);
}

/**
 *    MSA Endianness-detection function
 *        Basis code by Danny Kalev,
 *        available at http://www.devx.com/tips/Tip/5278
 */
bool IsLittleEndian()
{
    union probe
    { 
        unsigned int num;
        unsigned char bytes[sizeof(unsigned int)];
    };

    probe p = { 1U }; //initialize first member of p with unsigned 1
    return (p.bytes[0] == 1U); //in a big endian architecture, p.bytes[0] equals 0 
}

/**
 *  MSA Function for converting a double (usu. a CharacteristicValue)
 *        to a DF3Voxel (i.e. 1, 2 or 4-byte uint) representation
 */
DF3Voxel ConvertToDF3Voxel(double d, const double dmin, const double dmax)
{
    if(d<dmin) return 0;
    if(dmin>=dmax || d>dmax)    return 1;
    switch(sizeof(DF3Voxel))
    {
        case 1:
            d = 0xFF*((d-dmin)/(dmax-dmin));
            break;
        case 2:
            d = 0xFFFF*((d-dmin)/(dmax-dmin));
            break;
        case 4:
            d = 0xFFFFFFFF*((d-dmin)/(dmax-dmin));
            break;
        default:
            // Should never get here
            return 0;
    }
    return static_cast<DF3Voxel>(d);
}

/**
 *    MSA Functions for converting byte order
 *        Basis code (extern'd C macros above) by Paul Bourke,
 *        available at http://local.wasp.uwa.edu.au/~pbourke/miscellaneous/df3/
 */    
/*    Ignoring warning C4739 "reference to variable 'voxel' exceeds its storage space".
    FIX_INT causes this warning when compiling with sizeof(DF3Voxel)<4,
    but FIX_INT will never be called in that case.
*/
#pragma warning( push )
#pragma warning( disable: 4739 )
DF3Voxel FixEndianness(DF3Voxel voxel, const bool & isLittleEndian)
{            

    if(isLittleEndian)
    {
        if(sizeof(voxel)==2)
        {
            FIX_SHORT(voxel);
        }
        else if(sizeof(voxel)==4)
        { 
            FIX_INT(voxel);
        }
        else    RmAssert(sizeof(voxel)==1, "DF3 voxels must be either 1, 2 or 4 bytes");
    }
    return voxel;
}
#pragma warning( pop )



} // namespace Utility

} // namespace rootmap

