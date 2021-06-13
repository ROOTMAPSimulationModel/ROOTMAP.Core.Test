#ifndef ProcessActionDescriptor_H
#define ProcessActionDescriptor_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ClassTemplate.h
// Purpose:     Declaration of the ClassTemplate class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-03-03 03:36:51 +0900 (Tue, 03 Mar 2009) $
// $Revision: 43 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/common/Types.h"
#include "simulation/common/Types.h"

//#define PROCESSTIME_MIN LONG_MIN
//#define PROCESSTIME_MAX LONG_MAX

namespace rootmap
{
    class Process;
    class ProcessCoordinator;
    class PostOffice;
    class Scoreboard;
    class ScoreboardCoordinator;
    class ScoreboardStratum;
    class Simulation;
    class Message;
    class SharedAttributeManager;
    class SpecialProcessData;
    class VolumeObjectCoordinator;

    class ProcessActionDescriptor
    {
    public:
        ProcessActionDescriptor();
        ProcessActionDescriptor(ProcessTime_t time_,
            MessageType_t message_code,
            SpecialProcessData
            * data,
            Process* src,
            PostOffice* postoffice,
            Scoreboard* scoreboard,
            ScoreboardCoordinator* scoreboardcoordinator,
            ProcessCoordinator* processcoordinator,
            SharedAttributeManager
            * sharedattributemanager,
            VolumeObjectCoordinator
            * volumeobjectcoordinator
        );

        ProcessActionDescriptor(Message* message,
            PostOffice* postoffice,
            ScoreboardCoordinator* scoreboardcoordinator,
            ProcessCoordinator* processcoordinator,
            SharedAttributeManager
            * sharedattributemanager,
            VolumeObjectCoordinator
            * volumeobjectcoordinator
        );

        virtual ~ProcessActionDescriptor();

        ProcessTime_t GetTime();
        MessageType_t GetCode();
        SpecialProcessData* GetData();

        Process* GetSource();

        PostOffice* GetPostOffice();
        Scoreboard* GetScoreboard();

        ScoreboardCoordinator* GetScoreboardCoordinator();
        ProcessCoordinator* GetProcessCoordinator();
        SharedAttributeManager
            * GetSharedAttributeManager();

        VolumeObjectCoordinator* GetVolumeObjectCoordinator();


        /**
         * Resets the internal m_scoreboard value, based on the required stratum
         */
        void ResetScoreboard(const ScoreboardStratum& ss);

    private:
        // important variables
        ProcessTime_t m_Time;
        MessageType_t m_CodeID;
        SpecialProcessData* m_Data;

        // important processes
        Process* m_Source;

        // important objects
        PostOffice* m_PostOffice;
        Scoreboard* m_Scoreboard;
        ScoreboardCoordinator* m_ScoreboardCoordinator;
        ProcessCoordinator* m_ProcessCoordinator;
        SharedAttributeManager* m_SharedAttributeManager;
        VolumeObjectCoordinator* m_VolumeObjectCoordinator;
    };

    inline ProcessTime_t ProcessActionDescriptor::GetTime()
    {
        return m_Time;
    }


    inline OSType ProcessActionDescriptor::GetCode()
    {
        return m_CodeID;
    }


    inline SpecialProcessData* ProcessActionDescriptor::GetData()
    {
        return m_Data;
    }


    inline Process* ProcessActionDescriptor::GetSource()
    {
        return m_Source;
    }


    inline PostOffice* ProcessActionDescriptor::GetPostOffice()
    {
        return m_PostOffice;
    }


    inline Scoreboard* ProcessActionDescriptor::GetScoreboard()
    {
        return m_Scoreboard;
    }


    inline ScoreboardCoordinator* ProcessActionDescriptor::GetScoreboardCoordinator()
    {
        return m_ScoreboardCoordinator;
    }

    inline VolumeObjectCoordinator* ProcessActionDescriptor::GetVolumeObjectCoordinator()
    {
        return m_VolumeObjectCoordinator;
    }

    inline ProcessCoordinator* ProcessActionDescriptor::GetProcessCoordinator()
    {
        return m_ProcessCoordinator;
    }

    inline SharedAttributeManager* ProcessActionDescriptor::GetSharedAttributeManager()
    {
        return m_SharedAttributeManager;
    }
} /* namespace rootmap */

#endif     // ProcessActionDescriptor_H
