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

#include "core/log/Logger.h"
#include "core/scoreboard/Dimension.h"
#include <string>
#include <limits>
#include <bitset>

#include "core/common/ExtraStuff.h"
#include "core/utility/RmSHA1.h"

#include "wx/string.h"
#include "wx/datetime.h"

class CBitMap;
class wxFileName;

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
        Boolean isInfinite(double check_number);
        double infinity();
        //Boolean isInfinite (long check_number);

        template <class ABS_TYPE>
        ABS_TYPE CSAbs(ABS_TYPE x)
        {
            if (x < 0)
                return -x;
            else
                return x;
        }

        template <class SIGN_TYPE>
        SIGN_TYPE CSSign(SIGN_TYPE x)
        {
            return (x / (CSAbs(x)));
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
         *    Template function for swapping two objects. Objects must provide an assigment operator, of course
         */
        template <class SWAP_TYPE>
        inline void CSSwap(SWAP_TYPE& a, SWAP_TYPE& b)
        {
            SWAP_TYPE temp = a;
            a = b;
            b = temp;
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
         * Explicit specialisation for double of the CSAbs template function
         */
        template <>
        inline double CSAbs(double x)
        {
            return fabs(x);
        }

        double CalculateNewScale(double oldScale, short int oldH, short int oldV, short int newH, short int newV);

        /**
         * Basic wrapper for the std::rand() function
         */
        int Random();

        /**
         * Provides a random floating-point value between 0 and 1
         */
        double Random1();

        /**
         *    Provides a random boolean value
         */
        bool RandomBool();

        /**
         * Provides a random floating-point value within the given range
         */
        double RandomRange(double range_max, double range_min = 0.0);
        /**
         * Provides a random integer value within the given range
         */
        signed long RandomRange(signed long range_max, signed long range_min = 0);

        /* Files */
        //#pragma message (HERE "TODO Remove use of MacOS SFReply")
        //void CopySFReply (SFReply fromSF, SFReply *toSF);
        Boolean CS_GetString(short string_resource_id, std::string& rmstring);
        Boolean CS_GetIndString(std::string& rmstring, long strListID, long str_index);

        // String Input Conversion
        Dimension StringToDimension(char dim_string);
        double StringToDouble(const char* c_string);
        bool StringToBool(const std::string& s, bool throwOnFail = false);
        OSType StringToOSType(const std::string& s);
        ViewDirection StringToViewDirection(const std::string& s);
        std::string ViewDirectionToString(const ViewDirection& vd);

        // String manipulation and verification
        bool StringEndsWith(const std::string& s, const char* ends_with);
        bool StringBeginsWith(const std::string& s, const char* begins_with);
        bool StringContains(const std::string& s, const char* contains);

        /**
         * Trims whitespace from the end of the string
         */
        void TrimRight(char* s);

        /**
         * Trims whitespace from the beginning of the string
         */
        void TrimLeft(char* s);

        /**
         * Trims both left and right
         */
        void Trim(char* s);

        /**
         * Trims ALL whitespace within the string and returns a trim'd copy
         */
        wxString TrimAll(const wxString& s);

        /**
         * OSType conversions
         */
        void OSTypeToString(OSType ost, std::string& s);
        std::string OSTypeToString(OSType ost);

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
        std::string ToString(bool b, BooleanStringRepresentation bsr = bsrTrueFalse);
        std::string ToString(long l);
        std::string ToString(unsigned long l);
        std::string ToString(double, int precision = 5, bool isFixed = true);
        std::string ToString(const size_t s);

        /**
         *
         */
        bool NewBooleanState(BooleanStateChange stateChange, bool oldState);

        // Process String Conversion
        std::string NewUnitsString(SummaryRoutine sr);
        SummaryRoutineGroup DetermineSummaryTypeArrayIndex(SummaryRoutine sr);
        void ProcessName2ClassName(const std::string& processname, std::string& classname);

        // Will be used in DF3 format output for raytracing.
        // Valid int sizes are 1 byte, 2 bytes (usu. short) and 4 bytes (usu. int); all unsigned
        typedef unsigned int DF3Voxel;

        // Endianness detection
        bool IsLittleEndian();
        // Converts to network byte order, if little-endian
        DF3Voxel FixEndianness(DF3Voxel voxel, const bool& isLittleEndian);
        DF3Voxel ConvertToDF3Voxel(double d, const double dmin, const double dmax);

        // MSA 11.05.26 New utility methods for calculating the SHA-1 fingerprint of a file or string
        SHA1Fingerprint GetFingerprint(const wxFileName& file);
        SHA1Fingerprint GetFingerprint(std::string& str);

        // MSA 11.07.13 New utility methods for logging function performance
        const wxDateTime StartTimer();
        const double StopTimer(const wxDateTime& timer);

        RootMapLoggerDeclaration();
    } // namespace Utility

    inline double Utility::infinity()
    {
        // This assert should ensure that it is safe to negate infinity.
        static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 required");
        return std::numeric_limits<double>::infinity();
    }

    inline int Utility::Random()
    {
        return (std::rand());
    }

    inline double Utility::Random1()
    {
        return (((double)std::rand()) / ((double)RAND_MAX));
    }

    inline bool Utility::RandomBool()
    {
        return ((std::rand() % 2) == 0);
    }

    inline double Utility::RandomRange(double range_max, double range_min)
    {
        return (((double)std::rand() / (double)RAND_MAX) * (range_max - range_min) + range_min);
    }

    inline signed long Utility::RandomRange(signed long range_max, signed long range_min)
    {
        return (std::floor(RandomRange((double)range_max, (double)range_min)));
    }

    inline const wxDateTime Utility::StartTimer()
    {
        return wxDateTime::UNow();
    }

    inline const double Utility::StopTimer(const wxDateTime& timer)
    {
        const wxDateTime endtime = wxDateTime::UNow();
        const wxTimeSpan difftime = endtime.Subtract(timer);
        return (difftime.GetSeconds().ToDouble() + 0.001 * difftime.GetMilliseconds().ToDouble());
    }
} /* namespace rootmap */

#endif // #ifndef Utility_H
