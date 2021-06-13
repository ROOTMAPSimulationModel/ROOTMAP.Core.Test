#include "core/utility/StringParserUtility.h"
#include "core/utility/Utility.h"
#include "core/common/Exceptions.h"
#include "boost/lexical_cast.hpp"

#undef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
// less harsh :
//#undef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
//#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include <stdlib.h>

namespace rootmap
{
    namespace StringParserUtility
    {
        bool ParseCommaSeparatedDoubles
        (const char* buffer,
            std::vector<double>& double_stuff
        )
        {
            int index = 0;
            char c = buffer[index];
            std::string s = "";

            while (true)
            {
                /* from MSDN online:
                 * The str argument to atof and _wtof has the following form:
                 * [whitespace] [sign] [digits] [.digits] [ {d | D | e | E }[sign]digits]
                 */


                if (((c >= '0') && (c <= '9'))
                    || (c == '+') || (c == '-')
                    || (c == 'e') || (c == 'E')
                    || (c == 'd') || (c == 'D')
                    || (c == '.')
                    )
                {
                    s.append(1, c);
                }
                else if (c == 0) // end of string
                {
                    if (!s.empty())
                    {
                        double_stuff.push_back(atof(s.c_str()));
                    }
                    return (!double_stuff.empty());
                }
                else // assumed a delimiter - start with a new number string
                {
                    if (!s.empty())
                    {
                        double_stuff.push_back(atof(s.c_str()));
                    }
                    s = "";
                }

                c = buffer[++index];
            }

            return false;
        }


        bool ParseCommaSeparatedStrings
        (const char* buffer,
            std::vector<std::string>& string_stuff
        )
        {
            // check for null buffer
            if ((0 == buffer) || (0 == buffer[0])) return false;

            int index = 0;
            char c = buffer[index];
            std::string s = "";

            while (true)
            {
                if ((c >= ' ') && (c != ',') && (c != '\r') && (c != '\n'))
                {
                    s.append(1, c);
                }
                else if (c == 0) // end of string
                {
                    if (!s.empty())
                    {
                        string_stuff.push_back(s);
                    }
                    return (!string_stuff.empty());
                }
                else // assumed a delimiter - start with a new number string
                {
                    if (!s.empty())
                    {
                        string_stuff.push_back(s);
                    }
                    s = "";
                }

                c = buffer[++index];
            }

            return false;
        }


        bool ParseCommaSeparatedLongs
        (const char* buffer,
            std::vector<long>& long_stuff
        )
        {
            int index = 0;
            char c = buffer[index];
            std::string s = "";

            while (true)
            {
                if (((c >= '0') && (c <= '9')) || (c == '+') || (c == '-'))
                {
                    s.append(1, c);
                }
                else if (c == 0) // end of string
                {
                    if (!s.empty())
                    {
                        long_stuff.push_back(atol(s.c_str()));
                    }
                    return (!long_stuff.empty());
                }
                else // assumed a delimiter - start with a new number string
                {
                    if (!s.empty())
                    {
                        long_stuff.push_back(atol(s.c_str()));
                    }
                    s = "";
                }

                c = buffer[++index];
            }

            return false;
        }


        bool ParseCommaSeparatedInts
        (const char* buffer,
            std::vector<int>& int_stuff
        )
        {
            int index = 0;
            char c = buffer[index];
            std::string s = "";

            while (true)
            {
                if (((c >= '0') && (c <= '9')) || (c == '+') || (c == '-'))
                {
                    s.append(1, c);
                }
                else if (c == 0) // end of string
                {
                    if (!s.empty())
                    {
                        int_stuff.push_back(atoi(s.c_str()));
                    }
                    return (!int_stuff.empty());
                }
                else // assumed a delimiter - start with a new number string
                {
                    if (!s.empty())
                    {
                        int_stuff.push_back(atoi(s.c_str()));
                    }
                    s = "";
                }

                c = buffer[++index];
            }

            return false;
        } // bool ParseCommaSeparatedInts

        void UnParseCommaSeparatedStrings
        (const std::vector<std::string>& str_vec,
            std::string& str,
            char joinChar,
            bool joinCharOuter)
        {
            bool first_item = true;
            for (std::vector<std::string>::const_iterator iter(str_vec.begin());
                iter != str_vec.end(); ++iter)
            {
                if ((!first_item) || (joinCharOuter))
                {
                    str.append(1, joinChar);
                }

                str.append(*iter);
                first_item = false;
            }

            if (joinCharOuter)
            {
                str.append(1, joinChar);
            }
        }

        void UnParseCommaSeparatedLongs
        (const std::vector<long>& long_vec,
            std::string& str,
            char joinChar,
            bool joinCharOuter)
        {
            bool first_item = true;
            for (std::vector<long>::const_iterator iter(long_vec.begin());
                iter != long_vec.end(); ++iter)
            {
                if ((!first_item) || (joinCharOuter))
                {
                    str.append(1, joinChar);
                }

                append(str, *iter);
                first_item = false;
            }

            if (joinCharOuter)
            {
                str.append(1, joinChar);
            }
        }

        void UnParseCommaSeparatedDoubles
        (const std::vector<double>& double_vec,
            std::string& str,
            char joinChar,
            bool joinCharOuter)
        {
            bool first_item = true;
            for (std::vector<double>::const_iterator iter(double_vec.begin());
                iter != double_vec.end(); ++iter)
            {
                if ((!first_item) || (joinCharOuter))
                {
                    str.append(1, joinChar);
                }

                append(str, *iter);
                first_item = false;
            }

            if (joinCharOuter)
            {
                str.append(1, joinChar);
            }
        }

        void UnParseCommaSeparatedInts
        (const std::vector<int>& int_vec,
            std::string& str,
            char joinChar,
            bool joinCharOuter)
        {
            bool first_item = true;
            for (std::vector<int>::const_iterator iter(int_vec.begin());
                iter != int_vec.end(); ++iter)
            {
                if ((!first_item) || (joinCharOuter))
                {
                    str.append(1, joinChar);
                }

                append(str, *iter);
                first_item = false;
            }

            if (joinCharOuter)
            {
                str.append(1, joinChar);
            }
        }

        void append(std::string& s, const long& l)
        {
            s.append(boost::lexical_cast<std::string>(l));
        }

        void append(std::string& s, const double& d)
        {
            s.append(boost::lexical_cast<std::string>(d));
        }

        void append(std::string& s, const int& i)
        {
            s.append(boost::lexical_cast<std::string>(i));
        }
    } // namespace StringParserUtility
} /* namespace rootmap */
