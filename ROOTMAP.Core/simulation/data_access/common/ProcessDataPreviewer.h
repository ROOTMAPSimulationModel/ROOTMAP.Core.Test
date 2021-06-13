#ifndef ProcessDataPreviewer_H
#define ProcessDataPreviewer_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessDataPreviewer.h
// Purpose:     Declaration of the ProcessDataPreviewer class
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/DataPreviewer.h"

#include <algorithm>
#include <cctype>
#include <vector>

namespace rootmap
{
    class ProcessDataPreviewer : public DataPreviewer
    {
    public:

        ProcessDataPreviewer(const std::string& processName);

        virtual ~ProcessDataPreviewer();

        std::string GetProcessName();

        void AddCharacteristicSummary(const std::string& name, const double& min, const double& value, const double& max);

        struct CharacteristicSummary
        {
            std::string m_name;
            double m_value;
            double m_min;
            double m_max;

            CharacteristicSummary(const std::string& n, const double& mi, const double& v, const double& ma)
                : m_name(n)
                , m_min(mi)
                , m_value(v)
                , m_max(ma)
            {
            }

            // From http://www.ozzu.com/programming-forum/quicksort-strings-without-strcmp-alphabetical-order-t35695.html
            static const bool lessthanAlphabetic(std::string const& a, std::string const& b)
            {
                size_t offset = 0;
                while (true)
                {
                    if (offset == b.size()) // then b < a or b == a.
                        return false;

                    if (offset == a.size()) // then a < b.
                        return true;

                    if (tolower(a[offset]) < tolower(b[offset]))
                        return true; // true that a < b.

                    if (tolower(b[offset]) < tolower(a[offset]))
                        return false; // because b < a.

                    ++offset; // the current characters are the same, so check the next.
                }
            }

            // These comparison operators just work on the name string.
            const bool operator==(const CharacteristicSummary& rhs) const
            {
                return m_name == rhs.m_name;
            }

            const bool operator<(const CharacteristicSummary& rhs) const
            {
                return lessthanAlphabetic(m_name, rhs.m_name);
            }

            const bool operator>(const CharacteristicSummary& rhs) const
            {
                return lessthanAlphabetic(rhs.m_name, m_name);
            }
        };

    protected:

        std::string m_processName;

        std::vector<CharacteristicSummary*> m_characteristicSummaries;
    };


    inline std::string ProcessDataPreviewer::GetProcessName()
    {
        return m_processName;
    }
} /* namespace rootmap */

#endif // #ifndef ProcessDataPreviewer_H
