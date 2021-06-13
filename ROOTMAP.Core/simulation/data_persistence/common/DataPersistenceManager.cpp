/////////////////////////////////////////////////////////////////////////////
// Name:        DataPersistenceManager.cpp
// Purpose:     Implementation of the DataPersistenceManager class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_persistence/common/DataPersistenceManager.h"
#include "simulation/data_persistence/common/PersistedDataDirectory.h"


namespace rootmap
{
    DataPersistenceManager::DataPersistenceManager()
        : m_configurationDataStore(0)
    {
    }

    DataPersistenceManager::~DataPersistenceManager()
    {
    }

    void DataPersistenceManager::cleanup()
    {
        getPersistedDataDirectory().cleanup();
    }
} /* namespace rootmap */

