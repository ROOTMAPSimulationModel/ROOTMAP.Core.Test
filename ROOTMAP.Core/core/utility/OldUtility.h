#ifndef Utility_H
#define Utility_H
/////////////////////////////////////////////////////////////////////////////
// Name:        Utility.h
// Purpose:     Declaration of functions in the rootmap::Utility namespace
// Created:     1993 (approximately)
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////


#include "core/common/Structures.h"
#include "core/common/Constants.h"
#include "core/common/RmString.h"
#include "core/log/Logger.h"
#include "core/scoreboard/Dimension.h"
#include <string>
#include <limits>

#include "core/common/ExtraStuff.h"

#include "wx/string.h"

//const long int MaxDecimalPlaces = 5;
//#define MaxDecimalPlaces = 5;

//class Process;
class CBitMap;
//class CString;

/* Some general mathematical/logical routines */
#define isEven(v)  ( ( v & 1 ) == 0 )
#define isOdd(y)  ( y & 1 )
#define CS_HiWord(x) ((unsigned long)x>>16)
#define CS_LoWord(x) ((unsigned long)x & 0xFFFF)

namespace rootmap
{

namespace Utility
{

long MergeWords(short hi, short lo);
Boolean isInfinite (double_cs check_number);
double_cs infinity();
//Boolean isInfinite (long check_number);

template <class ABS_TYPE>
ABS_TYPE CSAbs( ABS_TYPE x )
{
    if (x < 0)
        return -x;
    else
        return x;
}

template <class SIGN_TYPE>
SIGN_TYPE CSSign( SIGN_TYPE x )
{
    return (x/(CSAbs(x)));
}

template <class MAX_TYPE>
inline MAX_TYPE CSMax(MAX_TYPE x, MAX_TYPE y)
{
    return (x < y) ? y : x;
}

template <class MIN_TYPE>
inline MIN_TYPE CSMin(MIN_TYPE x, MIN_TYPE y)
{
    return (x < y) ? x : y;
}

/**
 * Template function to restrict a value between an upper and lower bound
 */
template <class RESTRICT_TYPE>
inline RESTRICT_TYPE CSRestrict(RESTRICT_TYPE r_value, RESTRICT_TYPE r_min, RESTRICT_TYPE r_max)
{
    if (r_value < r_min) return r_min;
    
    if (r_value > r_max) return r_max;
    
    return r_value;
}

/**
 * Explicit specialisation for double_cs of the CSAbs template function
 */
template <>
inline double_cs CSAbs(double_cs x)
{
    return fabs(x);
}

double_cs CalculateNewScale(double_cs oldScale, short int oldH, short int oldV, short int newH, short int newV);

/**
 * Basic wrapper for the std::rand() function
 */
int Random();

/**
 * Provides a random floating-point value between 0 and 1
 */
double_cs Random1();

/**
 * Provides a random floating-point value within the given range
 */
double_cs RandomRange(double_cs range_max, double_cs range_min=0.0);
/**
 * Provides a random integer value within the given range
 */
signed long RandomRange(signed long range_max, signed long range_min=0);

/* Files */
//#pragma message (HERE "TODO Remove use of MacOS SFReply")
//void CopySFReply (SFReply fromSF, SFReply *toSF);
Boolean CS_GetString(short string_resource_id, RmString & rmstring);
Boolean CS_GetIndString(RmString & rmstring, long strListID, long str_index);

// String Input Conversion
Dimension StringToDimension(char dim_string);
double_cs StringToDouble (const char * c_string);
bool StringToBool (const RmString & s, bool throwOnFail=false);
OSType StringToOSType(const RmString & s);
ViewDirection StringToViewDirection(const RmString & s);

// String manipulation and verification
bool StringEndsWith(const std::string & s, const char * ends_with);
bool StringBeginsWith(const std::string & s, const char * begins_with);
bool StringContains(const std::string & s, const char * contains);

/**
 * Trims whitespace from the end of the string
 */
void TrimRight(char * s);

/**
 * Trims whitespace from the beginning of the string
 */
void TrimLeft(char * s);

/**
 * Trims both left and right
 */
void Trim(char * s);

/**
 * Trims ALL whitespace within the string and returns a trim'd copy
 */
wxString TrimAll(const wxString & s);

/**
 * OSType conversions
 */
void OSTypeToString(OSType ost, std::string & s);
RmString OSTypeToString(OSType ost);

enum BooleanStringRepresentation
{
    bsrYesNo,
    bsryesno,
    bsrTrueFalse,
    bsrtruefalse,
    bsr10
};
/**
 * convert long, unsigned long and floating point to string
 */
RmString ToString(bool b, BooleanStringRepresentation bsr=bsrTrueFalse);
RmString ToString(long l);
RmString ToString(unsigned long l);
RmString ToString(double_cs, int precision=5, bool isFixed=true);

/**
 *
 */
bool NewBooleanState(BooleanStateChange stateChange, bool oldState);

// Process String Conversion
RmString NewUnitsString(SummaryRoutine sr);
SummaryRoutineGroup DetermineSummaryTypeArrayIndex(SummaryRoutine sr);
void ProcessName2ClassName(const RmString & processname, RmString & classname);

// Will be used in DF3 format output for raytracing.
// Valid int sizes are 1 byte, 2 bytes (usu. short) and 4 bytes (usu. int); all unsigned
typedef unsigned int DF3Voxel;

// Endianness detection
bool IsLittleEndian();
// Converts to network byte order, if little-endian
DF3Voxel FixEndianness(DF3Voxel voxel, const bool & isLittleEndian);
DF3Voxel ConvertToDF3Voxel(double d, const double dmin, const double dmax);

RootMapLoggerDeclaration();

} // namespace Utility

inline double_cs Utility::infinity()
{
    return std::numeric_limits<double_cs>::infinity();
}

} // namespace rootmap

#endif // #ifndef Utility_H
