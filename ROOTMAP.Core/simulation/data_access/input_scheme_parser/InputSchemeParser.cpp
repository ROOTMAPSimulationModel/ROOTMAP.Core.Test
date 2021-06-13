/////////////////////////////////////////////////////////////////////////////
// Name:        InputSchemeParser.cpp
// Purpose:     Implementation of the InputSchemeParser class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "InputSchemeParser.h"
#include "core/common/Exceptions.h"
#include "core/utility/Utility.h"
#include "core/common/RmAssert.h"
#include "boost/tokenizer.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"


namespace rootmap
{
    RootMapLoggerDefinition(InputSchemeParser);

    typedef boost::char_separator<char> TokenSeparator;

    InputSchemeParser& InputSchemeParser::instance()
    {
        static InputSchemeParser isp;
        return isp;
    }

    InputSchemeParser::InputSchemeParser()
    {
        RootMapLoggerInitialisation("rootmap.InputSchemeParser");
    }

    InputSchemeParser::~InputSchemeParser()
    {
    }

    // MSA 10.08.26 Question. What guarantees are made of this scheme string?
    // Does it always start with "set"?
    // Only one directive per string?
    void InputSchemeParser::ParseScheme(const char* scheme_str)
    {
        startParse();

        std::string str(scheme_str);
        TokenSeparator separator(" ");
        boost::tokenizer<TokenSeparator> tok(str, separator);
        for (boost::tokenizer<TokenSeparator>::iterator iter = tok.begin(); iter != tok.end(); ++iter)
        {
            std::string token = *iter;
            if (boost::iequals(token, "set"))
            {
                // First token of the scheme string: "set" or "SET" or "sEt" etc etc.
                ++iter;
                RmAssert(iter != tok.end(), "Malformed scheme string");
                setElementType(*iter);
            }
            else
            {
                // Now, all remaining tokens should be assignment statements (e.g. "value=0.5;", "X=9", "[0,1,5]", etc)
                bool eol = false;
                parseAssignmentStatement(*iter, eol);
                if (eol)
                {
                    endStatement();
                }
            }
        }
    }

    void InputSchemeParser::parseAssignmentStatement(const std::string& rawString, bool& EOLFound)
    {
        EOLFound = rawString.find(";") != std::string::npos;

        TokenSeparator separator("=;");
        boost::tokenizer<TokenSeparator> tok(rawString, separator);
        boost::tokenizer<TokenSeparator>::iterator iter = tok.begin();

        char dim = NoDimension;
        if (boost::iequals(*iter, "X")) dim = 'X';
        else if (boost::iequals(*iter, "Y")) dim = 'Y';
        else if (boost::iequals(*iter, "Z")) dim = 'Z';

        // First case: assignment to a dimension (i.e. plane, row or column)
        if (dim != NoDimension)
        {
            ++iter;
            RmAssert(iter != tok.end(), "Malformed scheme string");
            try
            {
                accumulateDimension(dim, boost::lexical_cast<long>(*iter));
            }
            catch (boost::bad_lexical_cast&)
            {
                RmAssert(false, "Bad cast from string to numeric representation of value");
            }
            return;
        }

        // Second case: this is the actual value assignment.
        else if (boost::iequals(*iter, "value"))
        {
            ++iter;
            RmAssert(iter != tok.end(), "Malformed scheme string");
            try
            {
                accumulateValue(boost::lexical_cast<double>(*iter));
            }
            catch (boost::bad_lexical_cast&)
            {
                RmAssert(false, "Bad cast from string to numeric representation of value");
            }
            return;
        }
        // Third case: assignment to a box or boxen.
        else
        {
            RmAssert(m_element.m_elementType == sceBox || m_element.m_elementType == sceBoxes, "Box declaration found in non-box scheme string");
            TokenSeparator boxSep("[,]");
            boost::tokenizer<TokenSeparator> boxTok(*iter, boxSep);
            boost::tokenizer<TokenSeparator>::iterator boxIter = boxTok.begin();
            try
            {
                long x = boost::lexical_cast<long>(*boxIter);
                ++boxIter;
                RmAssert(boxIter != boxTok.end(), "Malformed scheme string");
                long y = boost::lexical_cast<long>(*boxIter);
                ++boxIter;
                RmAssert(boxIter != boxTok.end(), "Malformed scheme string");
                long z = boost::lexical_cast<long>(*boxIter);
                accumulateArray(x, y, z);
            }
            catch (boost::bad_lexical_cast&)
            {
                RmAssert(false, "Bad cast from string to numeric representation of value");
            }
            return;
        }

        // unreachable code
        //RmAssert(false,"Invalid assignment statement string");
    }

    void InputSchemeParser::setElementType(const std::string& rawString)
    {
        if (boost::iequals(rawString, "scoreboard"))
        {
            setElementType(rootmap::sceScoreboard);
            return;
        }
        if (boost::iequals(rawString, "plane"))
        {
            setElementType(rootmap::scePlane);
            return;
        }
        if (boost::iequals(rawString, "row") || boost::iequals(rawString, "column"))
        {
            setElementType(rootmap::sceRowColumn);
            return;
        }
        if (boost::iequals(rawString, "box"))
        {
            setElementType(rootmap::sceBox);
            return;
        }
        if (boost::iequals(rawString, "boxes"))
        {
            setElementType(rootmap::sceBoxes);
            return;
        }
        RmAssert(false, "Invalid scheme string element name");
    }

    void InputSchemeParser::setElementType(SchemeContentElementType elementType)
    {
        m_element.m_elementType = elementType;
    }

    void InputSchemeParser::accumulateDimension(char dim, long value)
    {
        Dimension dimension = Utility::StringToDimension(dim);
        m_element.m_dimensions.push_back(DimensionValuePair(dimension, value));
    }

    void InputSchemeParser::accumulateValue(double value)
    {
        m_element.m_value = value;
    }

    void InputSchemeParser::accumulateArray(long x, long y, long z)
    {
        m_element.m_arrays.push_back(DimensionArray(x, y, z));
    }

    void InputSchemeParser::startParse()
    {
        m_element.m_elementType = sceInvalid;
        m_elementList.clear();
    }

    void InputSchemeParser::endParse()
    {
        for (SchemeContentElementList::iterator iter = m_elementList.begin();
            iter != m_elementList.end(); ++iter)
        {
            LOG_DEBUG << "End of Parse - " << (*iter);
        }
        m_element.m_elementType = sceInvalid;
        m_element.m_arrays.clear();
        m_element.m_dimensions.clear();
        m_element.m_value = 0.0;
    }

    void InputSchemeParser::endStatement()
    {
        if (m_element.m_elementType != sceInvalid)
        {
            //LOG_DEBUG << "End of Statement - " << m_element;

            //
            // Perform sanity validation. No exceptions are thrown, although perhaps they should be ?
            switch (m_element.m_elementType)
            {
            case sceScoreboard:
                if (!m_element.m_arrays.empty())
                {
                    LOG_ERROR << m_element.m_arrays.size() << " arrays specified for Scoreboard statement, eliminating extraneous arrays";
                    m_element.m_arrays.clear();
                }
                if (!m_element.m_dimensions.empty())
                {
                    LOG_ERROR << m_element.m_dimensions.size() << " dimensions specified for Scoreboard statement, eliminating extraneous dimensions";
                    m_element.m_dimensions.clear();
                }
                break;
            case scePlane:
                if (!m_element.m_arrays.empty())
                {
                    LOG_ERROR << m_element.m_arrays.size() << " arrays specified for Plane statement, eliminating extraneous arrays";
                    m_element.m_arrays.clear();
                }
                if (m_element.m_dimensions.size() > 1)
                {
                    LOG_ERROR << m_element.m_dimensions.size() << " dimensions specified for Plane statement, eliminating extraneous dimensions";
                    while (m_element.m_dimensions.size() > 1)
                    {
                        m_element.m_dimensions.pop_back();
                    }
                }
                break;
            case sceRowColumn:
                if (!m_element.m_arrays.empty())
                {
                    LOG_ERROR << m_element.m_arrays.size() << " arrays specified for Row/Column statement, eliminating extraneous arrays";
                    m_element.m_arrays.clear();
                }
                if (m_element.m_dimensions.size() > 2)
                {
                    LOG_ERROR << m_element.m_dimensions.size() << " dimensions specified for Row/Column statement, eliminating extraneous dimensions";
                    while (m_element.m_dimensions.size() > 2)
                    {
                        m_element.m_dimensions.pop_back();
                    }
                }
                break;
            case sceBoxes:
                if (m_element.m_arrays.size() > 2)
                {
                    LOG_ERROR << m_element.m_arrays.size() << " arrays specified for Boxes statement, eliminating extraneous arrays";
                    while (m_element.m_arrays.size() > 2)
                    {
                        m_element.m_arrays.pop_back();
                    }
                }
                if (!m_element.m_dimensions.empty())
                {
                    LOG_ERROR << m_element.m_dimensions.size() << " dimensions specified for Boxes statement, eliminating extraneous dimensions";
                    m_element.m_dimensions.clear();
                }
                break;
            case sceBox:
                if (m_element.m_arrays.size() > 1)
                {
                    LOG_ERROR << m_element.m_arrays.size() << " arrays specified for Boxes statement, eliminating extraneous arrays";
                    while (m_element.m_arrays.size() > 1)
                    {
                        m_element.m_arrays.pop_back();
                    }
                }
                if (!m_element.m_dimensions.empty())
                {
                    LOG_ERROR << m_element.m_dimensions.size() << " dimensions specified for Boxes statement, eliminating extraneous dimensions";
                    m_element.m_dimensions.clear();
                }
                break;
            default:
                throw (new RmException("Bad element type (not even sceInvalid)"));
            }

            m_elementList.push_back(m_element);

            m_element.m_elementType = sceInvalid;
            m_element.m_arrays.clear();
            m_element.m_dimensions.clear();
            m_element.m_value = 0.0;
            //LOG_DEBUG << "element pushed onto list, current cleared:" << m_element;
        }
        else
        {
            throw (new RmException("Invalid input scheme statement"));
        }
    }


    std::ostream& operator<<(std::ostream& ostr, SchemeContentElementType elementType)
    {
        switch (elementType)
        {
        case sceScoreboard:
            ostr << "Scoreboard";
            break;
        case scePlane:
            ostr << "Plane";
            break;
        case sceRowColumn:
            ostr << "Row/Column";
            break;
        case sceBoxes:
            ostr << "Boxes";
            break;
        case sceBox:
            ostr << "Box";
            break;
        case sceInvalid:
            ostr << "Invalid";
            break;
        default:
            ostr << "Unknown";
        }
        return ostr;
    }

    std::ostream& operator<<(std::ostream& ostr, SchemeContentElement element)
    {
        ostr << "Element contents - Type:" << element.m_elementType << ", Dimensions:";
        for (std::list<DimensionValuePair>::iterator iter = element.m_dimensions.begin();
            iter != element.m_dimensions.end();
            ++iter)
        {
            ostr << "[" << (*iter).dimension << "=" << (*iter).dimension_value << "]";
        }

        ostr << ", Arrays:";
        for (std::list<DimensionArray>::iterator iter2 = element.m_arrays.begin();
            iter2 != element.m_arrays.end(); ++iter2)
        {
            ostr << "[" << (*iter2).x << "," << (*iter2).y << "," << (*iter2).z << "]";
        }

        ostr << ", Value:" << element.m_value;
        return ostr;
    }
} /* namespace rootmap */

