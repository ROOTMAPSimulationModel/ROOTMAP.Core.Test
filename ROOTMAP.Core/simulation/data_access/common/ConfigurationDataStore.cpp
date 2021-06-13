/////////////////////////////////////////////////////////////////////////////
// Name:        ConfigurationDataStore.cpp
// Purpose:     Implementation of the ConfigurationDataStore class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/ConfigurationDataStore.h"

#include "wx/log.h"


namespace rootmap
{
    RootMapLoggerDefinition(ConfigurationDataStore);

    ConfigurationDataStore::ConfigurationDataStore()
    {
        RootMapLoggerInitialisation("rootmap.ConfigurationDataStore");
    }

    ConfigurationDataStore::~ConfigurationDataStore()
    {
    }


    // /////////////////////////////////////////
    // addXXData
    // /////////////////////////////////////////
    void ConfigurationDataStore::addConfigurationData(const std::string& owner, const std::string& name, const std::string& location)
    {
        NameLocationData cd(name, location);
        NameLocationDataMap::value_type location_t(owner, cd);

        myConfigurationData.insert(location_t);
    }


    void ConfigurationDataStore::addConstructionData(const std::string& owner, const std::string& name, const std::string& location)
    {
        NameLocationData cd(name, location);
        NameLocationDataMap::value_type location_t(owner, cd);

        myConstructionData.insert(location_t);
    }


    void ConfigurationDataStore::addInitialisationData(const std::string& owner, const std::string& name, const std::string& location)
    {
        NameLocationData cd(name, location);
        NameLocationDataMap::value_type location_t(owner, cd);

        myInitialisationData.insert(location_t);
    }


    void ConfigurationDataStore::addVisualisationData(const std::string& owner, const std::string& name, const std::string& location)
    {
        NameLocationData cd(name, location);
        NameLocationDataMap::value_type location_t(owner, cd);

        myVisualisationData.insert(location_t);
    }


    // /////////////////////////////////////////
    // hasXXData
    // /////////////////////////////////////////
    bool ConfigurationDataStore::hasConfigurationData(const std::string& owner)
    {
        NameLocationDataMap::iterator iter = myConfigurationData.find(owner);
        if (iter != myConfigurationData.end())
        {
            return true;
        }

        return false;
    }


    bool ConfigurationDataStore::hasConstructionData(const std::string& owner)
    {
        NameLocationDataMap::iterator iter = myConstructionData.find(owner);
        if (iter != myConstructionData.end())
        {
            return true;
        }

        return false;
    }


    bool ConfigurationDataStore::hasInitialisationData(const std::string& owner)
    {
        NameLocationDataMap::iterator iter = myInitialisationData.find(owner);
        if (iter != myInitialisationData.end())
        {
            return true;
        }

        return false;
    }


    bool ConfigurationDataStore::hasVisualisationData(const std::string& owner)
    {
        NameLocationDataMap::iterator iter = myVisualisationData.find(owner);
        if (iter != myVisualisationData.end())
        {
            return true;
        }

        return false;
    }


    // /////////////////////////////////////////
    // getXXData
    // /////////////////////////////////////////
    NameLocationDataList ConfigurationDataStore::getConfigurationData(const std::string& owner)
    {
        NameLocationDataList cdl;

        NameLocationDataMap::iterator iter = myConfigurationData.find(owner);
        if (iter != myConfigurationData.end())
        {
            for (NameLocationDataMap::iterator iiter = myConfigurationData.find(owner);
                iiter != myConfigurationData.end();
                ++iiter
                )
            {
                if ((*iiter).first == owner)
                {
                    NameLocationData cd((*iiter).second);
                    cdl.push_back(cd);
                }
            }
        }

        return cdl;
    }


    NameLocationDataList ConfigurationDataStore::getConstructionData(const std::string& owner)
    {
        NameLocationDataList cdl;

        NameLocationDataMap::iterator iter = myConstructionData.find(owner);
        if (iter != myConstructionData.end())
        {
            for (NameLocationDataMap::iterator iiter = myConstructionData.find(owner);
                iiter != myConstructionData.end();
                ++iiter
                )
            {
                if ((*iiter).first == owner)
                {
                    NameLocationData cd((*iiter).second);
                    cdl.push_back(cd);
                }
            }
        }

        return cdl;
    }


    NameLocationDataList ConfigurationDataStore::getInitialisationData(const std::string& owner)
    {
        NameLocationDataList cdl;

        NameLocationDataMap::iterator iter = myInitialisationData.find(owner);
        if (iter != myInitialisationData.end())
        {
            for (NameLocationDataMap::iterator iiter = myInitialisationData.find(owner);
                iiter != myInitialisationData.end();
                ++iiter
                )
            {
                if ((*iiter).first == owner)
                {
                    NameLocationData cd((*iiter).second);
                    cdl.push_back(cd);
                }
            }
        }

        return cdl;
    }


    NameLocationDataList ConfigurationDataStore::getVisualisationData(const std::string& owner)
    {
        NameLocationDataList cdl;

        NameLocationDataMap::iterator iter = myVisualisationData.find(owner);
        if (iter != myVisualisationData.end())
        {
            for (NameLocationDataMap::iterator iiter = myVisualisationData.find(owner);
                iiter != myVisualisationData.end();
                ++iiter
                )
            {
                if ((*iiter).first == owner)
                {
                    NameLocationData cd((*iiter).second);
                    cdl.push_back(cd);
                }
            }
        }

        return cdl;
    }


    void ConfigurationDataStore::logNameLocationDataMap(const ConfigurationDataStore::NameLocationDataMap& nldm, const char* label)
    {
        for (ConfigurationDataStore::NameLocationDataMap::const_iterator iter = nldm.begin();
            iter != nldm.end(); ++iter)
        {
            RootMapLogDebug(label << " Data, Owner " << (*iter).first
                << ", Name " << (*iter).second.getName()
                << ", Location " << (*iter).second.getLocation());
        }
    }

    void ConfigurationDataStore::logAllData()
    {
        logNameLocationDataMap(myConfigurationData, "Configuration");
        logNameLocationDataMap(myConstructionData, "Construction");
        logNameLocationDataMap(myInitialisationData, "Initialisation");
        logNameLocationDataMap(myVisualisationData, "Visualisation");
    }

    void ConfigurationDataStore::cleanup()
    {
        myConfigurationData.erase(myConfigurationData.begin(), myConfigurationData.end());
        myConstructionData.erase(myConfigurationData.begin(), myConfigurationData.end());
        myInitialisationData.erase(myConfigurationData.begin(), myConfigurationData.end());
        myVisualisationData.erase(myConfigurationData.begin(), myConfigurationData.end());
    }
} /* namespace rootmap */

