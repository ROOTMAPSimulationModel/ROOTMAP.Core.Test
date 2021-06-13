#ifndef DataAccessManager_H
#define DataAccessManager_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DataAccessManager.h
// Purpose:     Declaration of the DataAccessManager class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-20 10:30:16 +0800 (Fri, 20 Jun 2008) $
// $Revision: 7 $
// Copyright:   ©2006 University of Tasmania
//
// The DataAccessManager is an abstract class that represents what is used by
// the SimulationEngine to form its simulation. The visitor pattern is used.
/////////////////////////////////////////////////////////////////////////////
#include "core/common/Exceptions.h"

namespace rootmap
{
    class ConfigurationDataStore;
    class SharedAttributeManager;
    class ScoreboardCoordinator;
    class DataOutputCoordinator;
    class ProcessCoordinator;
    class RenderCoordinator;
    class VolumeObjectCoordinator;
    class SimulationEngine;
    class PlantCoordinator;
    class IView3DDataUser;
    class ITableDataUser;
    class IViewDataUser;
    class PostOffice;
    class Process;
    class ScoreboardDataPreviewer;
    class PlantDataPreviewer;
    class ProcessDataPreviewer;
    class OutputDataPreviewer;
    class VolumeObjectDataPreviewer;
    class PostOfficeDataPreviewer;
    class SharedAttributeDataPreviewer;
    class WindowDataPreviewer;


    class DataAccessManager
    {
    public:
        // ////////////////////////////////////////////////////
        //
        // Preloading/previewing
        //
        // ////////////////////////////////////////////////////
        /**
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */
        virtual void previewScoreboards(ScoreboardDataPreviewer* sdp) = 0;
        virtual void previewVolumeObjects(VolumeObjectDataPreviewer* vodp) = 0;
        virtual void previewProcesses(ProcessDataPreviewer* pdp) = 0;
        virtual void previewPlants(PlantDataPreviewer* pdp) = 0;
        virtual void previewOutputs(OutputDataPreviewer* odp) = 0;
        virtual void previewPostOffice(PostOfficeDataPreviewer* podp) = 0;
        virtual void previewSharedAttributes(SharedAttributeDataPreviewer* sadp) = 0;
        virtual void previewWindows(WindowDataPreviewer* wdp) = 0;
        // ////////////////////////////////////////////////////
        //
        // Construction
        //
        // ////////////////////////////////////////////////////
        /**
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */
        virtual void constructScoreboards(ScoreboardCoordinator* scoord) = 0;
        virtual void constructVolumeObjects(VolumeObjectCoordinator* scoord) = 0;
        virtual void constructProcesses(ProcessCoordinator* pcoord) = 0;
        virtual void constructPlants(PlantCoordinator* pcoord) = 0;
        virtual void constructOutputs(DataOutputCoordinator* docoord) = 0;
        virtual void constructPostOffice(PostOffice* poffice) = 0;
        virtual void constructSharedAttributes(SharedAttributeManager* sam) = 0;
        // void construct(XXXCoordinator * scoord) = 0;


        // ////////////////////////////////////////////////////
        //
        // Initialisation
        //
        // ////////////////////////////////////////////////////
        virtual void initialiseScoreboardData(SimulationEngine* sengine) = 0;

        /**
         * Whereas the constructOutputs is for information that requires no other
         * primary object (scoreboards, processes, postoffice) be initialised,
         * this initialise function is for the actual output rules, which require
         * the postoffice and destination output file alarm process.
         */
        virtual void initialiseOutputs(DataOutputCoordinator* dcoord) = 0;

        /**
         * This function searches for process-special/specific data to initialise
         * the given process with
         */
        virtual void initialiseProcessData(Process* process) = 0;

        /**
         * This function initialises CharacteristicColours_t data
         * for the ScoreboardRenderer
         */
        virtual void initialiseRenderer(RenderCoordinator* rcoord) = 0;

        // ////////////////////////////////////////////////////
        //
        // Visualisation
        //
        // ////////////////////////////////////////////////////
        virtual void visualiseTables(ITableDataUser* tdatauser) = 0;
        virtual void visualiseViews(IViewDataUser* vdatauser) = 0;
        virtual void visualiseView3D(IView3DDataUser* v3datauser) = 0;


        // ////////////////////////////////////////////////////
        //
        // Administration
        //
        // ////////////////////////////////////////////////////

        /**
         * Called by the SimulationEngine after all the construction,
         * initialisation and visualisation is complete. This is the time to
         * de-allocate memory, as it will no longer be used.
         *
         * The default implementation calls cleanup on the configuration store
         */
        virtual void cleanup();


        /**
         * Accessor for the datastore
         *
         * @return a reference to the datastore
         */
        ConfigurationDataStore& getConfigurationDataStore();

        virtual ~DataAccessManager();

    protected:
        DataAccessManager();

        /**
         * Called by the derived DataManager class after the config data store
         * is instantiated.
         *
         * @param
         * @return
         */
        void setDataStore(ConfigurationDataStore* configurationDataStore);

    private:

        ///
        /// member declaration
        ConfigurationDataStore* m_configurationDataStore;
    }; // class DataAccessManager

    inline ConfigurationDataStore& DataAccessManager::getConfigurationDataStore()
    {
        // RmAssert(m_configurationDataStore,"ConfigurationDataStore not yet set");
        //
        // but in the meantime, while RmAssert is being implemented,..
        if (m_configurationDataStore == 0)
        {
            throw RmException("ConfigurationDataStore not yet set");
        }
        return *m_configurationDataStore;
    }

    inline void DataAccessManager::setDataStore(ConfigurationDataStore* configurationDataStore)
    {
        m_configurationDataStore = configurationDataStore;
    }
} /* namespace rootmap */

#endif // #ifndef DataAccessManager_H
