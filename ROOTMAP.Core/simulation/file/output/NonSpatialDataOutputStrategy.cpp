/////////////////////////////////////////////////////////////////////////////
// Name:        NonSpatialDataOutputStrategy.cpp
// Purpose:     Definition of the NonSpatialDataOutputStrategy class
// Created:     15/09/2009
// Author:      MSA
// $Date: 2009-09-15 02:07:36 +0800 (Tue, 15 Sep 2009) $
// $Revision: 1 $
// Copyright:   ©2009 University of Tasmania/University of Western Australia
/////////////////////////////////////////////////////////////////////////////

#include "simulation/file/output/NonSpatialDataOutputStrategy.h"
#include "simulation/process/interprocess_communication/PostOffice.h"
#include "simulation/file/output/DataOutputFile.h"
#include "core/common/RmAssert.h"
#include "simulation/file/output/OutputRule.h"
#include "simulation/process/shared_attributes/ProcessSharedAttribute.h"


namespace rootmap
{
    RootMapLoggerDefinition(NonSpatialDataOutputStrategy);

    NonSpatialDataOutputStrategy::NonSpatialDataOutputStrategy(Process* pptr, std::vector<SharedAttribute *>& sharedAttributeVector, const wxString& delim)
        : m_process(pptr)
        , m_sharedAttributes(sharedAttributeVector)
        , m_delimiter(delim)
        , m_step(0)
    {
        RmAssert(__nullptr != pptr, "Error: Process pointer NULL");
        // MSA 09.09.29 Serious issue here: how do we determine (in a less catastrophic manner) whether or not these SharedAttributes have valid GetValue(void) methods?
        const size_t count = m_sharedAttributes.size();
        for (size_t i = 0; i < count; ++i)
        {
            if (__nullptr == m_sharedAttributes[i])
            {
                throw new RmException("Error: SharedAttribute pointer NULL");
            }
            m_sharedAttributes[i]->GetValue(); // "Error: The SharedAttribute supplied is not valid for 0-dimensional values; i.e. it does not provide non-spatial data"
        }
    }

    NonSpatialDataOutputStrategy::NonSpatialDataOutputStrategy(Process* pptr, SharedAttribute* sharedAttribute, const wxString& delim)
        : m_process(pptr)
        , m_delimiter(delim)
        , m_step(0)
    {
        RmAssert(__nullptr != pptr && __nullptr != sharedAttribute, "Error: Process and/or SharedAttribute pointer(s) NULL");
        // MSA 09.09.29 Serious issue here: how do we determine (in a less catastrophic manner) whether or not this SharedAttribute has a valid GetValue(void) method?
        sharedAttribute->GetValue();// "Error: The SharedAttribute supplied is not valid for 0-dimensional values; i.e. it does not provide non-spatial data"
        m_sharedAttributes.push_back(sharedAttribute);
    }

    NonSpatialDataOutputStrategy::~NonSpatialDataOutputStrategy()
    {
        // This class does not own anything on the heap
    }

    void NonSpatialDataOutputStrategy::ArrangeOutput(ProcessActionDescriptor* action, OutputRule* output_rule)
    {
        Use_PostOffice;
        postoffice->sendMessage(m_process, m_process, kSpecialOutputDataMessage, output_rule);
        m_now = postoffice->GetNow();
    }

    void NonSpatialDataOutputStrategy::OutputToFile(DataOutputFile& file)
    {
        if (0 == m_step) // Write column headers
        {
            file << "Actual Date/time,Simulation Date/time";
            for (size_t i = 0; i < m_sharedAttributes.size(); ++i)
            {
                file << m_delimiter << m_sharedAttributes[i]->GetCharacteristicDescriptor()->Name;
            }
            file << "\n";
        }

        ++m_step;

        wxDateTime realNow(time(__nullptr));
        wxDateTime simNow(m_now);

        file << realNow.Format("%Y/%m/%d %H:%M:%S");
        file << simNow.Format(",%Y/%m/%d %H:%M:%S");
        for (size_t i = 0; i < m_sharedAttributes.size(); ++i)
        {
            file << m_delimiter << m_sharedAttributes[i]->GetValue();
        }
        file << "\n";
    }
} /* namespace rootmap */

