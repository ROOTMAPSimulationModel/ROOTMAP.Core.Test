/////////////////////////////////////////////////////////////////////////////
// Name:        DataAccessManager.cpp
// Purpose:     Implementation of the DataAccessManager class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/DataAccessManager.h"
#include "simulation/data_access/common/ConfigurationDataStore.h"


namespace rootmap
{
    DataAccessManager::DataAccessManager()
        : m_configurationDataStore(0)
    {
    }

    DataAccessManager::~DataAccessManager()
    {
    }

    void DataAccessManager::cleanup()
    {
        getConfigurationDataStore().cleanup();
    }
} /* namespace rootmap */

