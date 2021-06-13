/////////////////////////////////////////////////////////////////////////////
// Name:        TXUtility.cpp
// Purpose:     Implementation of the TXUtility class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/tinyxmlconfig/TXUtility.h"

#include "core/utility/Utility.h"
#include "core/utility/StringParserUtility.h"
#include "simulation/process/interprocess_communication/PostOffice.h"


namespace rootmap
{
    //     TXUtility::TXUtility()
    //     {
    //     }
    // 
    //     TXUtility::~TXUtility()
    //     {
    //     }

#define THROW_ON_NULL(t) if (t==__nullptr) { throw TXUtilityException("Failed to retrieve child element text", "N/A", childElemTag); }

    // MSA 10.02.03 Suppressing warning about intentional assignment within conditional
#pragma warning( push )
#pragma warning( disable: 4706 )
    const char* TXUtility::getChildElementText
    (TiXmlNode* elem, const char* childElemTag, bool throwOnNull)
        // throw TXUtilityException
    {
        TiXmlNode* node = 0;
        const char* textValue = 0;
        TiXmlElement* child = elem->FirstChildElement(childElemTag);

        if (child != 0)
        {
            while (node = child->IterateChildren(node))
            {
                TiXmlText* text = node->ToText();
                if (text != 0)
                {
                    textValue = text->Value();
                    break;
                }
            }
        }

        if (throwOnNull) { THROW_ON_NULL(textValue) }

        return textValue;
    } // getElementText
#pragma warning(pop)

    // MSA 10.02.03 Suppressing warning about intentional assignment within conditional
#pragma warning( push )
#pragma warning( disable: 4706 )
    bool TXUtility::setChildElementText
    (TiXmlNode* elem, const char* childElemTag, const char* textToSet, bool throwOnNull)
        // throw TXUtilityException
    {
        TiXmlNode* node = 0;
        const char* textValue = 0;

        TiXmlElement* child = elem->FirstChildElement(childElemTag);

        if (child != 0)
        {
            while (node = child->IterateChildren(node))
            {
                TiXmlText* text = node->ToText();
                if (text != 0)
                {
                    node->SetValue(textToSet);
                    return true;
                }
            }
        }

        if (throwOnNull) { THROW_ON_NULL(textValue) }

        return false;
    } // getElementText
#pragma warning(pop)

    bool TXUtility::hasChildElement(TiXmlNode * elem, const char * childElemTag) throw(TXUtilityException)
    {
        const char* text = getChildElementText(elem, childElemTag, false);
        return 0 != text;
    }

    double TXUtility::getChildElementDouble
    (TiXmlNode* elem, const char* childElemTag, bool throwOnNull, double defaultOnNull)
        throw(TXUtilityException)
    {


        const char* text = getChildElementText(elem, childElemTag, throwOnNull);
        if (0 == text)
        {
            return defaultOnNull;
        }

        return Utility::StringToDouble(text);
    }

    int TXUtility::getChildElementInt
    (TiXmlNode* elem, const char* childElemTag, bool throwOnNull, int defaultOnNull)
        throw(TXUtilityException)
    {
        const char* text = getChildElementText(elem, childElemTag, throwOnNull);
        if (0 == text)
        {
            return defaultOnNull;
        }

        return atoi(text);
    }

    unsigned int TXUtility::getChildElementUnsignedInt
    (TiXmlNode* elem, const char* childElemTag, bool throwOnNull, int defaultOnNull)
        throw(TXUtilityException)
    {
        const char* text = getChildElementText(elem, childElemTag, throwOnNull);
        if (0 == text)
        {
            return defaultOnNull;
        }

        return (unsigned int)strtoul(text, __nullptr, 10);
    }

    bool TXUtility::getChildElementBool
    (TiXmlNode* elem, const char* childElemTag, bool throwOnNull, bool defaultOnNull)
        throw(TXUtilityException)
    {
        const char* text = getChildElementText(elem, childElemTag, throwOnNull);
        if (0 == text)
        {
            return defaultOnNull;
        }

        bool value = defaultOnNull;
        try
        {
            value = Utility::StringToBool(text, true);
        }
        catch (StringConversionException& /*sce*/)
        {
            throw TXUtilityException("Invalid boolean string value in element", text, childElemTag);
        }
        // we only throw TXUtilityException so really must catch ALL others
        catch (...)
        {
            throw TXUtilityException("Invalid boolean string value in element", text, childElemTag);
        }

        return value;
    }


    bool TXUtility::getChildElementTextArray
    (TiXmlNode* elem, const char* childElemTag, std::vector<std::string>& textArray, bool throwOnNull)
        throw(TXUtilityException)
    {
        const char* text = getChildElementText(elem, childElemTag, throwOnNull);
        if (0 == text)
        {
            return false;
        }

        StringParserUtility::ParseCommaSeparatedStrings(text, textArray);
        return (!textArray.empty());
    }

    bool TXUtility::getChildElementDoubleArray
    (TiXmlNode* elem, const char* childElemTag, std::vector<double>& doubleArray, bool throwOnNull)
        throw(TXUtilityException)
    {
        const char* text = getChildElementText(elem, childElemTag, throwOnNull);
        if (0 == text)
        {
            return false;
        }

        StringParserUtility::ParseCommaSeparatedDoubles(text, doubleArray);
        return (!doubleArray.empty());
    }

    bool TXUtility::getChildElementIntArray
    (TiXmlNode* elem, const char* childElemTag, std::vector<int>& intArray, bool throwOnNull)
        throw(TXUtilityException)
    {
        const char* text = getChildElementText(elem, childElemTag, throwOnNull);
        if (0 == text)
        {
            return false;
        }

        StringParserUtility::ParseCommaSeparatedInts(text, intArray);
        return (!intArray.empty());
    }


    ProcessTime_t TXUtility::getChildElementDateTime(TiXmlNode* elem, const char* childElemTag)
        throw(TXUtilityException)
    {
        std::vector<int> dt_vec;
        TXUtility::getChildElementIntArray(elem, childElemTag, dt_vec, true);

        if (6 != dt_vec.size())
        {
            const char* bad_str = getChildElementText(elem, childElemTag, false);
            throw TXUtilityException("Bad DateTime string", bad_str, childElemTag);
        }

        return (PostOffice::CalculateDateTime(dt_vec[0], dt_vec[1], dt_vec[2], dt_vec[3], dt_vec[4], dt_vec[5]));
    }


    ProcessTime_t TXUtility::getChildElementTimeSpan(TiXmlNode* elem, const char* childElemTag)
        throw(TXUtilityException)
    {
        std::vector<int> dt_vec;
        TXUtility::getChildElementIntArray(elem, childElemTag, dt_vec, true);

        if (6 != dt_vec.size())
        {
            const char* bad_str = getChildElementText(elem, childElemTag, false);
            throw TXUtilityException("Bad TimeSpan string", bad_str, childElemTag);
        }

        return (PostOffice::CalculateTimeSpan(dt_vec[0], dt_vec[1], dt_vec[2], dt_vec[3], dt_vec[4], dt_vec[5]));
    }


    RootMapLoggerDefinition(TXUtilityException);


    TXUtilityException::TXUtilityException
    (const char* reason, const char* bad_string, const char* bad_tag)
        : RmException(reason)
        , m_badString(bad_string)
        , m_badTag(bad_tag)
    {
        RootMapLoggerInitialisation("rootmap.TXUtilityException");
        LOG_ERROR << "TXUtilityException constructed {Reason:" << reason
            << ", BadString:" << bad_string
            << ", BadTag:" << bad_tag << "}";
    }

    const char* TXUtilityException::whatString() const
    {
        return m_badString.c_str();
    }

    const char* TXUtilityException::whatTag() const
    {
        return m_badTag.c_str();
    }
} /* namespace rootmap */
