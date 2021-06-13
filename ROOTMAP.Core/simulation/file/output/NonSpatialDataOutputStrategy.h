#ifndef NonSpatialDataOutputStrategy_H
#define NonSpatialDataOutputStrategy_H
/////////////////////////////////////////////////////////////////////////////
// Name:        NonSpatialDataOutputStrategy.h
// Purpose:     Declaration of the NonSpatialDataOutputStrategy class
// Created:     15/09/2009
// Author:      MSA
// $Date: 2009-09-15 02:07:36 +0800 (Tue, 15 Sep 2009) $
// $Revision: 1 $
// Copyright:   ©2009 University of Tasmania/University of Western Australia
/////////////////////////////////////////////////////////////////////////////

#include "simulation/file/output/OutputStrategy.h"
#include "simulation/process/common/Process.h"
#include "simulation/process/shared_attributes/SharedAttribute.h"
#include "core/log/Logger.h"

namespace rootmap
{
    class NonSpatialDataOutputStrategy : public OutputStrategy
    {
    public:
        RootMapLoggerDeclaration();

        /**
         *    Main constructor for NonSpatialDataOutputStrategy.
         *    Takes vector of SharedAttributes, rather than ProcessSharedAttributes, as parameter
         *    due to the impossibility of dynamic_casting (SharedAttribute is not polymorphic, by design).
         *    Tests the 0-dimensional GetValue function in the constructor; will fail with a null pointer dereference
         *    if the SharedAttributes cannot handle 0-dimensional data
         *    Very far from ideal, but the only way I can see to do it without messing with the code of SharedAttribute.
         */
        NonSpatialDataOutputStrategy(Process* pptr, std::vector<SharedAttribute*>& sharedAttributeVector, const wxString& delim);

        /**
         *    Convenience constructor for single-SA output.
         */
        NonSpatialDataOutputStrategy(Process* pptr, SharedAttribute* sharedAttribute, const wxString& delim);
        // MSA NonSpatialDataOutputStrategy may in future be derived from, so destructor is virtual
        virtual ~NonSpatialDataOutputStrategy();

        virtual void ArrangeOutput(ProcessActionDescriptor* action, OutputRule* output_rule);

        virtual void OutputToFile(DataOutputFile& file);
    private:

        Process* m_process;
        std::vector<SharedAttribute *> m_sharedAttributes;
        const wxString m_delimiter;
        size_t m_step;
        ProcessTime_t m_now;
    }; // class NonSpatialDataOutputStrategy
} /* namespace rootmap */

#endif // #ifndef NonSpatialDataOutputStrategy_H
