//
//  Filename:       SharedAttributeCommon.h
//
//  Author:            Robert van Hugten
//
//  Description:    This file contains declarations and type definitions that
//                  are used throughout the SharedAttribute classes.
//
//  A SharedAttribute is one that is possible to be shared (used) by more than
//  one class, and more than one process.  For each group of processes and
//  classes that "use" a SharedAttribute, there is a leader whom "owns" all
//  SharedAttributes for that group.
//  
//  Each SharedAttribute has a "supplier" which provides storage for the
//  SharedAttribute's values.
//  
//  A CharacteristicDescriptor is combined with a SharedAttributeVariationList
//  to make a SharedAttributeFamily.  The CharacteristicDescriptor is applied
//  for each combination of the variations to make a SharedAttribute.
//  
//
//  Created:        20020821 (21 July 2002)
//
//  Modified:
//  
//  

#ifndef SharedAttributeCommon_H
#define SharedAttributeCommon_H

#include "core/common/Types.h"
#include "simulation/common/Types.h"

#include <map>
#include <vector>
#include <set>
#include <exception>


namespace rootmap
{
    //
    // Ownership of an attribute defines where storage will be allocated.
    //
    // Examples: PlantType/Plant/Scoreboard.
    //

    //
    //
    //
    class SharedAttributeOwner;
    typedef std::map<std::string, SharedAttributeOwner *> SharedAttributeOwnerMap;
    typedef std::vector<SharedAttributeOwner *> SharedAttributeOwnerList;
    typedef std::set<SharedAttributeOwner *> SharedAttributeOwnerSet;


    //
    // The owner identification is a short (4-byte) identifier given to each
    // class that can "own" an attribute.  Each owner also has a full length
    // name, which can be accessed by the GetSAOName.
    //
    typedef OSType SharedAttributeOwnerIdentification;


    //
    // Ownership of an attribute defines where storage will be allocated.
    //
    // Examples: PlantType/Plant/Scoreboard.
    //

    //
    //
    //
    class SharedAttributeSupplier;
    typedef std::map<std::string, SharedAttributeSupplier *> SharedAttributeSupplierMap;
    typedef std::vector<SharedAttributeSupplier *> SharedAttributeSupplierList;


    //
    // A Variation defines in which ways an attribute varies, ie. how it is divided
    //
    // For instance, an attribute might be per-PlantType.  It might be
    // per-PlantType per-Plant.  It might be per-PlantType per-RootOrder.
    // And so forth.
    //
    // The short version of the variation. 
    class SharedAttributeVariation;
    typedef std::map<std::string, SharedAttributeVariation *> SharedAttributeVariationMap;
    typedef std::vector<SharedAttributeVariation *> SharedAttributeVariationList;
    typedef std::vector<std::string> VariationNameArray;


    //
    //
    //
    class SharedAttribute;
    typedef std::vector<SharedAttribute *> SharedAttributeList;
    typedef SharedAttributeList::iterator SharedAttributeIterator;


    //
    // A SharedAttributeCluster is a vector of SharedAttributeList instances. 
    // 
    // As used by the SharedAttributeIteratorAssistant, each SharedAttributeList
    // has all the SharedAttributes for one instance of a particular Variation type.
    // The Cluster contains lists for all SharedAttributes by a Variation.
    //
    // For instance, a Cluster might hold all SharedAttributes as varied byPlant.
    // Each List within the Cluster holds all SharedAttributes for one Plant.
    //typedef std::vector<SharedAttributeList> SharedAttributeCluster;
    typedef std::map<std::string, SharedAttributeList> SharedAttributeCluster;


    //
    //
    //
    class SharedAttributeFamily;
    typedef std::vector<SharedAttributeFamily *> SharedAttributeFamilyList;


    //
    //
    //
    class SharedAttributeManager;
    typedef std::map<std::string, SharedAttributeManager *> SharedAttributeManagerMap;
    typedef std::vector<SharedAttributeManager *> SharedAttributeManagerList;


    typedef SignedIdentifier ClusterIndex;

    const ClusterIndex InvalidClusterIndex = -1;

    //
    //
    //
    //

    class SharedAttributeException : public std::exception
    {
    public:
        SharedAttributeException(const char* description)
            : myDescription(description)
        {
        }

        SharedAttributeException(const SharedAttributeException&)
        {
        }

        SharedAttributeException& operator=(const SharedAttributeException&) { return *this; }

        virtual ~SharedAttributeException()
        {
        };
        virtual const char* what() const { return myDescription.c_str(); };
        virtual const std::string description() const { return myDescription; };
    private:
        std::string myDescription;
    };
} /* namespace rootmap */

#endif // #ifndef SharedAttributeCommon_H
