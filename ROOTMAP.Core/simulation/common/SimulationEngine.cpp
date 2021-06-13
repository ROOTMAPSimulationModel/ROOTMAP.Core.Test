/////////////////////////////////////////////////////////////////////////////
// Name:        SimulationEngine.cpp
// Purpose:     Implementation of the SimulationEngine class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-10 06:43:25 +0800 (Fri, 10 Jul 2009) $
// $Revision: 69 $
// Copyright:   ?2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/common/SimulationEngine.h"
#include "simulation/common/ScoreboardProcessInitialisers.h"
#include "simulation/common/ISimulationActivityObserver.h"

#include "simulation/data_access/common/DataAccessManager.h"
#include "simulation/data_access/interface/ScoreboardDataDAI.h"
#include "simulation/process/common/ProcessCoordinator.h"
#include "simulation/process/common/ProcessExceptions.h"
#include "simulation/process/interprocess_communication/PostOffice.h"
#include "simulation/process/plant/PlantCoordinator.h"
#include "simulation/process/plant/Plant.h"
#include "simulation/process/plant/PlantSummarySupplier.h"
#include "simulation/process/shared_attributes/SharedAttributeManager.h"
#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"
#include "simulation/process/raytrace/RenderCoordinator.h"
#include "simulation/scoreboard/ScoreboardCoordinator.h"
#include "simulation/file/output/DataOutputCoordinator.h"
#include "simulation/process/modules/VolumeObjectCoordinator.h"

#include "core/common/RmAssert.h"

#include "wx/string.h"
#include "wx/datetime.h"

#include <cmath>

namespace rootmap
{
    class DataAccessManager;

    RootMapLoggerDefinition(SimulationEngine);

    SimulationEngine::SimulationEngine(DataAccessManager& dam)
        : m_phase(simNONE)
        , m_processCoordinator(0)
        , m_scoreboardCoordinator(0)
        , m_postOffice(0)
        , m_plantCoordinator(0)
        , m_dataAccessManager(dam)
        , m_dataOutputCoordinator(0)
        , m_renderCoordinator(0)
        , m_sharedAttributeManager(0)
        , m_volumeObjectCoordinator(0)
        , m_closeOnCompletion(false)
    {
        RootMapLoggerInitialisation("rootmap.SimulationEngine");

        wxDateTime now2(wxDateTime::Now());
        wxString logstr = now2.Format("\n\n\n------------------------ Engine Creation %Y/%m/%d %H:%M:%S ------------------------").c_str();
        RootMapLogNotice(logstr.c_str());
        wxDateTime zeroMoment((time_t)0);
        LOG_DEBUG << zeroMoment.Format("Zero time_t moment: %Y/%m/%d %H:%M:%S").c_str() << " wxDateTime::GetTicks()=" << zeroMoment.GetTicks();
        wxDateTime wxMaxMoment((time_t)LONG_MAX - 2);
        LOG_DEBUG << wxMaxMoment.Format("wxWidgets maximum time_t moment: %Y/%m/%d %H:%M:%S").c_str() << " wxDateTime::GetTicks()=" << wxMaxMoment.GetTicks();

        LOG_INFO << "SizeOf(double)=" << sizeof(double)
            << ", SizeOf(long int)=" << sizeof(long int)
            << ", SizeOf(CharacteristicColourInfo)=" << sizeof(CharacteristicColourInfo);

        // These four members (and m_dataOutputCoordinator below) reference the DataAccessManager in their constructors,
        // but do not retain it. This class owns m_dataAccessManager and deletes it upon destruction.
        m_sharedAttributeManager = new SharedAttributeManager(this, dam, "TheOnlyOne");
        m_scoreboardCoordinator = new ScoreboardCoordinator(*this, dam);
        m_processCoordinator = new ProcessCoordinator(*this, dam);
        m_postOffice = new PostOffice(*this, dam);

        Process* volumeObjectCoordinatorProcess = m_processCoordinator->FindProcessByProcessName("VolumeObjectCoordinator");
        if (0 != volumeObjectCoordinatorProcess)
        {
            VolumeObjectCoordinator* voCoordinator = dynamic_cast<VolumeObjectCoordinator *>(volumeObjectCoordinatorProcess);

            if (0 != voCoordinator)
            {
                m_volumeObjectCoordinator = voCoordinator;
            }
        }

        Process* plantCoordinatorProcess = m_processCoordinator->FindProcessByProcessName("PlantCoordinator");
        if (0 != plantCoordinatorProcess)
        {
            PlantCoordinator* plantCoordinator = dynamic_cast<PlantCoordinator *>(plantCoordinatorProcess);

            if (0 != plantCoordinator)
            {
                // Set the VolumeObjectCoordinator pointer.
                plantCoordinator->SetVolumeObjectCoordinator(*m_volumeObjectCoordinator);
                // GAH. Yuk. The PlantSummarySupplier really requires to be instantiate
                // with the PlantCoordinator and ScoreboardCoordinator. Once a supplier
                // is instantiated, there is typically no contact with the outside world
                // - it is self-sufficient awaiting use to supply attributes to a family.
                new PlantSummarySupplier(*plantCoordinator, *m_scoreboardCoordinator);
                RootMapLogInfo("PlantSummarySupplier instantiated");

                // More Yuk. Plants need to be included in the ProcessCoordinator's ALL processes list
                // so that they can be found by view windows. However plants are created during the
                // PlantCoordinator construction, which in turn is done during ProcessCoordinator
                // construction. So the PlantCoordinator is unable to call
                // processCoordinator->AddProcess(plant, ScoreboardStratum::ALL)
                // because the dang processCoordinator isn't available yet. So one guesses that this
                // must be done here. 
                // 
                // This raises the question of what we will need to do if we implement ProcessGroupLeader
                // and ProcessGroupMember classes. We would need to iterate finding ProcessGroupLeaders
                // and requesting their GroupMembers be added to the appropriate process list.
                PlantArray& plants = plantCoordinator->GetPlants();
                for (PlantArray::iterator plant_iter = plants.begin();
                    plant_iter != plants.end(); ++plant_iter)
                {
                    m_processCoordinator->AddProcess((*plant_iter), StratumALL /*ScoreboardStratum::ALL*/);
                }
            }
            else
            {
                RootMapLogCrit("Found process with PlantCoordinator name, but wasn't PlantCoordinator ?!?!?! (PlantSummarySupplier not instantiated)");
            }
        }
        else
        {
            RootMapLogCrit("Could not find PlantCoordinator process, PlantSummarySupplier not instantiated");
        }

        dam.constructSharedAttributes(m_sharedAttributeManager);
        m_dataOutputCoordinator = new DataOutputCoordinator(*this, dam);
        m_renderCoordinator = new RenderCoordinator(*this, dam);
    }

    SimulationEngine::~SimulationEngine()
    {
        SharedAttributeRegistrar::DeregisterAll();

        // MSA Note: DataOutputCoordinator destructor modified
        // to delete all OutputRules from the free store;
        // OutputRules' dtors delete all free store resources they own
        // (case in point: RaytracerDataOutputStrategy owns the common
        //  RaytracerData pointer. Revokes it from all other classes
        //  and deletes it upon destruction.)
        delete m_dataOutputCoordinator;
        m_dataOutputCoordinator = __nullptr;

        delete m_renderCoordinator;
        m_renderCoordinator = __nullptr;

        delete m_processCoordinator;
        m_processCoordinator = __nullptr;

        delete m_scoreboardCoordinator;
        m_scoreboardCoordinator = __nullptr;

        // ProcessCoordinator deletes this as part of its destruction.
        //delete m_volumeObjectCoordinator; 
        m_volumeObjectCoordinator = __nullptr;

        // Looks like SharedAttributeRegistrar::DeregisterAll() gets rid of this one
        //delete m_sharedAttributeManager; m_sharedAttributeManager=__nullptr;

        delete m_postOffice;
        m_postOffice = __nullptr;

        //delete m_plantCoordinator;
    }

    void SimulationEngine::Initialise()
    {
        InitialiseScoreboardCharacteristicDefaults();

        InitialiseScoreboardCharacteristicData();

        InitialiseProcessData();

        DoInitialise();

        m_sharedAttributeManager->LogSharedAttributeIteratorAssistantContents();

        //m_volumeObjectCoordinator->Initialise(*this);

        // Must initialise RenderCoordinator AFTER ProcessData
        m_renderCoordinator->Initialise();
    }

    void SimulationEngine::InitialiseScoreboardCharacteristicDefaults()
    {
        ScoreboardProcessDefaultInitialiser* initialiser = new ScoreboardProcessDefaultInitialiser(*this);
        m_scoreboardCoordinator->IterateOverScoreboards(initialiser);
        delete initialiser;
    }

    void SimulationEngine::InitialiseScoreboardCharacteristicData()
    {
        m_dataAccessManager.initialiseScoreboardData(this);
    }

    void SimulationEngine::InitialiseProcessData()
    {
        ScoreboardProcessDataInitialiser* initialiser = new ScoreboardProcessDataInitialiser(*this);
        m_processCoordinator->IterateOverProcesses(initialiser);
        delete initialiser;
    }

    void SimulationEngine::InitialiseScoreboardDataValues(ScoreboardDataDAI& data)
    {
        long characteristic_number = 0;
        Process* p = m_processCoordinator->FindProcessByCharacteristicName(data.getCharacteristicName(), &characteristic_number);

        if ((0 != p) && (data.getProcessName() == p->GetProcessName()))
        {
            CharacteristicIndex characteristic_index = p->GetCharacteristicIndex(characteristic_number);
            Scoreboard* sb = m_scoreboardCoordinator->GetScoreboard(p->GetCharacteristicScoreboardStratum(characteristic_number));
            RmAssert(0 != sb, "Scoreboard was not found to initialise data");

            const SchemeContentElementList& elements = data.getElementList();

            for (SchemeContentElementList::const_iterator iter = elements.begin();
                iter != elements.end(); ++iter)
            {
                const SchemeContentElement& element = (*iter);
                double value = element.m_value;
                switch (element.m_elementType)
                {
                case sceScoreboard:
                {
                    sb->FillCharacteristicWithValue(characteristic_index, value);
                }
                break;
                case scePlane:
                { // scope restriction for local variables
                    // NOTE that if there is more than one dimension specifier
                    // in the element's dimensions map, it should be logged as
                    // a warning at the parsing level. Here we just take the
                    // "first" node of the map, which could be any of the 
                    // specifications
                    Dimension dimension = (*(element.m_dimensions.begin())).dimension;
                    long dimension_index = (*(element.m_dimensions.begin())).dimension_value;

                    sb->SetPlaneCharacteristicValues(characteristic_index, dimension, dimension_index, value);
                }
                break;
                case sceRowColumn:
                {
                    long x_index = -1;
                    long y_index = -1;
                    long z_index = -1;

                    for (std::list<DimensionValuePair>::const_iterator dim_iter = element.m_dimensions.begin();
                        dim_iter != element.m_dimensions.end(); ++dim_iter)
                    {
                        switch ((*dim_iter).dimension)
                        {
                        case X:
                            x_index = (*dim_iter).dimension_value;
                            break;
                        case Y:
                            y_index = (*dim_iter).dimension_value;
                            break;
                        case Z:
                            z_index = (*dim_iter).dimension_value;
                            break;
                        case NoDimension:
                        default:
                            break;
                        }
                    }
                    sb->SetRowColumnCharacteristicValues(characteristic_index, x_index, y_index, z_index, value);
                }
                break;
                case sceBoxes:
                {
                    DimensionArray a1((*element.m_arrays.begin()));
                    // the input parser trims the list to 2, so we can safely do this
                    // which is good because we can't pop_front() due to const-ness
                    DimensionArray a2((*element.m_arrays.rbegin()));

                    BoxCoordinate start(a1.x, a1.y, a1.z);
                    BoxCoordinate end(a2.x, a2.y, a2.z);

                    sb->SetArbitraryBoxesCharacteristicValues(characteristic_index, start, end, value);
                }
                break;
                case sceBox:
                {
                    DimensionArray a1((*element.m_arrays.begin()));
                    sb->SetCharacteristicValue(characteristic_index, value, a1.x, a1.y, a1.z);
                }
                break;
                case sceInvalid:
                    throw (new RmException("Invalid input scheme element type"));
                    break;
                default:
                    throw (new RmException("Did not understand input scheme element type"));
                    break;
                }
            }
        }
        else
        {
            LOG_ALERT << LOG_LINE << "Did not find process to initialise scoreboard data {Characteristic:" << data.getCharacteristicName() << "}";
            //throw ProcessException("Did not find process to initialise scoreboard data",data.getProcessName().c_str());
        }
    }

    ProcessCoordinator& SimulationEngine::GetProcessCoordinator()
    {
        return *m_processCoordinator;
    }

    ScoreboardCoordinator& SimulationEngine::GetScoreboardCoordinator()
    {
        return *m_scoreboardCoordinator;
    }

    VolumeObjectCoordinator& SimulationEngine::GetVolumeObjectCoordinator()
    {
        return *m_volumeObjectCoordinator;
    }

    PostOffice& SimulationEngine::GetPostOffice()
    {
        return *m_postOffice;
    }

    const PostOffice& SimulationEngine::GetPostOffice() const
    {
        return *m_postOffice;
    }

    SharedAttributeManager& SimulationEngine::GetSharedAttributeManager()
    {
        return *m_sharedAttributeManager;
    }

    DataAccessManager& SimulationEngine::GetDataAccessManager()
    {
        return m_dataAccessManager;
    }

    size_t SimulationEngine::GetNumSimulationActivityObservers() const
    {
        return m_simulationActivityObservers.size();
    }

    void SimulationEngine::Run()
    {
        try
        {
            if (!IsBusy())
            {
                LOG_NOTE << "Starting simulation run";
                m_phase = simStarting;

                PostOffice& post_office = GetPostOffice();

                // notifies all observers of pre-run state
                NotifyPreRun();

                // Posts a start message for all the processes
                post_office.SendStart();

                // post the end message for when we want to end
                post_office.SendEnd(post_office.GetNow() + post_office.GetDefaultRunTime());

                bool finished = false;

                if (simStopRequested == m_phase)
                {
                    finished = true;
                }

                m_phase = simRunning;

                while (!finished)
                {
                    if (simStopRequested == m_phase)
                    {
                        LOG_NOTE << "Received request to stop simulation run";
                        post_office.SendEnd(post_office.GetNow());
                    }
                    if (simPauseRequested == m_phase)
                    {
                        // MSA 09.10.15 Note that the "Pause" message doesn't actually
                        // do the pausing. It simply sits at the front of the
                        // PostOffice's message queue, looking pretty for the 
                        // ProgressWindow until the GUI user unpauses the simulation,
                        // at which point it's discarded.
                        LOG_NOTE << "Received request to pause simulation run";
                        post_office.SendPause();
                        m_phase = simPaused;
                    }

                    // notifies all observers of pre-timestamp state
                    NotifyPreTimestamp();

                    if (simPaused != m_phase)
                    {
                        post_office.LogMessages();

                        // MSA 11.03.03 Implementing a more granular alternative to DoNextTimeStamp(),
                        // to allow for more frequent UI updates (for more responsive UI and more accurate progress window)

                        const static bool reportEachMessage = true;

                        if (reportEachMessage)
                        {
                            post_office.DoImmediateMessages(
                                &(GetScoreboardCoordinator()),
                                &(GetProcessCoordinator()),
                                &(GetSharedAttributeManager()),
                                &(GetVolumeObjectCoordinator()));

                            const ProcessTime_t nextTimeStamp = post_office.GetNextTimeStamp();

                            while (!finished && post_office.GetNextTimeStamp() <= nextTimeStamp)
                            {
                                finished = post_office.DoNextMessage(
                                    &(GetScoreboardCoordinator()),
                                    &(GetProcessCoordinator()),
                                    &(GetSharedAttributeManager()),
                                    &(GetVolumeObjectCoordinator()));
                                NotifyMidTimestamp();
                            }

                            if (!finished)
                            {
                                post_office.DoImmediateMessages(
                                    &(GetScoreboardCoordinator()),
                                    &(GetProcessCoordinator()),
                                    &(GetSharedAttributeManager()),
                                    &(GetVolumeObjectCoordinator()));
                            }
                        }
                        else
                        {
                            finished = post_office.DoNextTimeStamp(
                                &(GetScoreboardCoordinator()),
                                &(GetProcessCoordinator()),
                                &(GetSharedAttributeManager()),
                                &(GetVolumeObjectCoordinator()));
                        }
                    }
                    // notifies all observers of post-timestamp state
                    NotifyPostTimestamp();
                }

                m_phase = simStopping;

                // notifies all observers of post-run state
                NotifyPostRun();

                post_office.LogMessages();

                m_phase = simIdling;
                LOG_NOTE << "Ending simulation run";

                if (m_closeOnCompletion)
                {
                    CloseSimulation();
                }
            }
            else
            {
                LOG_NOTE << "Simulation run already in progress, not starting another";
            }

        }
        catch (const RmException& ex)
        {
            std::string what = ex.what();
            std::string msg = "Caught RmException reason: " + what;
            LOG_ERROR << msg;
            throw;
        }
    }

    // MSA 09.10.13 New method to allow suspending a simulation without resetting it
    void SimulationEngine::PauseToggle()
    {
        if (simRunning == m_phase)
        {
            m_phase = simPauseRequested;
        }
        else if (simPaused == m_phase)
        {
            LOG_NOTE << "Received request to resume simulation run";
            m_phase = simRunning;
        }
        // else do nothing (this method's GUI control should be disabled in this case, anyway)
    }

    void SimulationEngine::Stop()
    {
        // will need to use a proper semaphore when the Run() is executed in
        // a separate thread
        if (m_phase != simIdling && m_phase != simNONE)
        {
            m_phase = simStopRequested;
        }
    }

    void SimulationEngine::RegisterSimulationActivityObserver(ISimulationActivityObserver* observer)
    {
        m_simulationActivityObservers.push_back(observer);
    }

    void SimulationEngine::RemoveSimulationActivityObserver(ISimulationActivityObserver* observer)
    {
        m_simulationActivityObservers.remove(observer);
    }

    /*
     * PostOffice Dependency:
     *
     * The PostOffice depends on the kRegisterMessage being the first sent -
     * it won't send any messages on the immediate queue before it. If this
     * changes here (any other message is sent first) then also modify
     * PostOffice::SendToAllProcesses
     */
    void SimulationEngine::DoInitialise()
    {
        PostOffice& post_office = GetPostOffice();

        LOG_INFO << "Preparing to send all active processes the Register message";
        // set up the action descriptor
        ProcessActionDescriptor registerer
        (post_office.GetNow() - 1,// ProcessTime_t
            kRegisterMessage, // MessageType_t
            __nullptr, // SpecialProcessData *
            __nullptr, // Process *
            &(post_office),
            __nullptr, // Scoreboard *
            &(GetScoreboardCoordinator()),
            &(GetProcessCoordinator()),
            &(GetSharedAttributeManager()),
            &(GetVolumeObjectCoordinator()));

        // Get the post office to send the register message to all concerned
        post_office.SendToAllProcesses(&registerer);

        // MSA 10.09.22 Clearing the immediate message queue 
        // to ensure all plant summaries etc. are initialised before the DataOutputCoordinator
        post_office.DoImmediateMessages(m_scoreboardCoordinator, m_processCoordinator, m_sharedAttributeManager, m_volumeObjectCoordinator);

        //
        // the outputs may attempt to initialise to characteristics that are
        // actually shared attributes or plant summaries, so they need to be
        // initialised after all registrations
        LOG_INFO << "Initialising output rules";
        m_dataAccessManager.initialiseOutputs(m_dataOutputCoordinator);

        LOG_INFO << "Preparing to send all active processes the Initialise message";
        // set up the action descriptor
        ProcessActionDescriptor initialiser
        (post_office.GetNow() - 1,// ProcessTime_t
            kInitialiseMessage, // MessageType_t
            __nullptr, // SpecialProcessData *
            __nullptr, // Process *
            &(post_office),
            __nullptr, // Scoreboard *
            &(GetScoreboardCoordinator()),
            &(GetProcessCoordinator()),
            &(GetSharedAttributeManager()),
            &(GetVolumeObjectCoordinator()));

        // Get the post office to send the initialise message to all concerned
        post_office.SendToAllProcesses(&initialiser);

        // Also inform scoreboard coord w.r.t. rt caching

        /* This portion is to have all initial queued messages dealt with. That is,
        clear out all the messages caused by modules reacting to the initialise
        message. These messages are usually registrations of interest for various
        stuffs.

        By checking if the next message is greater than now, we deal with any and
        all that need sending before we get going. By fetching "Now" before we go about
        sending, we ensure that "Now" doesn't get updated to a time previous to "Now"
        when a message with a timestamp previous to "Now" gets sent.
        (This happens within the PostOffice when we send a message) */
        ProcessTime_t now = post_office.GetNow();
        post_office.SendEnd(now);

        LOG_INFO << "Clearing remaining initial messages";
        ProcessTime_t next_time = post_office.GetNextTimeStamp();
        bool finished_clearing_initial_messages = false;

        while ((!finished_clearing_initial_messages) &&
            (next_time <= now)
            )
        {
            finished_clearing_initial_messages =
                post_office.DoNextTimeStamp
                (&(GetScoreboardCoordinator()),
                    &(GetProcessCoordinator()),
                    &(GetSharedAttributeManager()),
                    &(GetVolumeObjectCoordinator()));
            next_time = post_office.GetNextTimeStamp();
        }
    }

    void SimulationEngine::DoTerminate()
    {
        LOG_INFO << "Preparing to send all active processes the Terminate message";
        // set up the action descriptor
        ProcessActionDescriptor terminator
        (0, // time
            kTerminateMessage, // MessageType_t
            __nullptr, // SpecialProcessData *
            __nullptr, // Process *
            &(GetPostOffice()), // PostOffice *
            __nullptr, // Scoreboard *
            &(GetScoreboardCoordinator()),
            &(GetProcessCoordinator()),
            &(GetSharedAttributeManager()),
            &(GetVolumeObjectCoordinator()));

        // Get the post office to send the initialise message to all concerned
        GetPostOffice().SendToAllProcesses(&terminator);
    }


    void SimulationEngine::NotifyPreRun()
    {
        RootMapLogInfo("Simulation Run Starting");

        for (SimulationActivityObserverCollection::iterator iter = m_simulationActivityObservers.begin();
            iter != m_simulationActivityObservers.end(); ++iter)
        {
            (*iter)->PreRunNotification(GetPostOffice());
        }
    }

    void SimulationEngine::NotifyPreTimestamp()
    {
        RootMapLogDebug("Simulation Pre-Timestamp Notification");
        for (SimulationActivityObserverCollection::iterator iter = m_simulationActivityObservers.begin();
            iter != m_simulationActivityObservers.end(); ++iter)
        {
            (*iter)->PreTimestampNotification(GetPostOffice());
        }
    }

    void SimulationEngine::NotifyMidTimestamp()
    {
        RootMapLogDebug("Simulation Mid-Timestamp Notification");
        for (SimulationActivityObserverCollection::iterator iter = m_simulationActivityObservers.begin();
            iter != m_simulationActivityObservers.end(); ++iter)
        {
            (*iter)->MidTimestampNotification(GetPostOffice());
        }
    }

    void SimulationEngine::NotifyPostTimestamp()
    {
        RootMapLogDebug("Simulation Post-Timestamp Notification");
        PostOffice& po = GetPostOffice();
        for (SimulationActivityObserverCollection::iterator iter = m_simulationActivityObservers.begin();
            iter != m_simulationActivityObservers.end(); ++iter)
        {
            (*iter)->PostTimestampNotification(po);
        }
    }

    void SimulationEngine::NotifyPostRun()
    {
        RootMapLogInfo("Simulation Run Complete");

        for (SimulationActivityObserverCollection::iterator iter = m_simulationActivityObservers.begin();
            iter != m_simulationActivityObservers.end(); ++iter)
        {
            (*iter)->PostRunNotification(GetPostOffice());
        }
    }

    void SimulationEngine::CloseSimulation()
    {
        // Not yet implemented for CLI ROOTMAP.
        // (does CLI ROOTMAP just use SimulationEngine directly,
        //  or will there be a GuiSimulationEngine-equivalent
        //  derived class CliSimulationEngine or something?)
    }
} /* namespace rootmap */
