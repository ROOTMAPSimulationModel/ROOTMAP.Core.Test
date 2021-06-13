/**
 *  ProcessActionDescriptor.cpp
 *
 *  Author:            Robert van Hugten
 *  Description:    <describe the ProcessActionDescriptor class here>
 */

#include "simulation/process/interprocess_communication/ProcessActionDescriptor.h"
#include "simulation/process/interprocess_communication/Message.h"
#include "simulation/scoreboard/ScoreboardCoordinator.h"


namespace rootmap
{
    ProcessActionDescriptor::ProcessActionDescriptor
    (ProcessTime_t time_,
        MessageType_t message_code,
        SpecialProcessData* data,
        Process* src,
        PostOffice* postoffice,
        Scoreboard* scoreboard,
        ScoreboardCoordinator* scoreboardcoordinator,
        ProcessCoordinator* processcoordinator,
        SharedAttributeManager* sharedattributemanager,
        VolumeObjectCoordinator* volumeobjectcoordinator)
        : m_Time(time_)
        , m_CodeID(message_code)
        , m_Data(data)
        , m_Source(src)
        , m_Scoreboard(scoreboard)
        , m_PostOffice(postoffice)
        , m_ScoreboardCoordinator(scoreboardcoordinator)
        , m_ProcessCoordinator(processcoordinator)
        , m_SharedAttributeManager(sharedattributemanager)
        , m_VolumeObjectCoordinator(volumeobjectcoordinator)
    {
    }

    ProcessActionDescriptor::ProcessActionDescriptor
    (Message* message,
        PostOffice* postoffice,
        ScoreboardCoordinator* scoreboardcoordinator,
        ProcessCoordinator* processcoordinator,
        SharedAttributeManager
        * sharedattributemanager,
        VolumeObjectCoordinator
        * volumeobjectcoordinator
    )
        : m_Time(message->GetTime())
        , m_CodeID(message->GetType())
        , m_Data(message->GetData())
        , m_Source(message->GetSource())
        , m_PostOffice(postoffice)
        , m_Scoreboard(__nullptr)
        , m_ScoreboardCoordinator(scoreboardcoordinator)
        , m_ProcessCoordinator(processcoordinator)
        , m_SharedAttributeManager(sharedattributemanager)
        , m_VolumeObjectCoordinator(volumeobjectcoordinator)
    {
    }

    ProcessActionDescriptor::ProcessActionDescriptor()
    {
    }


    ProcessActionDescriptor::~ProcessActionDescriptor()
    {
    }

    void ProcessActionDescriptor::ResetScoreboard(const ScoreboardStratum& ss)
    {
        m_Scoreboard = m_ScoreboardCoordinator->GetScoreboard(ss);
    }
} /* namespace rootmap */


