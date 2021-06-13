/*
    SharedAttributeRegistrar.cpp

    Author:            Robert van Hugten
    Description:    <describe the SharedAttributeRegistrar class here>
*/

#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"
#include "simulation/process/shared_attributes/SharedAttributeVariation.h"
#include "simulation/process/shared_attributes/SharedAttributeOwner.h"
#include "simulation/process/shared_attributes/SharedAttributeSupplier.h"
#include "simulation/process/shared_attributes/SharedAttributeManager.h"


namespace rootmap
{
    SharedAttributeVariation* SharedAttributeRegistrar::FindVariation(const std::string& name)
    {
        SharedAttributeVariation* found_variation = 0;

        SharedAttributeVariationMap::iterator found_iter = myRegisteredVariations.find(name);

        if (found_iter != myRegisteredVariations.end())
        {
            found_variation = (*found_iter).second;
        }

        return found_variation;
    }


    SharedAttributeOwner* SharedAttributeRegistrar::FindOwner(const std::string& name)
    {
        SharedAttributeOwner* found_owner = 0;

        SharedAttributeOwnerMap::iterator found_iter = myRegisteredOwners.find(name);

        if (found_iter != myRegisteredOwners.end())
        {
            found_owner = (*found_iter).second;
        }

        return found_owner;
    }


    SharedAttributeSupplier* SharedAttributeRegistrar::FindSupplier(const std::string& name)
    {
        SharedAttributeSupplier* found_supplier = 0;

        SharedAttributeSupplierMap::iterator found_iter = myRegisteredSuppliers.find(name);

        if (found_iter != myRegisteredSuppliers.end())
        {
            found_supplier = (*found_iter).second;
        }

        return found_supplier;
    }


    SharedAttributeManager* SharedAttributeRegistrar::FindManager(const std::string& name)
    {
        SharedAttributeManager* found_manager = 0;

        SharedAttributeManagerMap::iterator found_iter = myRegisteredManagers.find(name);

        if (found_iter != myRegisteredManagers.end())
        {
            found_manager = (*found_iter).second;
        }

        return found_manager;
    }

    SharedAttributeVariation* SharedAttributeRegistrar::FindVariationValue(const std::string& value)
    {
        SharedAttributeVariation* found_variation = 0;

        SharedAttributeVariationValueMap::iterator found_iter = myVariationValues.find(value);

        if (found_iter != myVariationValues.end())
        {
            const std::string& varname = found_iter->second;

            return FindVariation(varname);
        }

        return found_variation;
    }


    void SharedAttributeRegistrar::RegisterVariation(SharedAttributeVariation* sav)
    {
        if (sav != 0)
        {
            std::string sav_name = sav->GetName();
            SharedAttributeVariationMap::value_type value(sav_name, sav);
            myRegisteredVariations.insert(value);
        }
    }


    void SharedAttributeRegistrar::RegisterOwner(SharedAttributeOwner* sao)
    {
        if (sao != 0)
        {
            std::string sao_name = sao->GetSharedAttributeOwnerName();
            SharedAttributeOwnerMap::value_type value(sao_name, sao);
            myRegisteredOwners.insert(value);
        }
    }


    void SharedAttributeRegistrar::RegisterSupplier(SharedAttributeSupplier* sas)
    {
        if (sas != 0)
        {
            std::string sas_name = sas->GetSharedAttributeSupplierName();
            SharedAttributeSupplierMap::value_type value(sas_name, sas);
            myRegisteredSuppliers.insert(value);
        }
    }


    void SharedAttributeRegistrar::RegisterManager(SharedAttributeManager* sam)
    {
        if (sam != 0)
        {
            std::string sam_name = sam->GetSharedAttributeManagerName();
            SharedAttributeManagerMap::value_type value(sam_name, sam);
            myRegisteredManagers.insert(value);
        }
    }

    void SharedAttributeRegistrar::RegisterVariationValue(const std::string& value, SharedAttributeVariation* sav)
    {
        if ((sav != 0) && (myVariationValues.end() == myVariationValues.find(value)))
        {
            const std::string& savname = sav->GetName();
            SharedAttributeVariationValueMap::value_type vvalue(value, savname);
            myVariationValues.insert(vvalue);
        }
    }


    void SharedAttributeRegistrar::DeregisterAll()
    {
        //
        // Variations
        for (SharedAttributeVariationMap::const_iterator var_i = myRegisteredVariations.begin();
            var_i != myRegisteredVariations.end();
            ++var_i
            )
        {
            SharedAttributeVariation* var = (*var_i).second;
            delete var;
        }
        myRegisteredVariations.clear();

        //
        // Owners (which delete their attributes and families)
        for (SharedAttributeOwnerMap::const_iterator own_i = myRegisteredOwners.begin();
            own_i != myRegisteredOwners.end();
            ++own_i
            )
        {
            SharedAttributeOwner* own = (*own_i).second;
            delete own;
        }
        myRegisteredOwners.clear();

        //
        // Suppliers
        for (SharedAttributeSupplierMap::const_iterator sup_i = myRegisteredSuppliers.begin();
            sup_i != myRegisteredSuppliers.end();
            ++sup_i
            )
        {
            SharedAttributeSupplier* sup = (*sup_i).second;
            delete sup;
        }
        myRegisteredSuppliers.clear();

        //
        // Managers
        for (SharedAttributeManagerMap::const_iterator man_i = myRegisteredManagers.begin();
            man_i != myRegisteredManagers.end();
            ++man_i
            )
        {
            SharedAttributeManager* man = (*man_i).second;
            delete man;
        }
        myRegisteredManagers.clear();
    }


    const SharedAttributeOwnerMap& SharedAttributeRegistrar::GetRegisteredOwners()
    {
        return myRegisteredOwners;
    }

    //SharedAttributeRegistrar::SharedAttributeRegistrar()
    //{
    //}
    //
    //
    //SharedAttributeRegistrar::~SharedAttributeRegistrar()
    //{
    //}
    //
    SharedAttributeVariationMap SharedAttributeRegistrar::myRegisteredVariations;
    SharedAttributeOwnerMap SharedAttributeRegistrar::myRegisteredOwners;
    SharedAttributeSupplierMap SharedAttributeRegistrar::myRegisteredSuppliers;
    SharedAttributeManagerMap SharedAttributeRegistrar::myRegisteredManagers;
    SharedAttributeRegistrar::SharedAttributeVariationValueMap SharedAttributeRegistrar::myVariationValues;
} /* namespace rootmap */


