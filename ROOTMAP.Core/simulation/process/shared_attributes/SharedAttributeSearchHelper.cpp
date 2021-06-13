/////////////////////////////////////////////////////////////////////////////
// Name:        SharedAttributeSearchHelper.cpp
// Purpose:     Implementation of the SharedAttributeSearchHelper class
// Created:     10/01/2008
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/shared_attributes/SharedAttributeSearchHelper.h"
#include "simulation/process/shared_attributes/SharedAttributeManager.h"
#include "simulation/process/shared_attributes/SharedAttributeOwner.h"
#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"
#include "simulation/process/shared_attributes/SharedAttribute.h"
#include "simulation/process/common/CharacteristicDescriptor.h"
#include "core/common/RmAssert.h"
#include "core/utility/StringParserUtility.h"
#include "core/utility/Utility.h"
#include "core/common/Exceptions.h"
#include <cstring>

namespace rootmap
{
    RootMapLoggerDefinition(SharedAttributeSearchHelper);

    CharacteristicIndex SharedAttributeSearchHelper::SearchForClusterIndex(const char* attribute_name)
    {
        LOG_DEBUG << "SearchForClusterIndex('" << attribute_name << ")";

        CharacteristicIndex index =
            mySharedAttributeManager->SearchForClusterIndex(attribute_name);

        if (InvalidCharacteristicIndex == index)
        {
            LOG_WARN << "Failed to find ClusterIndex for attribute " << attribute_name;
        }

        return index;
    }


    CharacteristicIndex SharedAttributeSearchHelper::SearchForClusterIndex(const char* attribute_name, const char* cluster_name, const std::vector<std::string>& variation_names)
    {
        LOG_DEBUG << "SearchForClusterIndex {attribute:" << attribute_name
            << "} {cluster:" << cluster_name
            << "} {variations:" << variation_names.size()
            << "}";

        CharacteristicIndex index =
            mySharedAttributeManager->SearchForClusterIndex(attribute_name, cluster_name, variation_names);

        if (InvalidCharacteristicIndex == index)
        {
            LOG_WARN << "Failed to find ClusterIndex for attribute " << attribute_name << " in cluster " << cluster_name;
        }

        return index;
    }

    CharacteristicIndex SharedAttributeSearchHelper::SearchForClusterIndex(const char* attribute_name, const char* cluster_name, const char* variation_names)
    {
        return SearchForClusterIndex(attribute_name, cluster_name, variation_names, false);
    }

    CharacteristicIndex SharedAttributeSearchHelper::SearchForClusterIndex(const char* attribute_name, const char* cluster_name, const char* variation_names, const bool& throwExceptions)
    {
        LOG_DEBUG << "SearchForClusterIndex {attribute:" << attribute_name
            << "} {cluster:" << cluster_name
            << "} {variations:" << ((__nullptr == variation_names) ? ("NULL") : variation_names)
            << "}";

        CharacteristicIndex index = InvalidCharacteristicIndex;

        // If no variation names,
        if ((__nullptr == variation_names) || (0x00 == variation_names[0]))
        {
            // use the basic attribute-in-cluster method
            index = mySharedAttributeManager->SearchForClusterIndex(attribute_name, cluster_name);
        }
        // otherwise if there is a list of names,
        else if (__nullptr != strchr(variation_names, ','))
        {
            // parse and convert into vector
            std::vector<std::string> v_names;
            StringParserUtility::ParseCommaSeparatedStrings(variation_names, v_names);
            index = SearchForClusterIndex(attribute_name, cluster_name, v_names);
        }
        else
        {
            //
            // use the attribute-in-cluster (specific permutation) method
            index = mySharedAttributeManager->SearchForClusterIndex(attribute_name, cluster_name);
        }

        if (InvalidCharacteristicIndex == index)
        {
            if (throwExceptions)
            {
                throw RmException("Failed to find ClusterIndex");
            }
            else
            {
                LOG_WARN << "Failed to find ClusterIndex for attribute " << attribute_name << " in cluster " << cluster_name;
            }
        }
        return index;
    }


    SharedAttribute* SharedAttributeSearchHelper::SearchForAttribute(const char* attribute_name)
    {
        LOG_DEBUG << "SearchForAttribute('" << attribute_name << ")";

        SharedAttribute* attr = __nullptr;

        if (__nullptr != mySharedAttributeOwner)
        {
            attr = mySharedAttributeOwner->SearchForAttribute(attribute_name);
        }

        if (__nullptr == attr)
        {
            attr = mySharedAttributeManager->SearchForAttribute(attribute_name, mySharedAttributeOwner);
        }

        if (__nullptr == attr)
        {
            LOG_WARN << "Failed to find SharedAttribute " << attribute_name;
        }

        return attr;
    }


    SharedAttribute* SharedAttributeSearchHelper::SearchForAttribute(const char* attribute_name, const char* v_name)
    {
        LOG_DEBUG << "SearchForAttribute(" << attribute_name << ", " << v_name << ")";

        if (__nullptr != strchr(v_name, ','))
        {
            std::vector<std::string> v_names;
            StringParserUtility::ParseCommaSeparatedStrings(v_name, v_names);
            return SearchForAttribute(attribute_name, v_names);
        }

        SharedAttribute* attr = __nullptr;
        if (__nullptr != mySharedAttributeOwner)
        {
            attr = (mySharedAttributeOwner->SearchForAttribute(attribute_name, v_name));
        }

        if (__nullptr == attr)
        {
            // Not found in this owner, try manager (will try other owners)
            attr = (mySharedAttributeManager->SearchForAttribute(attribute_name, v_name, mySharedAttributeOwner));
        }

        if (__nullptr == attr)
        {
            LOG_WARN << "Failed to find SharedAttribute " << attribute_name << ", v_name " << v_name;
        }

        return attr;
    }


    SharedAttribute* SharedAttributeSearchHelper::SearchForAttribute(const char* attribute_name, const char* v_name, const char* v_string)
    {
        LOG_DEBUG << "SearchForAttribute(" << attribute_name << ", " << v_name << ", " << v_string << ")";

        if (__nullptr != strchr(v_name, ','))
        {
            std::vector<std::string> v_names;
            StringParserUtility::ParseCommaSeparatedStrings(v_name, v_names);
            return SearchForAttribute(attribute_name, v_names, v_string);
        }

        SharedAttribute* attr = __nullptr;
        if (__nullptr != mySharedAttributeOwner)
        {
            attr = (mySharedAttributeOwner->SearchForAttribute(attribute_name, v_name, v_string));
        }

        if (__nullptr == attr)
        {
            // Not found in this owner, try manager (will try other owners)
            attr = (mySharedAttributeManager->SearchForAttribute(attribute_name, v_name, v_string, mySharedAttributeOwner));
        }

        if (__nullptr == attr)
        {
            LOG_WARN << "Failed to find SharedAttribute " << attribute_name << ", v_name " << v_name << ", v_string " << v_string;
        }

        return attr;
    }


    SharedAttribute* SharedAttributeSearchHelper::SearchForAttribute(const char* attribute_name, const std::vector<std::string>& v_names)
    {
        LOG_DEBUG << "SearchForAttribute(" << attribute_name << ", " << v_names.size() << " variations)";

        SharedAttribute* attr = __nullptr;
        if (__nullptr != mySharedAttributeOwner)
        {
            attr = mySharedAttributeOwner->SearchForAttribute(attribute_name, v_names);
        }

        if (__nullptr == attr)
        {
            // Not found in this owner, try manager (will try other owners)
            attr = mySharedAttributeManager->SearchForAttribute(attribute_name, v_names, mySharedAttributeOwner);
        }

        if (__nullptr == attr)
        {
            LOG_WARN << "Failed to find SharedAttribute " << attribute_name << ", multiple v_names";
        }

        return attr;
    }


    SharedAttribute* SharedAttributeSearchHelper::SearchForAttribute(const char* attribute_name, const std::vector<std::string>& v_names, const char* v_string)
    {
        LOG_DEBUG << "SearchForAttribute {attribute:" << attribute_name
            << "} {variations:" << v_names.size()
            << "} {permutation:" << v_string << "}";

        SharedAttribute* attr = __nullptr;

        if (__nullptr != mySharedAttributeOwner)
        {
            attr = (mySharedAttributeOwner->SearchForAttribute(attribute_name, v_names, v_string));
        }

        if (__nullptr == attr)
        {
            // Not found in this owner, try manager (will try other owners)
            attr = mySharedAttributeManager->SearchForAttribute(attribute_name, v_names, v_string, mySharedAttributeOwner);
        }

        if (__nullptr == attr)
        {
            LOG_WARN << "Failed to find SharedAttribute " << attribute_name << ", multiple v_names, v_string " << v_string;
        }

        return attr;
    }

    SharedAttributeSearchHelper::SharedAttributeSearchHelper(SharedAttributeManager* sam, SharedAttributeOwner* owner)
        : mySharedAttributeManager(sam)
        , mySharedAttributeOwner(owner)
    {
        RootMapLoggerInitialisation("rootmap.SharedAttributeSearchHelper");
    }
} /* namespace rootmap */
