/////////////////////////////////////////////////////////////////////////////
// Name:        ConfigurationDataStore.h
// Purpose:     Declaration of the ConfigurationDataStore class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
// The ConfigurationDataStore is a class that Stores Configuration Data ! The
// actual data that can be stored is in this class - the derived classes
// provide an implementation for initialise() that populates the internal
// maps
/////////////////////////////////////////////////////////////////////////////
#ifndef ConfigurationDataStore_H
#define ConfigurationDataStore_H

#include <string>
#include <map>
#include <list>

#include "simulation/data_access/common/NameLocationData.h"

#include "core/log/Logger.h"

namespace rootmap
{
    /**
     * The four elements of configuration are
     * - configuration data itself (meta-config?)
     * - construction
     * - initialisation
     * - visualisation
     *
     * The configuration data in the returned lists is presented by pointer,
     * which remains the ownership of this class.
     */
    class ConfigurationDataStore
    {
    public:
        typedef std::multimap<std::string, NameLocationData> NameLocationDataMap;

        typedef bool(*HasOwnerTypeDataFunction)(const std::string& owner);

        // ////////////////////////////////////////////////////
        //
        // Data Presence Requests 
        //
        // ////////////////////////////////////////////////////
        bool hasConfigurationData(const std::string& owner);
        bool hasConstructionData(const std::string& owner);
        bool hasInitialisationData(const std::string& owner);
        bool hasVisualisationData(const std::string& owner);

        /** The configuration data in the returned lists is present by pointer,
         * which remains the ownership of this class.
         */
         // ////////////////////////////////////////////////////
         //
         // Data Retrieval
         //
         // ////////////////////////////////////////////////////
        NameLocationDataList getConfigurationData(const std::string& owner);
        NameLocationDataList getConstructionData(const std::string& owner);
        NameLocationDataList getInitialisationData(const std::string& owner);
        NameLocationDataList getVisualisationData(const std::string& owner);

        // ////////////////////////////////////////////////////
        //
        // Administration
        //
        // ////////////////////////////////////////////////////


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

    protected:
        ConfigurationDataStore();
        virtual ~ConfigurationDataStore();

        void addConfigurationData(const std::string& owner, const std::string& name, const std::string& location);
        void addConstructionData(const std::string& owner, const std::string& name, const std::string& location);
        void addInitialisationData(const std::string& owner, const std::string& name, const std::string& location);
        void addVisualisationData(const std::string& owner, const std::string& name, const std::string& location);

        void logAllData();

    private:
        RootMapLoggerDeclaration();

        ///
        /// map of NameLocationData, this one for actual config data
        NameLocationDataMap myConfigurationData;

        ///
        /// map of NameLocationData, this one for construction data
        NameLocationDataMap myConstructionData;

        ///
        /// map of NameLocationData, this one for intialisation data
        NameLocationDataMap myInitialisationData;

        ///
        /// map of NameLocationData, this one for visualisation data
        NameLocationDataMap myVisualisationData;

        static void logNameLocationDataMap(const NameLocationDataMap& nldm, const char* label);
    }; // class ConfigurationDataStore
} /* namespace rootmap */

#endif // #ifndef ConfigurationDataStore_H
