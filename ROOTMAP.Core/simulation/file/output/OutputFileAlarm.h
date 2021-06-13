#ifndef OutputFileAlarm_H
#define OutputFileAlarm_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ClassTemplate.h
// Purpose:     Declaration of the ClassTemplate class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/common/Process.h"

namespace rootmap
{
    class OutputRule;

    class OutputFileAlarm : public Process
    {
    public:

        DECLARE_DYNAMIC_CLASS(OutputFileAlarm)

        /** Default constructor : required for new_by_name() */
        OutputFileAlarm();

        /** Delete's all OutputRules */
        ~OutputFileAlarm();

        /** Initialises the OutputRules */
        virtual long int Initialise(ProcessActionDescriptor* action);

        /** Calls InvokeOutputRules */
        virtual long int DoNormalWakeUp(ProcessActionDescriptor* action); // 'Nrml'

        /** Receives an OutputRule from the DataOutputCoordinator, if the source
         * process was "this".
         *
         * Also could have used "DoSpecialInput".
         */
        virtual long int DoExternalWakeUp(ProcessActionDescriptor* action); // 'XWak'

        /** called from DoNormalWakeUp
         * Iterates over the OutputRule collection and if an O.R. DoesThisTime,
         * calls its ArrangeOutput.
         * Also implicitly performs the SendNextOutputTime while iterating.
         */
        virtual long int InvokeOutputRules(ProcessActionDescriptor* action);

        /**
         * Called from Initialise, this initiates the output process by iterating
         * over the OutputRule collection and determining the next closest time that
         * an OutputRule's TimingStrategy requires output, and sends a WakeUp to self
         * for that time.
         *
         * DoNormalWakeUp will activate all OutputRules at that time.
         */
        virtual ProcessTime_t SendNextOutputTime(ProcessActionDescriptor* action);

        /**
         * Yes, yes, this process does override.
         */
        virtual bool DoesOverride() const;

    private:

        ///
        ///
        typedef std::list<OutputRule *> OutputRuleCollection;

        /**
         * The collection of output rules, which are owned
         * by this class (and hence delete'd)
         */
        OutputRuleCollection m_outputRules;
    };
} /* namespace rootmap */

#endif // #ifndef OutputFileAlarm_H
