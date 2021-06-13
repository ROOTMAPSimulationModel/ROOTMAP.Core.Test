/////////////////////////////////////////////////////////////////////////////
// Name:        SharedAttributeManager.cpp
// Purpose:     Implementation of the SharedAttributeManager class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
/*
    SharedAttributeManager.cpp

    Author:            Robert van Hugten
    Description:    <describe the SharedAttributeManager class here>
*/

#include "simulation/process/shared_attributes/SharedAttributeManager.h"
#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"
#include "simulation/process/shared_attributes/SharedAttributeOwner.h"
#include "simulation/process/shared_attributes/SharedAttributeSupplier.h"
#include "simulation/process/shared_attributes/SharedAttributeVariation.h"
#include "simulation/process/shared_attributes/ScoreboardSharedAttribute.h"
#include "simulation/process/shared_attributes/SharedAttributeIteratorAssistant.h"

#include "simulation/common/SimulationEngine.h"
#include "simulation/data_access/common/DataAccessManager.h"
#include "simulation/data_access/interface/CharacteristicDAI.h"

#include "core/common/ExtraStuff.h"
#include "core/common/RmAssert.h"
#include "core/utility/StringParserUtility.h"


namespace rootmap
{
    RootMapLoggerDefinition(SharedAttributeManager);


    // ////////////////////////////////////////////////////////////////////
    void SharedAttributeManager::Initialise()
    {
        // No-op. See other Initialise method
    }


    // ////////////////////////////////////////////////////////////////////
    SharedAttribute* SharedAttributeManager::MakeAttribute
    (CharacteristicDescriptor* /* cd */,
        SharedAttributeOwner* /* owner */,
        SharedAttributeSupplier* /* supplier */,
        long /* variation */
    )
    {
        SharedAttribute* attribute = 0;
        return attribute;
    }


    // ////////////////////////////////////////////////////////////////////
    void SharedAttributeManager::MakeAttributes
    (CharacteristicDescriptor* cd,
        const std::string& owner_name,
        const std::string& supplier_name,
        const std::vector<std::string>& variation_names,
        const std::string& defaults_name,
        const std::vector<double>& defaults_values
    )
    {
        // Sanity checking of defaults.
        // a) are there any defaults
        // b) does the name match an existing variation
        // c) further down, the name will also be checked against the list of
        //    variations used. If this isn't one, it won't make sense as a default
        SharedAttributeVariation* default_variation = __nullptr;
        bool default_variation_ok = true;
        if (!defaults_name.empty())
        {
            default_variation = SharedAttributeRegistrar::FindVariation(defaults_name);
            if (__nullptr != default_variation)
            {
                // if it is non-null, need to verify it is one of this attribute's vars
                default_variation_ok = false;
            }
            else
            {
                LOG_ALERT << "No matching variation class found for defaults {Name:" << defaults_name.c_str() << "}";
            }
        }


        //
        // build the list of variations from the given names    
        std::vector<std::string>::const_iterator varname_iter = variation_names.begin();
        SharedAttributeVariationList savs;
        while (varname_iter != variation_names.end())
        {
            SharedAttributeVariation* found_sav = SharedAttributeRegistrar::FindVariation((*varname_iter));
            if (__nullptr != found_sav)
            {
                //TODO: implement SharedAttributeVariation::Clone()
                //(Issue 84)
                SharedAttributeVariation* used_sav = found_sav; // ->Clone();

                savs.push_back(used_sav);

                // verify if this variation is the default
                if (defaults_name == (*varname_iter))
                {
                    default_variation_ok = true;
                }
            }
            else
            {
                //TODO: what about this error, ie. if no variation class with the
                //      given name is found ?  It isn't major, so just logging a
                //      warning should be enough
                //UPDATE 20081103: config errors are now being logged at Alert level
                LOG_ALERT << "No matching variation class found {Name:" << (*varname_iter).c_str() << "}";
            }
            ++varname_iter;
        }

        if (!default_variation_ok)
        {
            // The variation was specified but not one of those used by this
            // attribute. Bad Config but not Fatal.
            //TODO: beware - need to delete SAV if Clone()d
            default_variation = __nullptr;
            LOG_ALERT << LOG_LINE << "Default variation not used by this attribute {AttributeName:" << cd->Name << ", DefaultVariationName:" << defaults_name << "}";
        }

        SharedAttributeOwner* owner = SharedAttributeRegistrar::FindOwner(owner_name);
        RmAssert(0 != owner, "SharedAttributeOwner not found with name!");
        myOwners.insert(owner);

        SharedAttributeSupplier* supplier = SharedAttributeRegistrar::FindSupplier(supplier_name);

        //
        // call upon the owner to do the actual SharedAttribute creating.
        owner->MakeAttributes(cd, supplier, savs, default_variation, defaults_values);
    }


    // ////////////////////////////////////////////////////////////////////
    void SharedAttributeManager::MakeAttributes
    (CharacteristicDAI& data,
        const std::string& owner_name,
        const std::string& supplier_name,
        const std::vector<std::string>& variation_names,
        const std::string& defaults_name,
        const std::vector<double>& defaults_values
    )
    {
        CharacteristicDescriptor* cd = new CharacteristicDescriptor(data);

        //CharacteristicDescriptor * cd = new CharacteristicDescriptor;
        //cd->SetName(data.getName());
        //cd->SetUnits(data.getUnits());
        //cd->SetScoreboardStratum(data.getStratum());
        //cd->SetMinimum(data.getMinimum());
        //cd->SetMaximum(data.getMaximum());
        //cd->SetDefault(data.getDefault());
        //cd->SetVisible(data.isVisible());
        //cd->SetEdittable(data.isEdittable());
        //cd->SetSavable(data.isSavable());
        //cd->SetSpecialPerBoxInfo(data.hasSpecialPerBoxInfo());

        MakeAttributes(cd, owner_name, supplier_name, variation_names, defaults_name, defaults_values);
    }


    // ////////////////////////////////////////////////////////////////////
    void SharedAttributeManager::SetSharedAttributeInitialValue
    (const std::string& owner_name,
        const std::string& attr_name,
        double value
    )
    {
        SharedAttributeOwner* owner = SharedAttributeRegistrar::FindOwner(owner_name);

        if (owner != 0)
        {
            owner->SetSharedAttributeInitialValue(attr_name, value);
        }
        else
        {
            LOG_WARN << "Couldn't find Attribute to assign Initial Value {Name:" << attr_name.c_str() << "}";
        }
    }


    // ////////////////////////////////////////////////////////////////////
    // Bridge method. Calls DoVariationChange on all this object's owners.
    // Also tells myAttributeIteratorAssistant about the change.  
    void SharedAttributeManager::VariationChange
    (const char* variation_name,
        long int change
    )
    {
        SharedAttributeOwnerSet::const_iterator owner_iterator = myOwners.begin();
        while (owner_iterator != myOwners.end())
        {
            (*owner_iterator)->DoVariationChange(variation_name, change);
            ++owner_iterator;
        }
    }


    SharedAttribute* SharedAttributeManager::SearchForAttribute
    (const std::string& attribute_name,
        SharedAttributeOwner* owner
    )
    {
        LOG_DEBUG << "SearchForAttribute {attribute:" << attribute_name.c_str()
            << "} {owner:" << ((__nullptr != owner) ? (owner->GetSharedAttributeOwnerName()) : "NULL") << "}";

        SharedAttributeOwnerSet::const_iterator owner_iterator = myOwners.begin();
        SharedAttribute* s = 0;

        //
        // Search the given owner first
        //s = owner->DoSearchForAttribute(attribute_name);

        //
        // Then all the others
        while ((owner_iterator != myOwners.end()) && (s == 0))
        {
            if ((*owner_iterator) != owner)
            {
                s = (*owner_iterator)->DoSearchForAttribute(attribute_name);
            }
            ++owner_iterator;
        }

        return s;
    }


    SharedAttribute* SharedAttributeManager::SearchForAttribute
    (const std::string& attribute_name,
        const std::string& variation_name,
        SharedAttributeOwner* owner
    )
    {
        {
            LOG_DEBUG << "SearchForAttribute {attribute:" << attribute_name.c_str()
                << "} {varname:" << variation_name.c_str()
                << "} {owner:" << ((__nullptr != owner) ? (owner->GetSharedAttributeOwnerName()) : "NULL") << "}";
        }

        SharedAttributeOwnerSet::const_iterator owner_iterator = myOwners.begin();
        SharedAttribute* s = 0;

        //
        // Search the given owner first
        //s = owner->DoSearchForAttribute(attribute_name,variation_name,variation_string);

        //
        // Then all the others
        while ((owner_iterator != myOwners.end()) && (s == 0))
        {
            if ((*owner_iterator) != owner)
            {
                s = (*owner_iterator)->DoSearchForAttribute(attribute_name, variation_name);
            }
            ++owner_iterator;
        }

        return s;
    }


    SharedAttribute* SharedAttributeManager::SearchForAttribute
    (const std::string& attribute_name,
        const std::string& variation_name,
        const std::string& variation_string,
        SharedAttributeOwner* owner
    )
    {
        {
            LOG_DEBUG << "SearchForAttribute {attribute:" << attribute_name.c_str()
                << "} {varname:" << variation_name.c_str()
                << "} {varstring:" << variation_string.c_str()
                << "} {owner:" << ((__nullptr != owner) ? (owner->GetSharedAttributeOwnerName()) : "NULL") << "}";
        }

        SharedAttributeOwnerSet::const_iterator owner_iterator = myOwners.begin();
        SharedAttribute* s = 0;

        //
        // Search the given owner first
        //s = owner->DoSearchForAttribute(attribute_name,variation_name,variation_string);

        //
        // Then all the others
        while ((owner_iterator != myOwners.end()) && (s == 0))
        {
            if ((*owner_iterator) != owner)
            {
                s = (*owner_iterator)->DoSearchForAttribute(attribute_name, variation_name, variation_string);
            }
            ++owner_iterator;
        }

        return s;
    }


    SharedAttribute* SharedAttributeManager::SearchForAttribute
    (const std::string& attribute_name,
        const std::vector<std::string>& variation_names,
        SharedAttributeOwner* owner
    )
    {
        {
            std::string variation_string;
            StringParserUtility::UnParseCommaSeparatedStrings(variation_names, variation_string);
            LOG_DEBUG << "SearchForAttribute {attribute:" << attribute_name
                << "}, {variations:" << variation_string
                << "}, {owner:" << ((__nullptr != owner) ? (owner->GetSharedAttributeOwnerName()) : "NULL") << "}";
        }

        SharedAttributeOwnerSet::const_iterator owner_iterator = myOwners.begin();
        SharedAttribute* s = 0;

        while ((owner_iterator != myOwners.end()) && (s == 0))
        {
            if ((*owner_iterator) != owner)
            {
                s = (*owner_iterator)->DoSearchForAttribute(attribute_name, variation_names);
            }
            ++owner_iterator;
        }

        return s;
    }

    SharedAttribute* SharedAttributeManager::SearchForAttribute
    (const std::string& attribute_name,
        const std::vector<std::string>& variation_names,
        const std::string& variation_string,
        SharedAttributeOwner* owner
    )
    {
        {
            std::string vname_string;
            StringParserUtility::UnParseCommaSeparatedStrings(variation_names, vname_string);
            LOG_DEBUG << "SearchForAttribute {attribute:" << attribute_name
                << "}, {variations:" << vname_string
                << "}, {permutation:" << variation_string
                << "}, {owner:" << ((__nullptr != owner) ? (owner->GetSharedAttributeOwnerName()) : "NULL") << "}";
        }

        SharedAttributeOwnerSet::const_iterator owner_iterator = myOwners.begin();
        SharedAttribute* s = 0;

        while ((owner_iterator != myOwners.end()) && (s == 0))
        {
            if ((*owner_iterator) != owner)
            {
                s = (*owner_iterator)->DoSearchForAttribute(attribute_name, variation_names, variation_string);
            }
            ++owner_iterator;
        }

        return s;
    }


    // ////////////////////////////////////////////////////////////////////
    CharacteristicIndex SharedAttributeManager::SearchForClusterIndex
    (const char* attribute_name)
    {
        return myAttributeIteratorAssistant->SearchForClusterIndex(attribute_name);
    }


    CharacteristicIndex SharedAttributeManager::SearchForClusterIndex
    (const char* attribute_name,
        const char* cluster_name
    )
    {
        SharedAttribute* attr = SearchForAttribute(attribute_name, cluster_name, __nullptr);

        if (__nullptr != attr)
        {
            std::string full_attr_name = attr->GetCharacteristicDescriptor()->Name;

            return myAttributeIteratorAssistant->SearchForClusterIndex(full_attr_name, cluster_name);
        }

        return InvalidCharacteristicIndex;
    }


    CharacteristicIndex SharedAttributeManager::SearchForClusterIndex
    (const char* attribute_name,
        const char* cluster_name,
        const std::vector<std::string>& variation_names)
    {
        SharedAttribute* attr = SearchForAttribute(attribute_name, variation_names, __nullptr);

        if (__nullptr != attr)
        {
            std::string full_attr_name = attr->GetCharacteristicDescriptor()->Name;

            return myAttributeIteratorAssistant->SearchForClusterIndex(full_attr_name, cluster_name);
        }

        return InvalidCharacteristicIndex;
    }


    // ////////////////////////////////////////////////////////////////////
    const SharedAttributeCluster& SharedAttributeManager::GetCluster
    (const char* variation_name
    ) const
    {
        return myAttributeIteratorAssistant->GetCluster(variation_name);
    }


    // ////////////////////////////////////////////////////////////////////
    void SharedAttributeManager::ClusteriseSharedAttribute
    (SharedAttribute* sa,
        const VariationNameArray& varKeyList,
        const SharedAttributeVariationList& varList
    )
    {
        RmAssert(varKeyList.size() == varList.size(), "Variation KeyList and List are different sizes");

        //    if (varKeyList.size() != varList.size())
        //    {
        //        LOG_ERROR << ;
        //    }


        size_t v_size = varKeyList.size();

        if (v_size > 0)
        {
            for (size_t i = 0; i < v_size; ++i)
            {
                myAttributeIteratorAssistant->ClusterSharedAttribute
                (sa,
                    ((varList[i])->GetName()).c_str(),
                    (varKeyList[i]).c_str());
            }
        }
        else
        {
            myAttributeIteratorAssistant->ClusterSharedAttribute(sa);
        }
    }


    // ////////////////////////////////////////////////////////////////////
    SharedAttributeManager::SharedAttributeManager
    (SimulationEngine* engine,
        DataAccessManager& /* dam */,
        const char* name)
        : myName(name)
        , myEngine(engine)
        , myAttributeIteratorAssistant(0)
    {
        RootMapLoggerInitialisation("rootmap.SharedAttributeManager");
        SharedAttributeRegistrar::RegisterManager(this);
        myAttributeIteratorAssistant = new SharedAttributeIteratorAssistant(this);
    }


    // ////////////////////////////////////////////////////////////////////
    SharedAttributeManager::~SharedAttributeManager()
    {
        delete myAttributeIteratorAssistant;
    }


    // ////////////////////////////////////////////////////////////////////
    const std::string& SharedAttributeManager::GetSharedAttributeManagerName()
    {
        return myName;
    }

    void SharedAttributeManager::LogSharedAttributeIteratorAssistantContents()
    {
        myAttributeIteratorAssistant->Log();
    }
} /* namespace rootmap */


