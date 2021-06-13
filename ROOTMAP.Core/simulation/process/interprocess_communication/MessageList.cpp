/////////////////////////////////////////////////////////////////////////////
// Name:        MessageList.cpp
// Purpose:     Implementation of the MessageList class
// Created:     1994
// Author:      RvH
// $Date: 2009-03-03 03:36:51 +0900 (Tue, 03 Mar 2009) $
// $Revision: 43 $
// Copyright:   ©2002-2009 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/interprocess_communication/MessageList.h"
#include "simulation/process/interprocess_communication/Message.h"

#include "core/utility/Utility.h"


namespace rootmap
{
    RootMapLoggerDefinition(MessageList);

    MessageList::MessageList()
    {
        RootMapLoggerInitialisation("rootmap.MessageList");
        Message::InitialiseMessageTypePrioritiesMap();
    }

    MessageList::~MessageList()
    {
        MessageListType::iterator it(m_messageList.begin());
        for (; it != m_messageList.end(); ++it)
        {
            delete (*it);
        }
    }

    Message* MessageList::Add(ProcessTime_t t, Process* src, Process* dest, SpecialProcessData* d, MessageType_t mt)
    {
        Message* new_message = new Message(t, src, dest, d, mt);

        Add(new_message);

        return new_message;
    }

    void MessageList::AddStart()
    {
        /* Message * new_message = */
        Add(PeekNextTime(), __nullptr, __nullptr, __nullptr, kStartMessage);
    }

    void MessageList::AddPause()
    {
        // Adds a Pause message to the front of the list (hence the -1)
        /* Message * new_message = */
        Add(PeekNextTime() - 1, __nullptr, __nullptr, __nullptr, kPauseMessage);
    }

    void MessageList::AddEnd(ProcessTime_t t)
    {
        for (MessageListType::iterator it(m_messageList.begin());
            it != m_messageList.end(); ++it)
        {
            if ((*it)->MatchParameters(__nullptr, __nullptr, __nullptr, kEndMessage))
            {
                delete (*it);

                m_messageList.erase(it);

                // reset our iterator. In theory, we shouldn't need to do this more
                // than once, because if this function works we should only ever
                // have at most 1 end message in the list
                it = m_messageList.begin();
            }
        }

        // now add a new end message
        Add(t, __nullptr, __nullptr, __nullptr, kEndMessage);
    }

    bool MessageList::Remove(ProcessTime_t t, Process* src, Process* dest, SpecialProcessData* d, MessageType_t mt)
    {
        LOG_DEBUG << "Removing message, Source:" << src
            << ", Destination:" << dest
            << ", Data:" << d
            << ", Type:" << Utility::OSTypeToString(mt)
            << ", Time:" << t;

        Message::ParameterMatch mpm(t, src, dest, d, mt);
        MessageListType::iterator fnd = std::remove_if(m_messageList.begin(), m_messageList.end(), mpm);

        return (fnd != m_messageList.end());

#if defined NORMAL_IMPLEMENTATION
        for (MessageListType::iterator it(m_messageList.begin());
            it != m_messageList.end(); ++it)
        {
            if ((*it)->MatchParameters(t, src, dest, d, mt))
            {
                m_messageList.erase(it);
                return true;
            }
        }

        return false;
#endif
    }

    bool MessageList::Remove(Process* src, Process* dest, SpecialProcessData* d, MessageType_t mt)
    {
        LOG_DEBUG << "Removing message, Source:" << src
            << ", Destination:" << dest
            << ", Data:" << d
            << ", Type:" << Utility::OSTypeToString(mt);

        Message::ParameterMatch mpm(src, dest, d, mt);
        MessageListType::iterator fnd = std::remove_if(m_messageList.begin(), m_messageList.end(), mpm);

        return (fnd != m_messageList.end());

#if defined NORMAL_IMPLEMENTATION
        for (MessageListType::iterator it(m_messageList.begin());
            it != m_messageList.end(); ++it)
        {
            if ((*it)->MatchParameters(src, dest, d, mt))
            {
                m_messageList.erase(it);
                return true;
            }
        }

        return false;
#endif
    }


    Message* MessageList::Pop()
    {
        if (!m_messageList.empty())
        {
            Message* m = m_messageList.front();
            m_messageList.pop_front();
            return m;
        }

        return __nullptr;
    }

    ProcessTime_t MessageList::PeekNextTime()
    {
        if (!m_messageList.empty())
        {
            return ((m_messageList.front())->GetTime());
        }

        return (0);
    }

    Process* MessageList::PeekNextProcess()
    {
        if (!m_messageList.empty())
        {
            return ((m_messageList.front())->GetDestination());
        }

        return (__nullptr);
    }

    void MessageList::PeekNextDestinationName(std::string& name, bool fAppend)
    {
        if (!m_messageList.empty())
        {
            if (fAppend)
            {
                name.append((m_messageList.front())->StringOfDestination());
            }
            else
            {
                name = (m_messageList.front())->StringOfDestination();
            }
        }
        else
        {
            name = "";
        }
    }

    void MessageList::PeekNextTypeName(std::string& name, bool fAppend)
    {
        if (!m_messageList.empty())
        {
            if (fAppend)
            {
                name.append((m_messageList.front())->StringOfType());
            }
            else
            {
                name = (m_messageList.front())->StringOfType();
            }
        }
        else
        {
            name = "";
        }
    }

    void MessageList::Add(Message* new_message)
    {
        LOG_DEBUG << "Adding message:" << *new_message;

        MessageListType::iterator it(m_messageList.begin());
        for (; it != m_messageList.end(); ++it)
        {
            if ((*new_message) < (*(*it)))
                //if (new_message->InsertBefore( *(*it) ) )
            {
                break;
            }
        }

        m_messageList.insert(it, new_message);
    }


    void MessageList::LogMessages() const
    {
        for (MessageListType::const_iterator it(m_messageList.begin());
            it != m_messageList.end(); ++it)
        {
            (*it)->LogDebug();
        }
    }
} /* namespace rootmap */

