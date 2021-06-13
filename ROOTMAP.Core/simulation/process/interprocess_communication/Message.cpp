/////////////////////////////////////////////////////////////////////////////
// Name:        Message.cpp
// Purpose:     Implementation of the Message class
// Created:     1994
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/interprocess_communication/Message.h"
#include "simulation/process/common/Process_Dictionary.h"
#include "simulation/process/common/Process.h"
#include "simulation/common/SprintUtility.h"
#include "core/utility/Utility.h"


//
// Comment out the following line to log timestamps as the raw seconds.
#define LOGTIME_STRING
#if defined LOGTIME_STRING
#define LOGTIME(t) PostOffice::LogTime(t)
#endif // #if defined LOGTIME_STRING
#ifndef LOGTIME
#define LOGTIME(t) t
#endif // #ifndef LOGTIME


namespace rootmap
{
    RootMapLoggerDefinition(Message);

    // Most member functions are implemented in the header file inline
    Message::Message
    (ProcessTime_t t,
        Process* source,
        Process* destination,
        SpecialProcessData* specialData,
        MessageType_t messageType
    )
        : m_time(t)
        , m_source_process(source)
        , m_destination_process(destination)
        , m_data(specialData)
        , m_type(messageType)
        , m_priority(FindPriority(messageType))
    {
        RootMapLoggerInitialisation("rootmap.Message");
    }

    bool Message::MatchParameters
    (Process* src,
        Process* dest,
        SpecialProcessData* d,
        MessageType_t mess_type
    )
    {
        return ((src == m_source_process) &&
            (dest == m_destination_process) &&
            (d == m_data) &&
            (mess_type == m_type));
    }

    bool Message::EqualTo(const Message& rhs) const
    {
        return ((rhs.m_source_process == m_source_process) &&
            (rhs.m_destination_process == m_destination_process) &&
            //( rhs.m_time == m_time ) && time is immaterial for searches
            (rhs.m_priority == m_priority) &&
            (rhs.m_data == m_data));
    }

    bool Message::SortBefore(const Message& rhs) const
    {
        if (m_time < rhs.m_time)
        {
            return true;
        }
        else if ((m_time == rhs.m_time) &&
            (m_priority < rhs.m_priority))
        {
            return true;
        }

        return false;
    }

    bool Message::operator<(const Message& rhs) const
    {
        return SortBefore(rhs);
    }


    std::string Message::StringOfDestination() const
    {
        switch (m_type)
        {
        case kNullMessage:
            return (std::string("None"));
            break;
        case kInitialiseMessage:
        case kStartMessage:
        case kEndMessage:
        case kTerminateMessage:
            return (std::string("All Active Processes"));
            break;
        case kPauseMessage:
            return (std::string("Simulation Engine"));
            break;
        default:
            if (0 != m_destination_process)
            {
                return m_destination_process->GetProcessName();
            }
            else
            {
                return (std::string("Unexpected NULL"));
            }
            break;
        }

        //return (std::string("<UNKNOWN>")); // MSA 09.10.07 Unused
    }


    std::string Message::StringOfSource() const
    {
        if (0 != m_source_process)
        {
            return m_source_process->GetProcessName();
        }

        return (std::string("<NULL>"));
    }


    std::string Message::StringOfType(bool plusExtra) const
    {
        return SprintUtility::StringOf(m_type, plusExtra);
    }

    void Message::LogDebug() const
    {
        LOG_DEBUG << (*this);
    }

    std::ostream& operator<<(std::ostream& s, const Message& m)
    {
        s << "Time:" << LOGTIME(m.GetTime()) <<
            ", Src:" << m.StringOfSource() <<
            ", Dest:" << m.StringOfDestination() <<
            ", Type:" << m.StringOfType() <<
            ", Data:" << m.GetData() <<
            ", Priority:" << m.m_priority;

        return s;
    }

    const Message::MessagePriority Message::sMaxMessagePriority = 100;
    Message::MessageTypePriorityMap Message::m_messageTypePriorityMap;

    void Message::InitialiseMessageTypePrioritiesMap()
    {
        // Appearance in order of priority
        m_messageTypePriorityMap[kStartMessage] = 0;

        m_messageTypePriorityMap[kInitialiseMessage] = 5;

        m_messageTypePriorityMap[kTerminateMessage] = 8;

        m_messageTypePriorityMap[kPreNormalMessage] = 39;

        m_messageTypePriorityMap[kNormalMessage] = 40;
        m_messageTypePriorityMap[kRegisteredCoughUpMessage] = 40;
        m_messageTypePriorityMap[kUnsolicitedCoughUpMessage] = 40;
        m_messageTypePriorityMap[kRegisterRequestMessage] = 40;
        m_messageTypePriorityMap[kNullMessage] = 40;

        m_messageTypePriorityMap[kSpecialInputDataMessage] = 45;

        m_messageTypePriorityMap[kRegisterGlobalPlantSummaryMessage] = 50;
        m_messageTypePriorityMap[kRegisterScoreboardPlantSummaryMessage] = 50;

        m_messageTypePriorityMap[kCoughUpImmediateMessage] = 60;

        m_messageTypePriorityMap[kExternalWakeUpMessage] = 70;

        m_messageTypePriorityMap[kGlobalPlantSummaryRegisteredMessage] = 80;
        m_messageTypePriorityMap[kSelfCoughedUpImmediateMessage] = 80;
        m_messageTypePriorityMap[kOtherCoughedUpImmediateMessage] = 80;
        m_messageTypePriorityMap[kScoreboardPlantSummaryRegisteredMessage] = 80;

        m_messageTypePriorityMap[kReceiverDelayedReactionMessage] = 96;
        m_messageTypePriorityMap[kSpecialOutputDataMessage] = 97;

        m_messageTypePriorityMap[kEndMessage] = 100;
    }


    Message::MessagePriority Message::FindPriority(MessageType_t messageType)
    {
        MessageTypePriorityMap::iterator found = m_messageTypePriorityMap.find(messageType);
        if (m_messageTypePriorityMap.end() != found)
        {
            return (found->second);
        }

        return sMaxMessagePriority;
    }


    bool Message::ParameterMatch::operator()(Message* m)
    {
        if ((m->GetSource() == m_src) &&
            (m->GetDestination() == m_dst) &&
            (m->GetData() == m_data) &&
            (m->GetType() == m_type)
            )
        {
            if (m_time_set)
            {
                //
                return (m->GetTime() == m_time);
            }
            else
            {
                // already have enough matches with the 4 params already
                return true;
            }
        }

        return false;
    }
} /* namespace rootmap */

