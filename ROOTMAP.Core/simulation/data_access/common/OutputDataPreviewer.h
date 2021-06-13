#ifndef OutputDataPreviewer_H
#define OutputDataPreviewer_H
/////////////////////////////////////////////////////////////////////////////
// Name:        OutputDataPreviewer.h
// Purpose:     Declaration of the OutputDataPreviewer class
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/DataPreviewer.h"
#include "simulation/process/interprocess_communication/PostOffice.h"
#include "simulation/data_access/interface/OutputRuleDAI.h"
#include <vector>
#include "wx/string.h"

namespace rootmap
{
    class OutputDataPreviewer : public DataPreviewer
    {
        friend class RmCWOutputRulesPage;

    public:

        OutputDataPreviewer(PostOffice* po);
        virtual ~OutputDataPreviewer();

        void ResetOutputRuleIndex();
        const size_t outputRuleCount() const;
        void ChangesHaveBeenMade();
        const bool DataChanged() const;

        void AddOutputRule(OutputRuleDAI* bc);
        void RemoveLastOutputRule();
        const bool HasMoreOutputRules() const;
        OutputRuleDAI* GetNextOutputRule();

        PostOffice* GetPostOffice();

    protected:

        std::vector<OutputRuleDAI*> m_outputRules;

    private:

        size_t m_nextOutputRuleIndex;
        bool m_dataChanged;
        PostOffice* m_po;
    };

    inline void OutputDataPreviewer::ResetOutputRuleIndex()
    {
        m_nextOutputRuleIndex = 0;
    }

    inline const size_t OutputDataPreviewer::outputRuleCount() const
    {
        return m_outputRules.size();
    }

    inline const bool OutputDataPreviewer::HasMoreOutputRules() const
    {
        return m_nextOutputRuleIndex < m_outputRules.size();
    }

    inline void OutputDataPreviewer::RemoveLastOutputRule()
    {
        std::vector<OutputRuleDAI*>::const_iterator i = m_outputRules.end() - 1;
        delete (*i);
        m_outputRules.erase(i);
    }

    inline void OutputDataPreviewer::ChangesHaveBeenMade()
    {
        m_dataChanged = true;
    }

    inline const bool OutputDataPreviewer::DataChanged() const
    {
        return m_dataChanged;
    }

    inline PostOffice* OutputDataPreviewer::GetPostOffice()
    {
        return m_po;
    }
} /* namespace rootmap */


#endif // #ifndef OutputDataPreviewer_H
