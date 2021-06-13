#include "BoostPrecomp.h"

#include "Process.h"
#include "OutputFileAlarm.h"
#include "Utilities.h"
#include "JOutputAssociation.h"
#include "JDataOutputFile.h"
#include <limits>
#include "simulation/common/BoostPrecomp.h"

namespace rootmap
{

OutputFileAlarm::OutputFileAlarm()
: Process()
, pofa_associations(0)
, m_associations(NULL)
{
    TCL_END_CONSTRUCTOR
}

OutputFileAlarm::~OutputFileAlarm()
{
    TCL_START_DESTRUCTOR
}

#if 0
/* Initialise
Called before the user sees or does anything. Called, in fact, before ANY messages
are sent, even those with a timestamp <0. We will use this moment to <insert stuff here>.

This process currently doesn't use SetPeriodicWaking, because although it would
take away some of the tedium (what else are computers for, anyway?!) we would
need to override GetPeriodicDelay to return the time between now and the next
output requirement. Which is getting more complicated than it needs to be.*/
long int OutputFileAlarm::Initialise(ProcessActionDescriptor *action)
{

    // Do the other, default stuff
    return (Process::Initialise(action));
}

long int OutputFileAlarm::StartUp(ProcessActionDescriptor *action)
{
    return (Process::StartUp(action));
}

long int OutputFileAlarm::Ending(ProcessActionDescriptor *action)
{
    return (Process::Ending(action));
}
#endif

long int OutputFileAlarm::DoNormalWakeUp(ProcessActionDescriptor *action)
{
    Use_ReturnValue;

    return_value = DoWakeOutputProcesses(action);

    return_value |= Process::DoNormalWakeUp(action);

    return (return_value);
}


long int OutputFileAlarm::DoExternalWakeUp(ProcessActionDescriptor *action)
{
    Use_Source;
    Use_ReturnValue;
    Use_ProcessCoordinator;

    /* we have a special agreement with JIOManager::MakeOutputAssociations
    such that it sends "this" process as the source of a kExternalWakeUpMessage,
    to let us know it is sending us output associations. */
    if (source == this)
    {
        pofa_associations = (JOutputAssociation*)(action->padData);

        /* and now, just for the fun of it, we could do some stuff to each association. */
/*        JOutputAssociation *next_assoc = pofa_associations;
        while (next_assoc != 0)
        {
            for instance : 
            next_assoc->output_assoc_origin_process = processcoordinator->FindProcessByProcessName(next_assoc->output_assoc_origin_name);
            (except, of course, that not every association involves a process)
        }
*/        
    }

    // here, we establish our first waking time. We could just call DoWakeOutputProcesses,
    // but that seems a bit dodgy
    JOutputAssociation *r = pofa_associations;
    Use_Time;
    Use_PostOffice;
    long next_closest_time = LONG_MAX;
    long next_time;

    while (r != 0)
    {
        next_time = r->GetNextTime(time);
        if (next_time > time) next_closest_time = TCLMin(next_closest_time, next_time);
        r = (JOutputAssociation *)(r->GetNext());
    }

    if ((next_closest_time < LONG_MAX) && (next_closest_time > time))
        Send_WakeUp_Message(next_closest_time);

    return (TCLMax(return_value, Process::DoExternalWakeUp(action)));
}

/*
long int OutputFileAlarm::DoSpecialInput(ProcessActionDescriptor *action)
{
    return (Process::DoSpecialInput(action));
}
*/

long int OutputFileAlarm::DoWakeOutputProcesses(ProcessActionDescriptor * action)
{
    JOutputAssociation * r = pofa_associations;
    Use_Time;
    Use_PostOffice;
    unsigned char usc = 0, *fake_name = &usc;
    long next_closest_time = LONG_MAX;
    long next_time;

    while (r != 0)
    {
        // if (we export once, and this is that once) or (this is one of the many times we export)
//        if ((r->DoThisTime(time)) && (r->ExportFile() != 0))
        if (r->DoThisTime(time))
        {
//            r->ExportFile()->SetCreatorAndType();
            if (r->SpecifyFile(fake_name) != 0)
            {
                action->padData = (Handle)r;
                r->DoProvokeOutput(action);
            }
        }

        next_time = r->GetNextTime(time);
        if (next_time > time) next_closest_time = TCLMin(next_closest_time, next_time);
        r = (JOutputAssociation *)(r->GetNext());
    }

    if ((next_closest_time < LONG_MAX) && (next_closest_time > time))
        Send_WakeUp_Message(next_closest_time);

    return (kNoError);
}

bool OutputFileAlarm::DoesOverride()
{
    return (true);
}

} // namespace rootmap

