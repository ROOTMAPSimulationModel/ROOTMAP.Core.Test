#ifndef DummyDataAccessManager_H
#define DummyDataAccessManager_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DummyDataAccessManager.h
// Purpose:     Declaration of the DummyDataAccessManager class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
// The DummyDataAccessManager is an abstract class that represents what is used by
// the SimulationEngine to form its simulation. The visitor pattern is used.
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/DataAccessManager.h"

namespace rootmap
{
    class DummyConfigurationDataStore;

    class DummyDataAccessManager : public DataAccessManager
    {
    public:
        void constructScoreboards(ScoreboardCoordinator* scoord);
        void constructProcesses(ProcessCoordinator* pcoord);
        void constructPlants(PlantCoordinator* pcoord);
        void constructOutputs(DataOutputCoordinator* docoord);
        void constructPostOffice(PostOffice* poffice);
        void constructSharedAttributes(SharedAttributeManager* sam);

        // void construct(XXXCoordinator * scoord);
        // void construct(XXXCoordinator * scoord);

        void initialiseScoreboardData(SimulationEngine* sengine);

        void visualiseTables(ITableDataUser* tdatauser);
        void visualiseViews(IViewDataUser* vdatauser);


        DummyDataAccessManager();
        virtual ~DummyDataAccessManager();

        /**
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */
    protected:

    private:

        ///
        /// member declaration

        ///
        /// member declaration
        DummyConfigurationDataStore* m_txConfigurationDataStore;
    }; // class DummyDataAccessManager
} /* namespace rootmap */

#endif // #ifndef DummyDataAccessManager_H
