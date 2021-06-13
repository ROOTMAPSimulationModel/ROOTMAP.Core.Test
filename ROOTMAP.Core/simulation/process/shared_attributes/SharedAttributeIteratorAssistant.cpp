#include "simulation/process/shared_attributes/SharedAttributeIteratorAssistant.h"
#include "simulation/process/shared_attributes/SharedAttribute.h"
#include "simulation/process/common/CharacteristicDescriptor.h"
#include "core/utility/Utility.h"

#ifdef LOG_DEBUG
#define CS_DEBUG_SHAREDATTRIBUTEITERATORASSISTANT_SEARCHFORINDEX
#endif // #ifdef LOG_DEBUG

namespace rootmap
{
    RootMapLoggerDefinition(SharedAttributeIteratorAssistant);

    const SharedAttributeCluster& SharedAttributeIteratorAssistant::GetCluster(const std::string& variation_name) const
    {
        SharedAttributeClusterBundle::const_iterator found_cluster = myAttributeClusterBundle.find(variation_name);
        if (found_cluster != myAttributeClusterBundle.end())
        {
            LOG_DEBUG << "Cluster found for '" << variation_name << "'";
            // const SharedAttributeCluster & the_cluster = (*found_cluster).second;
            return (*found_cluster).second;
        }

        LOG_WARN << LOG_LINE << "Cluster NOT found for '" << variation_name << "'";
        return sNullCluster;
    }

    CharacteristicIndex SharedAttributeIteratorAssistant::SearchForClusterIndex(const std::string& attribute_name) const
    {
        SharedAttributeClusterBundle::const_iterator found_cluster = myAttributeClusterBundle.find(sNoVariationClusterName);
        if (found_cluster != myAttributeClusterBundle.end())
        {
            const SharedAttributeCluster& the_cluster = (*found_cluster).second;
            if (the_cluster.size() > 0)
            {
                const SharedAttributeList& first_list = (*(the_cluster.begin())).second;
                SharedAttributeList::const_iterator list_iter = first_list.begin();
                CharacteristicIndex i = 0;
                while (list_iter != first_list.end())
                {
                    SharedAttribute* attribute = *list_iter;
                    std::string aname = attribute->GetCharacteristicDescriptor()->Name;
                    if (aname == attribute_name)
                    {
                        // Found it!
                        LOG_DEBUG << "Search for '" << attribute_name << "', Found '" << aname.c_str() << "' index " << i;
                        return i;
                    }

                    ++list_iter;
                    ++i;
                }
            }
        }

        LOG_WARN << LOG_LINE << "Did NOT find attribute:'" << attribute_name << "'";
        return InvalidCharacteristicIndex;
    }

    //
    //  Search for the index of an Attribute within the variation cluster
    //
    CharacteristicIndex SharedAttributeIteratorAssistant::SearchForClusterIndex(const std::string& attribute_name, const std::string& cluster_name) const
    {
        SharedAttributeClusterBundle::const_iterator found_cluster = myAttributeClusterBundle.find(cluster_name);
        if (found_cluster != myAttributeClusterBundle.end())
        {
            const SharedAttributeCluster& the_cluster = (*found_cluster).second;
            if (the_cluster.size() > 0)
            {
                const SharedAttributeList& first_list = (*(the_cluster.begin())).second;
                SharedAttributeList::const_iterator list_iter = first_list.begin();
                CharacteristicIndex cluster_index = 0;

                while (list_iter != first_list.end())
                {
                    SharedAttribute* attribute = *list_iter;
                    CharacteristicDescriptor* cd = attribute->GetCharacteristicDescriptor();

                    // Now that the attribute_name parameter must specify the full
                    // characteristic, we can use equality
                    //if (Utility::StringBeginsWith(cd->Name,attribute_name))
                    if (attribute_name == cd->Name)
                    {
                        // NOTE that we aren't looking for the ScoreboardIndex.
                        // We're looking for the index of the attribute within
                        // the current SharedAttributeList (see the diagram in
                        // header file)
                        //  cd->ScoreboardIndex; // means nothing here

                        return cluster_index;
                    }

                    ++list_iter;
                    ++cluster_index;
                }
            }
        }

        LOG_WARN << LOG_LINE << "Did NOT find ClusterIndex {attribute:" << attribute_name << "} {variation:" << cluster_name << "}";
        return InvalidCharacteristicIndex;
    }


    void SharedAttributeIteratorAssistant::ClusterSharedAttribute(SharedAttribute* sa)
    {
        ClusterSharedAttribute(sa, sNoVariationClusterName, sNoVariationClusterName);
    }

    void SharedAttributeIteratorAssistant::ClusterSharedAttribute(SharedAttribute* sa, const std::string& variation_name, const std::string& variation_desc)
    {
        SharedAttributeClusterBundle::iterator found_cluster = myAttributeClusterBundle.find(variation_name);

        if ((sa->GetCharacteristicDescriptor())->Name == "Root Density Wrap None Plant 1")
        {
            LOG_INFO << LOG_LINE << "About to clusterificationise \"Root Density Wrap None Plant 1\" SharedAttribute";
        }

        // 1.
        // Find the cluster for the given variation_name.  If there isn't one yet,
        // create one.
        if (found_cluster == myAttributeClusterBundle.end())
        {
            SharedAttributeCluster new_cluster;
            SharedAttributeClusterBundle::value_type value(variation_name, new_cluster);
            // this insert form is used because it returns a pair with an iterator
            // that can be assigned to found_cluster
            found_cluster = (myAttributeClusterBundle.insert(value)).first;

            LOG_DEBUG << LOG_LINE << "Cluster added for variation_name '" << variation_name << "'";
        }


        // 2.
        // add the new SharedAttributeList to the Cluster for the named variation
        SharedAttributeCluster::iterator found_list = (*found_cluster).second.find(variation_desc);
        if (found_list == (*found_cluster).second.end())
        {
            SharedAttributeList new_list;
            SharedAttributeCluster::value_type value(variation_desc, new_list);
            found_list = ((*found_cluster).second.insert(value)).first;
        }

        // 3.
        // add the attribute to the cluster
        (*found_list).second.push_back(sa);

        // MSA 2016-09-23 Need to make the empty string a valid List. 
        // E.g. for Cluster 'VolumeObject', valid Lists should be
        // '', 'VolumeObject 1', ... 'VolumeObject n'
        LOG_DEBUG << LOG_LINE << "Attribute '" << (sa->GetCharacteristicDescriptor())->Name.c_str() << "' added to Cluster '" << variation_name << "' List '" << variation_desc << "'";
    }


    SharedAttributeIteratorAssistant::SharedAttributeIteratorAssistant(SharedAttributeManager* sam)
        : myManager(sam)
    {
        RootMapLoggerInitialisation("rootmap.SharedAttributeIteratorAssistant");
    }


    SharedAttributeIteratorAssistant::~SharedAttributeIteratorAssistant()
    {
    }

    void SharedAttributeIteratorAssistant::Log() const
    {
        LogBundle(myAttributeClusterBundle);
    }

    void SharedAttributeIteratorAssistant::LogBundle(const SharedAttributeClusterBundle& bundle) const
    {
        LOG_INFO << "Logging Bundles of Clusters";
        for (SharedAttributeClusterBundle::const_iterator biter = bundle.begin();
            biter != bundle.end(); ++biter)
        {
            LogBundleCluster((*biter).first, (*biter).second);
        }
    }

    void SharedAttributeIteratorAssistant::LogBundleCluster(const std::string& bundleName, const SharedAttributeCluster& cluster) const
    {
        for (SharedAttributeCluster::const_iterator citer = cluster.begin();
            citer != cluster.end(); ++citer)
        {
            LogBundleClusterList(bundleName, (*citer).first, (*citer).second);
        }
    }

    void SharedAttributeIteratorAssistant::LogBundleClusterList(const std::string& bundleName, const std::string& clusterName, const SharedAttributeList& bcList) const
    {
        for (SharedAttributeList::const_iterator liter = bcList.begin();
            liter != bcList.end(); ++liter)
        {
            LOG_INFO << "{Bundle:" << bundleName
                << "} {Cluster:" << clusterName << "} "
                << (*(*liter));
        }
    }

    const SharedAttributeCluster SharedAttributeIteratorAssistant::sNullCluster;
    const std::string SharedAttributeIteratorAssistant::sNoVariationClusterName = "[NOVARIATION]";
} /* namespace rootmap */
