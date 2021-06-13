/////////////////////////////////////////////////////////////////////////////
// Name:        PersistedDataDirectory.h
// Purpose:     Declaration of the PersistedDataDirectory class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
/////////////////////////////////////////////////////////////////////////////
#ifndef PersistedDataDirectory_H
#define PersistedDataDirectory_H

#include <string>
#include <map>
#include <list>

#include "simulation/data_persistence/common/DirectoryData.h"

#include "core/log/Logger.h"

namespace rootmap
{
    /**
     * The PersistedDataDirectory is a class that stores information about the
     * data that is being or has been persisted.
     *
     * The four elements persisted are identical to configuration elements:
     * - configuration data itself (meta-config?)
     * - construction
     * - initialisation
     * - visualisation
     *
     * The persisted data in the returned lists is presented by pointer,
     * which remains the ownership of this class.
     */
    class PersistedDataDirectory
    {
    public:
        typedef std::multimap<std::string, DirectoryData> DirectoryDataMap;

        /** The configuration data in the returned lists is present by pointer,
         * which remains the ownership of this class.
         */
         // ////////////////////////////////////////////////////
         //
         // Data Retrieval
         //
         // ////////////////////////////////////////////////////
        DirectoryDataList getConfigurationData(const std::string& owner);
        DirectoryDataList getConstructionData(const std::string& owner);
        DirectoryDataList getInitialisationData(const std::string& owner);
        DirectoryDataList getVisualisationData(const std::string& owner);

        // ////////////////////////////////////////////////////
        //
        // Administration
        //
        // ////////////////////////////////////////////////////

        /**
         *
         */
        virtual void initialise() = 0;

        /**
         * Called by the SimulationEngine via the default DataAccessManager
         * implementation after all the construction, initialisation and
         * visualisation is complete. This is the time to de-allocate memory,
         * as it will no longer be used.'
         *
         * This default implementation erases all data in all maps. Ensure any
         * override calls this inherited method.
         */
        virtual void cleanup();

        void addConfigurationData(const std::string& owner, const std::string& name, const std::string& location);
        void addConstructionData(const std::string& owner, const std::string& name, const std::string& location);
        void addInitialisationData(const std::string& owner, const std::string& name, const std::string& location);
        void addVisualisationData(const std::string& owner, const std::string& name, const std::string& location);

        void logAllData();

        virtual ~PersistedDataDirectory();

    protected:
        PersistedDataDirectory();

    private:
        RootMapLoggerDeclaration();

        ///
        /// map of DirectoryData, this one for actual config data
        DirectoryDataMap myConfigurationData;

        ///
        /// map of DirectoryData, this one for construction data
        DirectoryDataMap myConstructionData;

        ///
        /// map of DirectoryData, this one for intialisation data
        DirectoryDataMap myInitialisationData;

        ///
        /// map of DirectoryData, this one for visualisation data
        DirectoryDataMap myVisualisationData;

        static void logDirectoryDataMap(const DirectoryDataMap& nldm, const char* label);
    }; // class PersistedDataDirectory
} /* namespace rootmap */

#endif // #ifndef PersistedDataDirectory_H
