/////////////////////////////////////////////////////////////////////////////
// Name:        TXConfigurationDataStore.h
// Purpose:     Declaration of the TXConfigurationDataStore class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#ifndef TXConfigurationDataStore_H
#define TXConfigurationDataStore_H

#include <string>
#include <map>
#include <list>

#include "simulation/data_access/common/NameLocationData.h"
#include "simulation/data_access/common/ConfigurationDataStore.h"

#include "core/log/Logger.h"

// un-namespaced classes
class TiXmlDocument;
class TiXmlElement;
class TiXmlText;

namespace rootmap
{
    /**
     * The four elements of configuration
     */
    class TXConfigurationDataStore : public ConfigurationDataStore
    {
    public:
        /**
         * ThisFunctionName
         *
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */
        void initialise();

        /**
         *
         * @param
         * @return
         */
        TiXmlDocument* getTxDocument();

        TXConfigurationDataStore(std::string absoluteConfigDir, std::string configFile = "rootmap.xml");
        virtual ~TXConfigurationDataStore();

        /**
         *
         * @param
         * @return
         */
        const std::string& getConfigDir() const;

        const std::string& getConfigFile() const;

    private:
        RootMapLoggerDeclaration();

        ///
        /// flag to indicate state of initialisation
        bool m_isInitialised;

        TiXmlDocument* m_configDocument;

        std::string m_configFile;

        std::string m_configDir;
    }; // class TXConfigurationDataStore

    inline TiXmlDocument* TXConfigurationDataStore::getTxDocument()
    {
        return m_configDocument;
    }

    inline const std::string& TXConfigurationDataStore::getConfigDir() const
    {
        return m_configDir;
    }

    inline const std::string& TXConfigurationDataStore::getConfigFile() const
    {
        return m_configFile;
    }
} /* namespace rootmap */

#endif // #ifndef TXConfigurationDataStore_H
