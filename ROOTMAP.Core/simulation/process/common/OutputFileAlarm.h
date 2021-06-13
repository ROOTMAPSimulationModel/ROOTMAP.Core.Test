#ifndef OutputFileAlarm_H
#define OutputFileAlarm_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ClassTemplate.h
// Purpose:     Declaration of the ClassTemplate class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "Process.h"

namespace rootmap
{

    class OutputAssociation;

    class OutputFileAlarm : public Process
    {
    public:
        // Construction function : required for new_by_name()
        OutputFileAlarm();
        ~OutputFileAlarm();

        // Core action overrides
    //    virtual long int Initialise(ProcessActionDescriptor *action);
    //    virtual long int StartUp(ProcessActionDescriptor *action);
    //    virtual long int Ending(ProcessActionDescriptor *action);
    //    virtual long int Terminate(ProcessActionDescriptor *action);

        // Standard action overrides
        virtual long int DoNormalWakeUp(ProcessActionDescriptor *action); // 'Nrml'
        virtual long int DoExternalWakeUp(ProcessActionDescriptor *action); // 'XWak'
    //    virtual long int DoSpecialInput(ProcessActionDescriptor *action); // 'Inpu'

        // Stuff i made up while i was tired
        virtual long int DoWakeOutputProcesses(ProcessActionDescriptor *action);

        virtual bool DoesOverride();

    private:
        OutputAssociation * m_associations;

    };

} // namespace rootmap

#endif // #ifndef OutputFileAlarm_H
