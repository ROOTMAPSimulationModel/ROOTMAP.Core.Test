#ifndef StringParserUtility_H
#define StringParserUtility_H

#include "core/common/Types.h"
#include "core/common/ExtraStuff.h"

#include <string>
#include <vector>

namespace rootmap
{
    namespace StringParserUtility
    {
        // any classes that wish to be called back during parsing of strings should
        // derive from this class, declared further down.
        class CommaSeparatedStringParserCallback;

        //
        // These routines do NOT use strtok or derivatives thereof
        //
        //TODO: enable parsing of strings surrounded by quotation marks.
        //
        bool ParseCommaSeparatedDoubles(const char* buffer, std::vector<double>& double_cs_stuff);
        bool ParseCommaSeparatedStrings(const char* buffer, std::vector<std::string>& string_stuff);
        bool ParseCommaSeparatedLongs(const char* buffer, std::vector<long>& long_stuff);
        bool ParseCommaSeparatedInts(const char* buffer, std::vector<int>& int_stuff);

        /**
         * @param strs the vector of strings to UnParse a.k.a. join
         * @param str  the joined string
         * @param joinChar the char that will be placed between items of the vector
         *                 [default=',']
         * @param joinCharOuter determines if the joinChar is also placed around
         *        the items in the joined vector. The default only places the
         *        joinChars within the joined items [default=false]
         */
        void UnParseCommaSeparatedStrings(const std::vector<std::string>& strs, std::string& str, char joinChar = ',', bool joinCharOuter = false);
        /**
         * @param long the vector of longs to UnParse a.k.a. join
         * @param str  the joined string
         * @param joinChar the char that will be placed between items of the vector
         *                 [default=',']
         * @param joinCharOuter determines if the joinChar is also placed around
         *        the items in the joined vector. The default only places the
         *        joinChars within the joined items [default=false]
         */
        void UnParseCommaSeparatedLongs(const std::vector<long>& longs, std::string& str, char joinChar = ',', bool joinCharOuter = false);
        /**
         * @param doubles the vector of doubles to UnParse a.k.a. join
         * @param str  the joined string
         * @param joinChar the char that will be placed between items of the vector
         *                 [default=',']
         * @param joinCharOuter determines if the joinChar is also placed around
         *        the items in the joined vector. The default only places the
         *        joinChars within the joined items [default=false]
         */
        void UnParseCommaSeparatedDoubles(const std::vector<double>& doubles, std::string& str, char joinChar = ',', bool joinCharOuter = false);
        /**
         * @param ints the vector of ints to UnParse a.k.a. join
         * @param str  the joined string
         * @param joinChar the char that will be placed between items of the vector
         *                 [default=',']
         * @param joinCharOuter determines if the joinChar is also placed around
         *        the items in the joined vector. The default only places the
         *        joinChars within the joined items [default=false]
         */
        void UnParseCommaSeparatedInts(const std::vector<int>& ints, std::string& str, char joinChar = ',', bool joinCharOuter = false);

        /*
        template <>
         void append(std::string & s, const T & t)
        {
            s.append(boost::lexical_cast<std::string>(t);
        }
        */

        void append(std::string& s, const long& l);

        void append(std::string& s, const double& d);

        void append(std::string& s, const int& i);
    } // namespace StringParserUtility
} /* namespace rootmap */

#endif // #ifndef StringParserUtility_H
