/////////////////////////////////////////////////////////////////////////////
// Name:        DummyConfigurationDataStore.h
// Purpose:     Declaration of the DummyConfigurationDataStore class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#ifndef DummyConfigurationDataStore_H
#define DummyConfigurationDataStore_H

#include <string>
#include <map>
#include <list>

#include "simulation/data_access/common/NameLocationData.h"
#include "simulation/data_access/common/ConfigurationDataStore.h"

namespace rootmap
{
    /**
     * The four elements of configuration
     */
    class DummyConfigurationDataStore : public ConfigurationDataStore
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


        DummyConfigurationDataStore();
        virtual ~DummyConfigurationDataStore();

    private:
        ///
        /// flag to indicate state of initialisation
        bool myIsInitialised;
    }; // class DummyConfigurationDataStore
} /* namespace rootmap */

#endif // #ifndef DummyConfigurationDataStore_H
