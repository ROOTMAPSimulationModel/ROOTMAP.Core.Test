#ifndef TXUtility_H
#define TXUtility_H
/////////////////////////////////////////////////////////////////////////////
// Name:        TXUtility.h
// Purpose:     Declaration of the TXUtility class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/common/Exceptions.h"
#include "core/common/Types.h"
#include "core/log/Logger.h"
#include "simulation/common/Types.h"

#include "tinyxml.h"

#include <vector>

// warning C4290: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
#pragma warning( disable : 4290 )

namespace rootmap
{
    class TXUtilityException;

    class TXUtility
    {
    public:

        /**
         *
         * @param
         * @return
         */
        static const char* getChildElementText(TiXmlNode* elem, const char* childElemTag, bool throwOnNull = true) throw(TXUtilityException);

        static double getChildElementDouble(TiXmlNode* elem, const char* childElemTag, bool throwOnNull = true, double defaultOnNull = 0.0) throw(TXUtilityException);

        static int getChildElementInt(TiXmlNode* elem, const char* childElemTag, bool throwOnNull = true, int defaultOnNull = 0) throw(TXUtilityException);

        static unsigned int getChildElementUnsignedInt(TiXmlNode* elem, const char* childElemTag, bool throwOnNull = true, int defaultOnNull = 0) throw(TXUtilityException);

        static bool getChildElementBool(TiXmlNode* elem, const char* childElemTag, bool throwOnNull = true, bool defaultOnNull = false) throw(TXUtilityException);

        static bool getChildElementTextArray(TiXmlNode* elem, const char* childElemTag, std::vector<std::string>& textArray, bool throwOnNull = true) throw(TXUtilityException);

        static bool getChildElementDoubleArray(TiXmlNode* elem, const char* childElemTag, std::vector<double>& doubleArray, bool throwOnNull = true) throw(TXUtilityException);

        static bool getChildElementIntArray(TiXmlNode* elem, const char* childElemTag, std::vector<int>& intArray, bool throwOnNull = true) throw(TXUtilityException);

        static ProcessTime_t getChildElementDateTime(TiXmlNode* elem, const char* childElemTag) throw(TXUtilityException);

        static ProcessTime_t getChildElementTimeSpan(TiXmlNode* elem, const char* childElemTag) throw(TXUtilityException);

        static bool setChildElementText(TiXmlNode* elem, const char* childElemTag, const char* textToSet, bool throwOnNull = true) throw(TXUtilityException);

        static bool hasChildElement(TiXmlNode* elem, const char* childElemTag) throw(TXUtilityException);
    private:

        TXUtility();
        ~TXUtility();

        ///
        /// member declaration
    }; // class TXUtility

    class TXUtilityException : public RmException
    {
    public:
        TXUtilityException(const char* reason, const char* bad_string, const char* bad_tag);
        TXUtilityException(const TXUtility&);
        virtual const char* whatString() const;
        virtual const char* whatTag() const;

    private:
        RootMapLoggerDeclaration();

        std::string m_badString;
        std::string m_badTag;

        TXUtilityException& operator=(const TXUtilityException&);
    };
} /* namespace rootmap */

#endif // #ifndef TXUtility_H
