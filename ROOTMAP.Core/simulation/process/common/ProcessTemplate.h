#ifndef ProcessTemplate_H
#define ProcessTemplate_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessTemplate.h
// Purpose:     Declaration of the ProcessTemplate class
// Created:     22-03-2009
// Author:      Your Name
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
// File-wide comments can go here, but class comments should go just above the
// start of the class itself (for javadoc/doxygen effectiveness)
/////////////////////////////////////////////////////////////////////////////
//other files whose definitions we use in this file
#include "simulation/process/common/Process.h"
#include "core/common/DoubleCoordinates.h"
// Rootmap log4cpp macros
#include "core/debug/log/Logger.h"

namespace rootmap
{

// Forward declarations.
class ProcessSharedAttributeOwner;
class SharedAttribute;


/**
 * Class description goes here
 */
class ProcessTemplate : public Process
{
public:
    // wxWidgets macro for dynamic by-name instantiation
    DECLARE_DYNAMIC_CLASS(ProcessTemplate)

    // this is rootmap's macro to force this class to be linked into the executable
    // (otherwise the linker strips it out because there is no explicit use of the class)
    DECLARE_DYNAMIC_CLASS_FORCE_USE(ProcessTemplate)

public:
    ProcessTemplate();
    ~ProcessTemplate();

    virtual long int StartUp(ProcessActionDescriptor *action);
    virtual long int Initialise(ProcessActionDescriptor *action);

    virtual long int DoNormalWakeUp(ProcessActionDescriptor *action);

    virtual bool DoesOverride() const;

private:
    RootMapLoggerDeclaration();
};

} /* namespace rootmap */

#endif // #ifndef ProcessTemplate_H
