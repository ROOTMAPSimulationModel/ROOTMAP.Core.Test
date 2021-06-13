/////////////////////////////////////////////////////////////////////////////
// Name:        PersistedDataDirectory.cpp
// Purpose:     Implementation of the PersistedDataDirectory class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_persistence/common/PersistedDataDirectory.h"

#include "wx/log.h"


namespace rootmap
{
    RootMapLoggerDefinition(PersistedDataDirectory);

    PersistedDataDirectory::PersistedDataDirectory()
    {
        RootMapLoggerInitialisation("rootmap.PersistedDataDirectory");
    }

    PersistedDataDirectory::~PersistedDataDirectory()
    {
    }


    // /////////////////////////////////////////
    // addXXData
    // /////////////////////////////////////////
    void PersistedDataDirectory::addConfigurationData(const std::string& owner, const std::string& name, const std::string& location)
    {
        DirectoryData cd(name, location);
        DirectoryDataMap::value_type location_t(owner, cd);

        myConfigurationData.insert(location_t);
    }


    void PersistedDataDirectory::addConstructionData(const std::string& owner, const std::string& name, const std::string& location)
    {
        DirectoryData cd(name, location);
        DirectoryDataMap::value_type location_t(owner, cd);

        myConstructionData.insert(location_t);
    }


    void PersistedDataDirectory::addInitialisationData(const std::string& owner, const std::string& name, const std::string& location)
    {
        DirectoryData cd(name, location);
        DirectoryDataMap::value_type location_t(owner, cd);

        myInitialisationData.insert(location_t);
    }


    void PersistedDataDirectory::addVisualisationData(const std::string& owner, const std::string& name, const std::string& location)
    {
        DirectoryData cd(name, location);
        DirectoryDataMap::value_type location_t(owner, cd);

        myVisualisationData.insert(location_t);
    }


    // /////////////////////////////////////////
    // getXXData
    // /////////////////////////////////////////
    DirectoryDataList PersistedDataDirectory::getConfigurationData(const std::string& owner)
    {
        DirectoryDataList cdl;

        DirectoryDataMap::iterator iter = myConfigurationData.find(owner);
        if (iter != myConfigurationData.end())
        {
            for (DirectoryDataMap::iterator iiter = myConfigurationData.find(owner);
                iiter != myConfigurationData.end();
                ++iiter
                )
            {
                if ((*iiter).first == owner)
                {
                    DirectoryData cd((*iiter).second);
                    cdl.push_back(cd);
                }
            }
        }

        return cdl;
    }


    DirectoryDataList PersistedDataDirectory::getConstructionData(const std::string& owner)
    {
        DirectoryDataList cdl;

        DirectoryDataMap::iterator iter = myConstructionData.find(owner);
        if (iter != myConstructionData.end())
        {
            for (DirectoryDataMap::iterator iiter = myConstructionData.find(owner);
                iiter != myConstructionData.end();
                ++iiter
                )
            {
                if ((*iiter).first == owner)
                {
                    DirectoryData cd((*iiter).second);
                    cdl.push_back(cd);
                }
            }
        }

        return cdl;
    }


    DirectoryDataList PersistedDataDirectory::getInitialisationData(const std::string& owner)
    {
        DirectoryDataList cdl;

        DirectoryDataMap::iterator iter = myInitialisationData.find(owner);
        if (iter != myInitialisationData.end())
        {
            for (DirectoryDataMap::iterator iiter = myInitialisationData.find(owner);
                iiter != myInitialisationData.end();
                ++iiter
                )
            {
                if ((*iiter).first == owner)
                {
                    DirectoryData cd((*iiter).second);
                    cdl.push_back(cd);
                }
            }
        }

        return cdl;
    }


    DirectoryDataList PersistedDataDirectory::getVisualisationData(const std::string& owner)
    {
        DirectoryDataList cdl;

        DirectoryDataMap::iterator iter = myVisualisationData.find(owner);
        if (iter != myVisualisationData.end())
        {
            for (DirectoryDataMap::iterator iiter = myVisualisationData.find(owner);
                iiter != myVisualisationData.end();
                ++iiter
                )
            {
                if ((*iiter).first == owner)
                {
                    DirectoryData cd((*iiter).second);
                    cdl.push_back(cd);
                }
            }
        }

        return cdl;
    }


    void PersistedDataDirectory::logDirectoryDataMap(const PersistedDataDirectory::DirectoryDataMap& nldm, const char* label)
    {
        for (PersistedDataDirectory::DirectoryDataMap::const_iterator iter = nldm.begin();
            iter != nldm.end(); ++iter)
        {
            RootMapLogDebug(label << " Data, Owner " << (*iter).first
                << ", Name " << (*iter).second.getName()
                << ", Location " << (*iter).second.getLocation());
        }
    }

    void PersistedDataDirectory::logAllData()
    {
        logDirectoryDataMap(myConfigurationData, "Configuration");
        logDirectoryDataMap(myConstructionData, "Construction");
        logDirectoryDataMap(myInitialisationData, "Initialisation");
        logDirectoryDataMap(myVisualisationData, "Visualisation");
    }

    void PersistedDataDirectory::cleanup()
    {
        myConfigurationData.erase(myConfigurationData.begin(), myConfigurationData.end());
        myConstructionData.erase(myConfigurationData.begin(), myConfigurationData.end());
        myInitialisationData.erase(myConfigurationData.begin(), myConfigurationData.end());
        myVisualisationData.erase(myConfigurationData.begin(), myConfigurationData.end());
    }
} /* namespace rootmap */

