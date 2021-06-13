/////////////////////////////////////////////////////////////////////////////
// Name:        PostOffice.cpp
// Purpose:     Implementation of the PostOffice class
// Created:     1995
// Author:      RvH
// $Date: 2009-07-19 19:34:59 +0800 (Sun, 19 Jul 2009) $
// $Revision: 72 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/interprocess_communication/PostOffice.h"
#include "simulation/process/interprocess_communication/Message.h"
#include "simulation/process/interprocess_communication/MessageList.h"
#include "simulation/process/common/Process.h"
#include "simulation/process/common/ProcessList.h"
#include "simulation/process/common/ProcessCoordinator.h"
#include "simulation/data_access/interface/PostOfficeDAI.h"
#include "simulation/data_access/common/DataAccessManager.h"
#include "simulation/scoreboard/ScoreboardCoordinator.h"

#include "core/common/Structures.h"
#include "core/utility/Utility.h"
#include "core/common/ExtraStuff.h"
#include "core/common/RmAssert.h"

#include "wx/datetime.h"

#define DEBUG_POSTOFFICE_DOIMMEDIATEMESSAGES 1
#define DEBUG_POSTOFFICE_DONEXTMESSAGE 1

//
// Comment out the following line to log timestamps as the raw seconds.
#define LOGTIME_STRING
#if defined LOGTIME_STRING
#define LOGTIME(t) LogTime(t)
#endif // #if defined LOGTIME_STRING
#ifndef LOGTIME
#define LOGTIME(t) t
#endif // #ifndef LOGTIME

namespace rootmap
{
    RootMapLoggerDefinition(PostOffice);

    /* PostOffice
    The one and only constructor. Just sets the time variables to zero, and the list object(s) to NULL */
    PostOffice::PostOffice(SimulationEngine& /* engine */, DataAccessManager& dam)
        : m_seed(0)
        , m_previous(0)
        , m_now(0)
        , m_start(0)
        , m_nearestEnd(0)
        , m_defaultRunTime(0)
        , m_messages(0)
        , m_sendingImmediate(false)
        , m_simulationPaused(false)
    {
        RootMapLoggerInitialisation("rootmap.PostOffice");
        RootMapLogTrace("Constructor");

        m_messages = new MessageList;
        dam.constructPostOffice(this);
    }

    void PostOffice::constructPostOffice(PostOfficeDAI& postoffice_data)
    {
        RootMapLogTrace("constructPostOffice");
        // Seed the PRNG with the given value.
        // If a seed value has not been specified in the configuration,
        // a timestamp will be used.
        // By requiring an external seed for the PRNG,
        // we can ensure that identical simulations can be reproduced if desired.
        m_seed = postoffice_data.getSeed();
        srand(m_seed);

        m_previous = postoffice_data.getPreviousTime();
        m_now = postoffice_data.getNow();
        m_start = postoffice_data.getStartTime();
        m_nearestEnd = postoffice_data.getNearestEnd();
        m_defaultRunTime = postoffice_data.getDefaultRunTime();
        RootMapLogDebug("Constructing {Previous:" << LOGTIME(m_previous) <<
            ", Now:" << LOGTIME(m_now) <<
            ", Start:" << LOGTIME(m_start) <<
            ", End:" << LOGTIME(m_nearestEnd) <<
            ", Runtime:" << LOGTIME(m_defaultRunTime) <<
            ", #messages:" << m_messages->GetNumMessages()) <<
            ", PRNG Seed:" << m_seed << "}";
    }


    /* ~PostOffice
    The destructor. Dispose of the "cBreaks" list */
    PostOffice::~PostOffice()
    {
        RootMapLogTrace("~PostOffice");
        LOG_DEBUG << "Destructing {Previous:" << LOGTIME(m_previous) <<
            ", Now:" << LOGTIME(m_now) <<
            ", Start:" << LOGTIME(m_start) <<
            ", End:" << LOGTIME(m_nearestEnd) <<
            ", Runtime:" << LOGTIME(m_defaultRunTime) <<
            ", #messages:" << m_messages->GetNumMessages() << "}";
        delete m_messages;
    }


    //
    // Function:
    //  sendMessage
    //
    //
    // Description:
    //  Short format, for sending a kNormalMessage (without data) to oneself at
    //  a specifiable time.
    //
    //
    // Modifications:
    //  YYYYMMDD RvH - 
    //
    void PostOffice::sendMessage
    (ProcessTime_t time,
        Process* sender
    )
    {
        RootMapLogTrace("sendMessage(ProcessTime_t, Process *)");
        if ((m_sendingImmediate) && (time <= m_now))
        {
            // can't send messages in this timestamp anymore if immediate messages
            // are being dealt with
            RootMapLogInfo("Not sending any more messages this timestamp - dealing with immediate messages first");
        }
        else
        {
            Message* m = m_messages->Add(time, sender, sender, 0, kNormalMessage);
            if (0 == m)
            {
                // The message was not constructed for one of many reasons.  If this
                // is going to be reported as an error that should be done in
                // MessageList::Add(), so the return value can be ignored.
                RootMapLogWarn(LOG_LINE << "Self-message for Process '" << sender->GetProcessName() << " was not created {Time:" << LOGTIME(time) << "}");
            }
            else
            {
                RootMapLogDebug(LOG_LINE << "Self-message for Process '" << sender->GetProcessName() << " was added to message list {Time:" << LOGTIME(time) << "}");
            }
        }
    }

    //
    // Function:
    //  sendMessage
    //
    //
    // Description:
    //  Sends a timed data-enhanced message to the process as specified.
    //
    //
    // Modifications:
    //  YYYYMMDD RvH - 
    //
    void PostOffice::sendMessage
    (ProcessTime_t time,
        Process* src,
        Process* dest,
        SpecialProcessData* data,
        MessageType_t type
    )
    {
        RootMapLogTrace("sendMessage(ProcessTime_t, Process*, Process*, SpecialProcessData*, MessageType_t)");

        if ((m_sendingImmediate) && (time <= m_now))
        {
            // can't send messages in this timestamp anymore if immediate messages
            // are being dealt with
            RootMapLogInfo("Not sending anymore messages this timestamp - dealing with immediate messages first");
        }
        else
        {
            Message* m = m_messages->Add(time, src, dest, data, type);
            if (0 == m)
            {
                // The message was not constructed for one of many reasons.  If this
                // is going to be reported as an error that should be done in
                // MessageList::Add(), so the return value can be ignored.
                RootMapLogWarn("Message was not created {Type:'" << Utility::OSTypeToString(type)
                    << "', from Process:'" << src->GetProcessName()
                    << "' to Process:'" << dest->GetProcessName()
                    << "' Time:" << LOGTIME(time) << "}");
            }
        }
    }


    //
    // Function:
    //  sendMessage
    //
    //
    // Description:
    //  Sends an UNtimed, possibly data-enhanced (default=0) message to the process
    //  as specified.
    //
    void PostOffice::sendMessage(Process* src, Process* dst, MessageType_t type, SpecialProcessData* data)
    {
        RootMapLogTrace("sendMessage(Process*, Process*, MessageType_t, SpecialProcessData*)");
        Message* m = new Message(m_now, src, dst, data, type);
        m_immediateMessages.push(m);
        RootMapLogDebug("Message was added to the immediate message queue {Type '" << m->StringOfType(true) // Utility::OSTypeToString(type)
            << "', from Process:'" << src->GetProcessName()
            << "', to Process:'" << dst->GetProcessName()
            << "'}");
    }


    //
    // Function:
    //  sendMessage
    //
    //
    // Description:
    //  Sends an UNtimed, data-rich message to the process as specified.
    //
    // Working Notes:
    //  NOT IMPLEMENTED
    //
    void PostOffice::sendMessage(Process* /*src*/, Process* /*dst*/, InterprocessDescriptor* /*descriptor*/)
    {
        RootMapLogTrace("sendMessage(Process*, Process*, InterprocessDescriptor*)");
        RootMapLogWarn("Function not implemented: sendMessage(Process*, Process*, InterprocessDescriptor*)");
    }


    //
    // Function:
    //  UnsendMessage
    //
    //
    // Description:
    //  Removes from the timestamped list the message that matches the given
    //  parameters.
    //  
    void PostOffice::UnsendMessage(ProcessTime_t time, Process* sender)
    {
        RootMapLogTrace("UnsendMessage(ProcessTime_t, Process*)");
        m_messages->Remove(time, sender, sender, 0, kNormalMessage);
        LOG_DEBUG << "Removed message {Process:'" << sender->GetProcessName()
            << "', Time:" << LOGTIME(time) << "}";
    }


    //
    // Function:
    //  UnsendMessage
    //
    //
    // Description:
    //  Removes from the timestamped list the message that matches all of the given
    //  parameters.
    //
    void PostOffice::UnsendMessage(ProcessTime_t time, Process* src, Process* dest, SpecialProcessData* data, MessageType_t type)
    {
        RootMapLogTrace("UnsendMessage(ProcessTime_t, Process*, Process*, SpecialProcessData*, MessageType_t)");
        m_messages->Remove(time, src, dest, data, type);
        LOG_DEBUG << "Removed message {From Process:'" << src->GetProcessName()
            << "', To Process:'" << dest->GetProcessName()
            << "', Time:" << LOGTIME(time) << "}";
    }


    //
    // Function:
    //  SendPauseToggle
    //
    //
    // Description:
    //  Adds the special "Pause" message to the beginning of the list
    //
    void PostOffice::SendPause()
    {
        RootMapLogTrace("SendPause");
        m_messages->AddPause();

        LOG_DEBUG << "Added pause message to list";
    }

    //
    // Function:
    //  SendEnd
    //
    //
    // Description:
    //  Adds the special "End" message to the beginning of the list
    //
    void PostOffice::SendEnd(ProcessTime_t endTime)
    {
        RootMapLogTrace("SendEnd(ProcessTime_t)");
        // First make corrections for certain exceptions
        if (endTime == kNow) endTime = m_now;

        // used to do "endTime = m_now" but have decided to ignore endTime's that are before now
        if (endTime < m_now) return;

        m_messages->AddEnd(endTime);

        // because the message list will remove all other "end" messages
        m_nearestEnd = endTime;

        LOG_DEBUG << "Added END message {Time " << LOGTIME(endTime) << "}";
    }


    //
    // Function:
    //  SendStart
    //
    //
    // Description:
    //  Adds the special "Start" message to the beginning of the list
    //
    void PostOffice::SendStart()
    {
        RootMapLogTrace("SendStart()");
        m_messages->AddStart();
    }


    //
    // Function:
    //  DoNextTimeStamp
    //
    //
    // Description:
    //  Takes action for all the messages waiting with the same timestamp as the
    //  next one
    //
    //
    // Working Notes:
    //  Technical notes about algorithms and other implementation-level details.
    //  
    //  
    //
    // Modifications:
    //  YYYYMMDD RvH - 
    //
    bool PostOffice::DoNextTimeStamp
    (ScoreboardCoordinator* scoreboardcoordinator,
        ProcessCoordinator* processcoordinator,
        SharedAttributeManager* sam,
        VolumeObjectCoordinator* voc
    )
    {
        RootMapLogTrace("DoNextTimeStamp");
        ProcessTime_t nextTime = 0;
        ProcessTime_t nowTime = m_messages->PeekNextTime();
        bool isEnd = false;

        DoImmediateMessages(scoreboardcoordinator, processcoordinator, sam, voc);

        while ((nextTime = m_messages->PeekNextTime()) == nowTime)
        {
            isEnd = DoNextMessage(scoreboardcoordinator, processcoordinator, sam, voc);

            if (isEnd)
            {
                break;
            }
        }

        //
        // If the last message was a kEndMessage, the immediate messages will have
        // been done.    
        if (!isEnd)
        {
            DoImmediateMessages(scoreboardcoordinator, processcoordinator, sam, voc);
        }
        return (isEnd);
    }


    //
    // Function:
    //  DoNextMessage
    //
    //
    // Description:
    //  Deals with the next message in the list.  Called by DoNextTimeStamp()
    //
    //
    // Returns:
    //  True if the message that is dealt with is a kEndMessage, false otherwise.
    //
    bool PostOffice::DoNextMessage
    (ScoreboardCoordinator* scoreboardcoordinator,
        ProcessCoordinator* processcoordinator,
        SharedAttributeManager* sam,
        VolumeObjectCoordinator* voc
    )
    {
        RootMapLogTrace("DoNextMessage");
        MessageType_t messageType = kNullMessage;
        //long error = 0;
        bool is_end = false;

        Message* message = m_messages->Pop();


#ifdef DEBUG_POSTOFFICE_DONEXTMESSAGE
        const wxDateTime t = Utility::StartTimer();
#endif // #ifdef DEBUG_POSTOFFICE_DONEXTMESSAGE

        //
        // TODO: This should probably be done elsewhere
        m_previous = m_now;

        ProcessActionDescriptor* action = 0;
        Process* dest = 0;

        if (message != 0)
        {
            m_now = message->GetTime();
            messageType = message->GetType();
            dest = message->GetDestination();
            Scoreboard* scoreboard = __nullptr;
            if (0 != dest)
            {
                scoreboard = scoreboardcoordinator->GetScoreboard(dest->GetProcessStratum());
            }

            action = new ProcessActionDescriptor
            (m_now,
                messageType,
                message->GetData(),
                message->GetSource(),
                this,
                scoreboard,
                scoreboardcoordinator,
                processcoordinator,
                sam,
                voc);
        }
        else
        {
            // return "true", because we can't access any other messages if the
            // next one was nil. This could mean one of several very bad things,
            // and if any of them come true, this is a reasonable solution.
            return (true);
        }

        //
        // if this is the End that is about to be dealt with, we need to take care
        // of the immediate messages.
        if (messageType == kEndMessage)
        {
            is_end = true;
            // done in SendToAllProcesses()
            // DoImmediateMessages(scoreboardcoordinator,processcoordinator,sam);
        }


#ifdef DEBUG_POSTOFFICE_DONEXTMESSAGE
        LOG_DEBUG << "Dispatching Next Message {Source:" << message->StringOfSource()
            << ", Destination:" << message->StringOfDestination()
            << ", Time:" << LOGTIME(message->GetTime())
            << ", Type:" << message->StringOfType(true) << "}";
#endif // #ifdef DEBUG_POSTOFFICE_DONEXTMESSAGE

        //
        // The required message contents have been copied into the
        // ProcessActionDescriptor, so we can (and should) delete it.
        delete message;
        message = __nullptr;

        switch (messageType)
        {
        case kInitialiseMessage:
        case kTerminateMessage:
        case kRegisterMessage:
        case kStartMessage:
        case kEndMessage:
            SendToAllProcesses(action);
            break;

        case kPauseMessage:
            // MSA 09.10.15 By the time a pause message is processed, it's no longer needed. Discard.
            break;

        case kNullMessage:
            break;

        default:
            // at this point, we are now sure that it isn't a multi-process
            // message, so the destination should be non-NULL. Discard and warn
            // if it is, though
            if (0 != dest)
            {
                // scoreboard already set, above
                //action.padScoreboard = scoreboardcoordinator->GetScoreboard(dest->GetProcessVolume());
                // MSA 11.05.02 Error code unused.
                /*error = */
                dest->WakeUp(action);
            }
            else
            {
                RootMapLogError("Non Multi-process message target is NULL {Type:" << Utility::OSTypeToString(messageType)
                    << ", Data:" << message->GetData() << "}");
            }
            break;
        }

#ifdef DEBUG_POSTOFFICE_DONEXTMESSAGE
        LOG_DEBUG << "Message processed in " << Utility::StopTimer(t) << " s";
#endif // #ifdef DEBUG_POSTOFFICE_DONEXTMESSAGE

        delete action;

        return (is_end);
    } // DoNextMessage()


    //
    // Function:
    //  SendToAllProcesses
    //
    //
    // Description:
    //  For the Start, End, Initialise and Terminate messages, calls the appropriate
    //  process module action for each and every process
    //
    // Working Notes:
    //
    void PostOffice::SendToAllProcesses(ProcessActionDescriptor* action)
    {
        RootMapLogTrace("SendToAllProcesses");
        ScoreboardCoordinator* sl = action->GetScoreboardCoordinator();
        ProcessCoordinator* pl = action->GetProcessCoordinator();

        if (action->GetCode() != kRegisterMessage)
        {
            DoImmediateMessages(sl, pl, action->GetSharedAttributeManager(), action->GetVolumeObjectCoordinator());
        }

        AllProcessSender* sender = new AllProcessSender(action);
        action->GetProcessCoordinator()->IterateOverProcesses(sender);
        delete sender;
    } // PostOffice::SendToAllProcesses


    ProcessTime_t PostOffice::GetNextTimeStamp() const
    {
        RootMapLogTrace("GetNextTimeStamp");
        if (m_nearestEnd > m_now)
        {
            return (m_messages->PeekNextTime());
        }

        return m_now;
    }

    Process* PostOffice::GetNextProcess() const
    {
        RootMapLogTrace("GetNextProcess");
        return (m_messages->PeekNextProcess());
    }

    void PostOffice::GetNextDestinationName(std::string& aname) const
    {
        RootMapLogTrace("GetNextDestinationName");
        m_messages->PeekNextDestinationName(aname, false);
        aname.append(" (");
        m_messages->PeekNextTypeName(aname, true);
        aname.append(1, ')');
    }


    void PostOffice::DoImmediateMessages
    (ScoreboardCoordinator* scoreboardcoordinator,
        ProcessCoordinator* processcoordinator,
        SharedAttributeManager* sam,
        VolumeObjectCoordinator* voc
    )
    {
        RootMapLogTrace("DoImmediateMessages");
        // 
        // check if we're already doing this - as this gets called from inside
        // SendToAllProcesses(), which in turn can be called from here
        while ((!m_immediateMessages.empty()) && (!m_sendingImmediate))
        {
#ifdef DEBUG_POSTOFFICE_DONEXTMESSAGE
            const wxDateTime t = Utility::StartTimer();
#endif // #ifdef DEBUG_POSTOFFICE_DONEXTMESSAGE
            m_sendingImmediate = true;
            //
            // See the SGI STL documentation for why a combination of front() and
            // m_p() are required to get the head of the queue, and how queue::empty()
            // being false is a precondition to using front()
            Message* m = m_immediateMessages.front();
            m_immediateMessages.pop(); //

            ProcessActionDescriptor action(m, this, scoreboardcoordinator, processcoordinator, sam, voc);

            action.ResetScoreboard(m->GetDestination()->GetProcessStratum());

            long error = noErr;


#ifdef DEBUG_POSTOFFICE_DOIMMEDIATEMESSAGES
            LOG_DEBUG << "Dispatching Immediate Message {Source:" << m->StringOfSource()
                << ", Destination:" << m->StringOfDestination()
                << ", Time:" << LOGTIME(m->GetTime())
                << ", Type:" << m->StringOfType(true) << "}";
#endif // #ifdef DEBUG_POSTOFFICE_DOIMMEDIATEMESSAGES


            switch (m->GetType())
            {
                // 
                // All the messages that would normally be sent to all processes,
                // except the end message.
            case kInitialiseMessage:
            case kTerminateMessage:
            case kRegisterMessage:
            case kStartMessage:
                SendToAllProcesses(&action);
                break;

                //
                // Completely ignore null and end messages if they have been placed
                // on this queue
            case kEndMessage:
            case kNullMessage:
                break;

                //
                // Usually we just want to do this
            default:
                error = m->GetDestination()->WakeUp(&action);
                break;
            } // switch (m->GetType())

#ifdef DEBUG_POSTOFFICE_DONEXTMESSAGE
            LOG_DEBUG << "Message processed in " << Utility::StopTimer(t) << " s";
#endif // #ifdef DEBUG_POSTOFFICE_DONEXTMESSAGE

            delete m;
            m_sendingImmediate = false;
        } // while ( ! m_immediateMessages.empty() )

        m_sendingImmediate = false;
    }


    ProcessTime_t PostOffice::CalculateTimeSpan(int years, int months, int days, int hours, int minutes, int seconds)
    {
        auto d = years * 365 + months * 31 + days;
        return ((d * (60 * 60 * 24)) + (hours * (60 * 60)) + (minutes * 60) + seconds);
    }


    //struct tm {
    //       int tm_sec;     /* seconds after the minute - [0,59] */
    //       int tm_min;     /* minutes after the hour - [0,59] */
    //       int tm_hour;    /* hours since midnight - [0,23] */
    //       int tm_mday;    /* day of the month - [1,31] */
    //       int tm_mon;     /* months since January - [0,11] */
    //       int tm_year;    /* years since 1900 */
    //       int tm_wday;    /* days since Sunday - [0,6] */
    //       int tm_yday;    /* days since January 1 - [0,365] */
    //       int tm_isdst;   /* daylight savings time flag */
    //       };
    // wxDateTime constructor:
    //(wxDateTime_t day,
    // Month month = Inv_Month, - Jan is 0
    // int year = Inv_Year,
    // wxDateTime_t hour = 0,
    // wxDateTime_t minute = 0,
    // wxDateTime_t second = 0,
    // wxDateTime_t millisec = 0)
    ProcessTime_t PostOffice::CalculateDateTime(int YY, int mm, int dd, int HH, int MM, int SS)
    {
        //LOG_DEBUG <<   "YY:" << YY << ", mm:" << mm << ", dd:" << dd
        //          << ", HH:" << HH << ", MM:" << MM << ", SS:" << SS;
        wxDateTime date_time(dd, (::wxDateTime::Month)(mm - 1), (::wxDateTime::Year)YY, HH, MM, SS, 0);
        //LOG_DEBUG << "Formatted:" << date_time.Format();
        //time_t ticks = date_time.GetTicks();
        //LOG_DEBUG << "date_time.GetTicks():" << ticks;
        //return ticks;

        return (date_time.GetTicks());
    }


    void PostOffice::LogMessages() const
    {
        RootMapLogDebug("------- " << m_messages->GetNumMessages() << " TIMED MESSAGES -------");
        m_messages->LogMessages();
        RootMapLogDebug("------- END TIMED MESSAGES -------");
        RootMapLogDebug(" Not showing " << m_immediateMessages.size() << " Immediate messages");
    }

    wxString PostOffice::LogTime(ProcessTime_t t)
    {
        wxDateTime dt(t);
        return dt.Format(_T("%Y-%m-%d %H:%M:%S"));
    }

    RootMapLoggerDefinition(AllProcessSender);

    AllProcessSender::AllProcessSender(ProcessActionDescriptor* actionDescriptor)
        : m_descriptor(actionDescriptor)
    {
        RootMapLoggerInitialisation("rootmap.AllProcessSender");
    }


    AllProcessSender::~AllProcessSender()
    {
        /* Do Nothing */
    }


    void AllProcessSender::UseProcess(Process* process)
    {
        RootMapLogTrace("UseProcess");

        if (process->GetProcessActivity() != paNONE)
        {
            ScoreboardStratum stratum = process->GetProcessStratum();
            if ((stratum != ScoreboardStratum::ALL) && (stratum != ScoreboardStratum::NONE))
            {
                m_descriptor->ResetScoreboard(stratum);
            }

            long error = kNoError;

            LOG_DEBUG << "Sending message {Type:'" << Utility::OSTypeToString(m_descriptor->GetCode())
                << "', Destination:'" << process->GetProcessName() << "'}";

            switch (m_descriptor->GetCode())
            {
            case kStartMessage:
                error = process->StartUp(m_descriptor);
                break;

            case kEndMessage:
                error = process->Ending(m_descriptor);
                break;

            case kInitialiseMessage:
                error = process->Initialise(m_descriptor);
                break;

            case kTerminateMessage:
                error = process->Terminate(m_descriptor);
                break;

            case kRegisterMessage:
                error = process->Register(m_descriptor);
                break;

            default:
                // Those are the only messages we send to all processes.
                // Ignore all others.
                break;
            } // switch (action->padCodeID)
        } // if (dest->GetProcessActivity() != paNONE)
    }


    void AllProcessSender::UseProcess(Process* /* process */, const ScoreboardStratum& /* stratum */)
    {
        /* Do Nothing */
    }
} /* namespace rootmap */

