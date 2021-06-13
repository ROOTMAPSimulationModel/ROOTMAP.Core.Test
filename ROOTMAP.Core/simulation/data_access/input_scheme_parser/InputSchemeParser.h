#ifndef InputSchemeParser_H
#define InputSchemeParser_H
/////////////////////////////////////////////////////////////////////////////
// Name:        InputSchemeParser.h
// Purpose:     Declaration of the InputSchemeParser class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/interface/ScoreboardDataDAI.h"
#include "core/log/Logger.h"
#include <map>
#include <list>

namespace rootmap
{
    class InputSchemeParser
    {
    public:
        /**
         * Parses the scheme which contents are defined by the given string
         */
        void ParseScheme(const char* scheme_str);

        /**
         * Parses the assignment string (e.g. "value=0.5;", "X=9", "[0,1,5]", etc)
         * @param A raw input string which must be the textual representation of an assignment
           * @param bool endofLine - true if a semicolon was found, otherwise false
         */
        void parseAssignmentStatement(const std::string& rawString, bool& EOLFound);

        /**
         * Sets the current element's type
         * @param A raw input string which must be the textual representation of an element type
         */
        void setElementType(const std::string& rawString);

        /**
         * Sets the current element's type
         * @param the type of scheme element
         */
        void setElementType(SchemeContentElementType elementType);
        /**
         * adds a dimension specification to the list
         * @param dim the dimension specifier [X|Y|Z]
         */
        void accumulateDimension(char dim, long value);
        /**
         * adds a value - although there is only one...
         * @param
         */
        void accumulateValue(double value);
        /**
         * adds an array of [x,y,z] to the list
         * @param
         */
        void accumulateArray(long x, long y, long z);

        /**
         * announces the start of parsing
         *
         */
        void startParse();
        /**
         * announces the end of parsing
         *
         */
        void endParse();

        /**
         * Announces the end of a statement (ie. semicolon)
         *
         */
        void endStatement();

        /**
         * Accessor for the parsed elements
         */
        SchemeContentElementList& getElementList();

        /**
         * Access to the instance of this singleton.
         *
         * @return the instance
         */
        static InputSchemeParser& instance();

    private:

        InputSchemeParser();
        virtual ~InputSchemeParser();

        RootMapLoggerDeclaration();

        ///
        /// The element being populated by the current statement
        SchemeContentElement m_element;

        ///
        /// list of elements in parsed statements for the current scheme
        SchemeContentElementList m_elementList;
    }; // class InputSchemeParser

    inline SchemeContentElementList& InputSchemeParser::getElementList()
    {
        return m_elementList;
    }
} /* namespace rootmap */

#endif // #ifndef InputSchemeParser_H
