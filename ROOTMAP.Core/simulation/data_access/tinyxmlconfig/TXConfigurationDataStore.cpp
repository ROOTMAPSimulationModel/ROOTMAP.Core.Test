/////////////////////////////////////////////////////////////////////////////
// Name:        TXConfigurationDataStore.cpp
// Purpose:     Implementation of the TXConfigurationDataStore class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include <iostream>

#include "simulation/data_access/tinyxmlconfig/TXConfigurationDataStore.h"
#include "simulation/data_access/tinyxmlconfig/TXUtility.h"

#include "core/common/ExtraStuff.h"
#include "core/common/RmAssert.h"

#include "wx/filename.h"

#include "tinyxml.h"

namespace rootmap
{
    RootMapLoggerDefinition(TXConfigurationDataStore);

    TXConfigurationDataStore::TXConfigurationDataStore(std::string absoluteConfigDir, std::string configFile)
        : m_isInitialised(false)
        , m_configDocument(__nullptr)
        , m_configDir(absoluteConfigDir)
        , m_configFile(configFile)
    {
        RootMapLoggerInitialisation("rootmap.TXConfigurationDataStore");
    }

    TXConfigurationDataStore::~TXConfigurationDataStore()
    {
        delete m_configDocument;
    }


    void TXConfigurationDataStore::initialise()
    {
        if (m_isInitialised) return;

        auto configFile = m_configDir + "\\" + m_configFile;
        m_configDocument = new TiXmlDocument(configFile.c_str());
        bool loadOk = m_configDocument->LoadFile();

        if (loadOk)
        {
            RootMapLogDebug("Successfully loaded RootMap config file \"" << configFile << "\".");
        }
        else
        {
            RootMapLogError("Could not load RootMap config file \"" << configFile << "\", error=\"" << m_configDocument->ErrorDesc() << "\"");
            //TODO: throw a ConfigurationDataException with same string
            return;
        }

        /*
           EXAMPLE CONFIGURATION FILE CONTENT
        <rootmap>

          <!-- CONSTRUCTION CONFIGURATION -->
          <configuration>
            <name>Scoreboard Boundaries</name>
            <type>construction</type>
            <owner>ScoreboardCoordinator</owner>
            <location>Scoreboards.xml</location>
          </configuration>
          <configuration>
            <name>Plant Initial Values</name>
            <type>initialisation</type>
            <owner>PlantCoordinator</owner>
            <location>./shared attributes/PlantAttributeInitialValues.xml</location>
          </configuration>
        </rootmap>
        */
        TiXmlElement* rootElement = m_configDocument->RootElement();
        const char* root_value = rootElement->Value();
        RmAssert(0 == strcmp(root_value, "rootmap"), "Root node of any rootmap xml file must be \"rootmap\"");

        TiXmlNode* configNode = 0;
        //const char * m=0;

        // for each configuration item
        while (configNode = rootElement->IterateChildren("configuration", configNode))
        {
            // The configuration elements are name, type, owner, location
            //
            TiXmlElement* configElem = configNode->ToElement();
            if (configElem != 0)
            {
                std::string name = TXUtility::getChildElementText(configElem, "name");
                std::string type = TXUtility::getChildElementText(configElem, "type");
                std::string owner = TXUtility::getChildElementText(configElem, "owner");
                std::string location = TXUtility::getChildElementText(configElem, "location");
                if ((!name.empty()) && (!type.empty()) && (!owner.empty()) && (!location.empty()))
                {
                    if (type == "configuration")
                    {
                        addConfigurationData(owner, name, location);
                    }
                    else if (type == "construction")
                    {
                        addConstructionData(owner, name, location);
                    }
                    else if (type == "initialisation")
                    {
                        addInitialisationData(owner, name, location);
                    }
                    else if (type == "visualisation")
                    {
                        addVisualisationData(owner, name, location);
                    }
                }
                else
                {
                    // LOG badly formed configuration section
                    RootMapLogError(HERE << "Bad configuration in file " << configFile << ", <Row,Col>=<" << configNode->Column() << "," << configNode->Row() << ">");
                }
            }
            else
            {
                // LOG could not convert configuration node to element
                RootMapLogError(HERE << "Could not convert configuration node to element in file " << configFile << ", <Row,Col>=<" << configNode->Column() << "," << configNode->Row() << ">");
            }
        }

        m_isInitialised = true;

        logAllData();
    }
} /* namespace rootmap */
