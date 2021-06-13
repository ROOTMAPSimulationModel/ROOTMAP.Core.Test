/*
    SharedAttributeOwner.cpp

    Author:            Robert van Hugten
    Description:    <describe the SharedAttributeOwner class here>
*/

#include "simulation/process/shared_attributes/SharedAttributeOwner.h"
#include "simulation/process/shared_attributes/SharedAttribute.h"
#include "simulation/process/shared_attributes/SharedAttributeFamily.h"
#include "simulation/process/shared_attributes/SharedAttributeVariation.h"
#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"
#include "simulation/process/shared_attributes/SharedAttributeManager.h"

#include "simulation/process/common/CharacteristicDescriptor.h"

#include "core/common/RmAssert.h"
#include "core/common/ExtraStuff.h"
#include "core/utility/StringParserUtility.h"


namespace rootmap
{
#ifdef LOG_DEBUG
    //#define CS_DEBUG_SHAREDATTRIBUTEOWNER_DOFINDATTRIBUTE_4PARAMETER
    bool DEBUG_SHAREDATTRIBUTEOWNER_DOFINDATTRIBUTE_4PARAMETER = true;
#endif


    RootMapLoggerDefinition(SharedAttributeOwner);

    void SharedAttributeOwner::MakeAttributes
    (CharacteristicDescriptor* cd,
        SharedAttributeSupplier* supplier,
        const SharedAttributeVariationList& variations,
        SharedAttributeVariation* default_variation,
        const std::vector<double>& defaults_values
    )
    {
        SharedAttributeFamily* saf = new SharedAttributeFamily(cd, supplier, variations, default_variation, defaults_values);

        myAttributeFamilies.push_back(saf);

        //
        // If all of the variations are fixed, we must make all the SharedAttributes
        // now.  In fact, it ought also be done if any of the variable-number
        // variations have anything to vary.
        saf->MakeAllAttributes(this);
    }


    void SharedAttributeOwner::VariationChange
    (const char* variation_name,
        long int change
    )
    {
        //HACK
        SharedAttributeManager* sam = SharedAttributeRegistrar::FindManager("TheOnlyOne");
        RmAssert(0 != sam, "SharedAttributeManager not yet initialised!!");
        sam->VariationChange(variation_name, change);
    }


    void SharedAttributeOwner::DoVariationChange
    (const char* variation_name,
        long int change
    )
    {
        LOG_DEBUG << LOG_LINE << GetSharedAttributeOwnerName().c_str() << " DoVariationChange(" << variation_name << ", " << change << ")";

        std::string vname = variation_name;
        SharedAttributeFamilyList::iterator saf_iter = myAttributeFamilies.begin();
        // long int cindex = change; // MSA 09.11.02 Unused    

        while (saf_iter != myAttributeFamilies.end())
        {
            // 
            // the family from its iterator
            SharedAttributeFamily* saf = *saf_iter;

            //
            // See if this variation is in the family
            SharedAttributeVariation* sav = saf->SearchForVariation(vname);

            //
            // 'cause if it is, we need to make an attribute for each combination
            // of the remaining variations.
            // 
            if (sav != 0)
            {
                saf->MakeVariationAttributes(sav, this);
            }

            ++saf_iter;
        }
    }


    void SharedAttributeOwner::RegisterSharedAttribute
    (SharedAttribute* sa,
        SharedAttributeSupplier* /* supplier */,
        const VariationNameArray& savKeyList,
        const SharedAttributeVariationList& savList
    )
    {
        myAttributes.push_back(sa);

        //HACK
        SharedAttributeRegistrar::FindManager("TheOnlyOne")->ClusteriseSharedAttribute(sa, savKeyList, savList);
    }

    SharedAttribute* SharedAttributeOwner::SearchForAttribute
    (const std::string& attribute_name
    ) const
    {
        return (DoSearchForAttribute(attribute_name));
    }

    SharedAttribute* SharedAttributeOwner::SearchForAttribute
    (const std::string& attribute_name,
        const std::string& variation_name
    ) const
    {
        return (DoSearchForAttribute(attribute_name, variation_name));
    }

    SharedAttribute* SharedAttributeOwner::SearchForAttribute
    (const std::string& attribute_name,
        const std::string& variation_name,
        const std::string& variation_string
    ) const
    {
        return (DoSearchForAttribute(attribute_name, variation_name, variation_string));
    }


    SharedAttribute* SharedAttributeOwner::SearchForAttribute
    (const std::string& attribute_name,
        const std::vector<std::string>& v_names
    ) const
    {
        return (DoSearchForAttribute(attribute_name, v_names));
    }

    SharedAttribute* SharedAttributeOwner::SearchForAttribute
    (const std::string& attribute_name,
        const std::vector<std::string>& v_names,
        const std::string& variation_string
    ) const
    {
        return (DoSearchForAttribute(attribute_name, v_names, variation_string));
    }


    // /////////////////////////////////////////////////
    // DoSearchForAttribute

    SharedAttribute* SharedAttributeOwner::DoSearchForAttribute
    (const std::string& attribute_name
    ) const
    {
        for (SharedAttributeList::const_iterator iter = myAttributes.begin();
            iter != myAttributes.end(); ++iter)
        {
            if (attribute_name == ((*iter)->GetCharacteristicDescriptor())->Name)
            {
                LOG_INFO << LOG_LINE << GetSharedAttributeOwnerName().c_str() << " found attribute '" << attribute_name.c_str() << "'";
                return (*iter);
            }
        }

        LOG_INFO << LOG_LINE << GetSharedAttributeOwnerName().c_str() << " did NOT find attribute '" << attribute_name.c_str() << "'";

        return __nullptr;
    }


    //
    // Working Notes:
    //  Iterates through our Attribute Familes.  If a Family is found whose
    //  CharacteristicDescriptor's name matches the attribute_name, a further
    //  search is done with that family:
    //
    //  If the family has a variation type that matches variation_name, then we
    //  OUGHT to be able to find an attribute named "attribute_name variation_name ..."
    //  in our list of already-made attributes.
    //  
    //  So MakeSearchString() makes
    //
    SharedAttribute* SharedAttributeOwner::DoSearchForAttribute
    (const std::string& attribute_name,
        const std::string& variation_name
    ) const
    {
        {
            LOG_DEBUG << LOG_LINE << GetSharedAttributeOwnerName()
                << "::DoSearchForAttribute(" << attribute_name
                << "," << variation_name
                //<< "," << variation_string
                << ")";
        }

        for (SharedAttributeFamilyList::const_iterator saf_iter = myAttributeFamilies.begin();
            saf_iter != myAttributeFamilies.end(); ++saf_iter)
        {
            SharedAttributeFamily* saf = (*saf_iter);
            if (attribute_name == saf->GetCharacteristicDescriptor()->Name)
            {
                if (saf->MatchesVariation(variation_name))
                {
                    std::string sa_name = saf->MakeSharedAttributeName0();
                    SharedAttribute* sa = DoSearchForAttribute(sa_name);
                    if (__nullptr != sa)
                    {
                        LOG_INFO << LOG_LINE << "Search success, found {attribute:" << attribute_name << "} in {owner:" << GetSharedAttributeOwnerName() << "}";
                        return sa;
                    }
                }
            }
        }

        LOG_DEBUG << LOG_LINE << "Search failure, did not find {attribute:" << attribute_name << "} in {owner:" << GetSharedAttributeOwnerName() << "}";
        return __nullptr;
    }


    SharedAttribute* SharedAttributeOwner::DoSearchForAttribute
    (const std::string& attribute_name,
        const std::string& variation_name,
        const std::string& variation_string
    ) const
    {
        if (true)
        {
            LOG_DEBUG << LOG_LINE << GetSharedAttributeOwnerName()
                << "::DoSearchForAttribute(" << attribute_name
                << "," << variation_name
                << "," << variation_string
                << ")";
        }

        for (SharedAttributeFamilyList::const_iterator saf_iter = myAttributeFamilies.begin();
            saf_iter != myAttributeFamilies.end(); ++saf_iter)
        {
            SharedAttributeFamily* saf = (*saf_iter);
            if (attribute_name == saf->GetCharacteristicDescriptor()->Name)
            {
                if (saf->MatchesVariation(variation_name))
                {
                    //std::string sa_name = saf->MakeSharedAttributeName0();
                    std::string sa_name("");
                    SharedAttributeVariation* variation = saf->SearchForVariation(variation_name);
                    saf->MakeSearchString(variation, variation_string, sa_name);
                    SharedAttribute* sa = DoSearchForAttribute(sa_name);

                    if (__nullptr != sa)
                    {
                        LOG_INFO << LOG_LINE << "Search success, found {attribute:" << attribute_name << "} in {owner:" << GetSharedAttributeOwnerName() << "}";
                        return sa;
                    }
                }
            }
        }

        LOG_DEBUG << LOG_LINE << "Search failure, did not find {attribute:" << attribute_name << "} in {owner:" << GetSharedAttributeOwnerName() << "}";
        return __nullptr;
    }

    SharedAttribute* SharedAttributeOwner::DoSearchForAttribute
    (const std::string& attribute_name,
        const std::vector<std::string>& v_names
    ) const
    {
        if (true)
        {
            std::string vnamestr;
            StringParserUtility::UnParseCommaSeparatedStrings(v_names, vnamestr, ':', true);
            LOG_DEBUG << LOG_LINE << GetSharedAttributeOwnerName()
                << "::DoSearchForAttribute(" << attribute_name
                << "," << vnamestr << ")";
        }

        for (SharedAttributeFamilyList::const_iterator saf_iter = myAttributeFamilies.begin();
            saf_iter != myAttributeFamilies.end(); ++saf_iter)
        {
            SharedAttributeFamily* saf = (*saf_iter);
            if (attribute_name == saf->GetCharacteristicDescriptor()->Name)
            {
                if (saf->MatchesAllVariations(v_names))
                {
                    // Funny (not haha) way of getting the first SA: make the name
                    // and search for it.
                    // TODO: there must be a better way of doing this
                    std::string sa_name = saf->MakeSharedAttributeName0();
                    SharedAttribute* sa = DoSearchForAttribute(sa_name);
                    if (__nullptr != sa)
                    {
                        LOG_INFO << LOG_LINE << "Search success, found {attribute:" << attribute_name << "} in {owner:" << GetSharedAttributeOwnerName() << "}";
                        return sa;
                    }
                }
            }
        }

        LOG_DEBUG << LOG_LINE << "Search failure, did not find {attribute:" << attribute_name << "} in {owner:" << GetSharedAttributeOwnerName() << "}";
        return __nullptr;
    }

    SharedAttribute* SharedAttributeOwner::DoSearchForAttribute
    (const std::string& attribute_name,
        const std::vector<std::string>& v_names,
        const std::string& variation_string
    ) const
    {
        if (true)
        {
            std::string vnamestr;
            StringParserUtility::UnParseCommaSeparatedStrings(v_names, vnamestr, ':', true);
            LOG_DEBUG << LOG_LINE << GetSharedAttributeOwnerName()
                << "::DoSearchForAttribute(" << attribute_name
                << "} {variations:" << v_names.size()
                << "} {permutation:" << variation_string << "}";
        }

        for (SharedAttributeFamilyList::const_iterator saf_iter = myAttributeFamilies.begin();
            saf_iter != myAttributeFamilies.end(); ++saf_iter)
        {
            SharedAttributeFamily* saf = (*saf_iter);
            if (attribute_name == saf->GetCharacteristicDescriptor()->Name)
            {
                if (saf->MatchesAllVariations(v_names))
                {
                    SharedAttributeVariation* variation = SharedAttributeRegistrar::FindVariationValue(variation_string);

                    std::string sa_name("");
                    saf->MakeSearchString(variation, variation_string, sa_name);
                    SharedAttribute* sa = DoSearchForAttribute(sa_name);

                    if (__nullptr != sa)
                    {
                        LOG_INFO << LOG_LINE << "Search success, found {attribute:" << attribute_name << "} in {owner:" << GetSharedAttributeOwnerName() << "}";
                        return sa;
                    }
                }
            }
        }

        LOG_DEBUG << LOG_LINE << "Search failure, did not find {attribute:" << attribute_name << "} in {owner:" << GetSharedAttributeOwnerName() << "}";
        return __nullptr;
    }

    void SharedAttributeOwner::SetSharedAttributeInitialValue
    (const std::string& attr_name,
        double value
    )
    {
        SharedAttribute* attr = SearchForAttribute(attr_name);

        if (attr != 0)
        {
            (attr->GetCharacteristicDescriptor())->SetDefault(value);
        }
    }


    SharedAttributeOwner::SharedAttributeOwner()
    {
        RootMapLoggerInitialisation("rootmap.SharedAttributeOwner");
    }


    SharedAttributeOwner::~SharedAttributeOwner()
    {
        RootMapLogTrace("~SharedAttributeOwner");
        for (SharedAttributeList::const_iterator attr_iter = myAttributes.begin();
            attr_iter != myAttributes.end();
            ++attr_iter
            )
        {
            delete (*attr_iter);
        }


        //DONE: delete all myAttributeFamilies
        for (SharedAttributeFamilyList::const_iterator fam_iter = myAttributeFamilies.begin();
            fam_iter != myAttributeFamilies.end();
            ++fam_iter
            )
        {
            delete (*fam_iter);
        }
    }
} /* namespace rootmap */


