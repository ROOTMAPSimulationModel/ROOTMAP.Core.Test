/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessExceptions.cpp
// Purpose:     Implementation of the various ProcessExceptions class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/common/ProcessExceptions.h"


namespace rootmap
{
    ProcessException::ProcessException(const char* reason, const char* processName)
        : RmException(reason)
        , m_processName(processName)
    {
    }

    ProcessException::ProcessException(const ProcessException& original)
        : RmException(original)
        , m_processName(original.m_processName)
    {
    }
} /* namespace rootmap */



