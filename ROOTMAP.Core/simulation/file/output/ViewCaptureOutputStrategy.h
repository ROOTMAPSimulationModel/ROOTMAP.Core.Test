#ifndef ViewCaptureOutputStrategy_H
#define ViewCaptureOutputStrategy_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ViewCaptureOutputStrategy.h
// Purpose:     Declaration of the ViewCaptureOutputStrategy class
// Created:     21/09/2009
// Author:      MSA
// $Date: 2010-01-19 02:07:36 +0800 (Mon, 21 Sep 2009) $
// $Revision: 2 $
// Copyright:   ©2009 University of Tasmania/University of Western Australia
/////////////////////////////////////////////////////////////////////////////

#include "simulation/file/output/OutputStrategy.h"
#include "simulation/common/SimulationEngine.h"
#include "core/log/Logger.h"

namespace rootmap
{
    class ViewCaptor;

    class ViewCaptureOutputStrategy : public OutputStrategy
    {
    public:
        RootMapLoggerDeclaration();

        // Enumeration of View types.
        // VIEW_2D corresponds to ViewCoordinator, VIEW_3D to View3DCoordinator
        enum ViewType
        {
            VIEW_2D,
            VIEW_3D
        };

        ViewCaptureOutputStrategy(SimulationEngine* gseptr, const ViewType vt, const size_t& viewWindowIndex, const wxString& fileExt = "bmp");
        // MSA ViewCaptureOutputStrategy may in future be derived from, so destructor is virtual
        virtual ~ViewCaptureOutputStrategy();

        virtual void ArrangeOutput(ProcessActionDescriptor* action, OutputRule* output_rule);

        virtual void OutputToFile(DataOutputFile& file);
    private:
        ViewCaptor* m_viewCaptor;
        SimulationEngine* m_simulationEngine;
        const ViewType m_viewType;
        const size_t m_viewWindowIndex;
        const wxString m_fileExt;
    }; // class ViewCaptureOutputStrategy
} /* namespace rootmap */

#endif // #ifndef ViewCaptureOutputStrategy_H
