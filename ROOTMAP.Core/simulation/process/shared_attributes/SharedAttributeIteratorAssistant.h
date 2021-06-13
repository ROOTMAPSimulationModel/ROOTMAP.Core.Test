#ifndef SharedAttributeIteratorAssistant_H
#define SharedAttributeIteratorAssistant_H
/////////////////////////////////////////////////////////////////////////////
// Name:        SharedAttributeIteratorAssistant.h
// Purpose:     Declaration of the SharedAttributeIteratorAssistant class
// Created:     Jun 2002
// Author:      RvH
// $Date: 2009-10-02 22:31:43 +0800 (Fri, 02 Oct 2009) $
// $Revision: 77 $
// Copyright:   �2002-2008 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
//
// File 
//  
//
// Author
//  Robert van Hugten
//
// Description
//  This class manages data.  Its job is to gather and organise the SharedAttributes
//  (that are created and owned elsewhere) in a fashion that makes them usable by
//  ProcessModules that want to iterate over them according to one of the
//  SharedAttributeVariation types.  That means iterating by PlantType or by
//  Plant or by RootOrder.  At the moment.  
//
//  Q. So what's an appropriate name for this class?
//  A. ProcessSharedAttributeIteratingAssistant
//  
//  Even though it is intended as a means of assisting ProcessModules to iterate,
//  um...well...um... i guess that's what it should be called, then :
//  
//  SharedAttributeIteratorAssis.cp
//  SharedAttributeIteratorAstnt.cp
//  SharedAttribIterateAssistant.cp
//  1234567890123456789012345678901
//           1         2         3
//  
//  
//  
//  Here is an illustration of what is stored here: 
//  +-------+-----------+-----------+   Keys of SharedAttributeClusterBundle.
//  | Plant | PlantType | RootOrder |   The names of SharedAttributeVariations.
//  +-------+-----------+-----------+
//      |         |           |
//      |         |           +----------------------------------+
//      |         |           | RootOrder SharedAttributeCluster |
//      |         |           +----------------------------------+
//      |         |               +----------------------------------------+----------------------------------------+------------------------------------ -- - - 
//      |         |               |                                        |                                        |
//      |         |               +-------------------------------------+  +-------------------------------------+  +---------------------------------------+
//      |         |               | Growth Rate Max Plant A RootOrder 0 |  | Growth Rate Max Plant A RootOrder 1 |  | Growth Rate Max Plant B RootOrder 2   |
//      |         |               +-------------------------------------+  +-------------------------------------+  +---------------------------------------+
//      |         |               | Growth Rate Max Plant B RootOrder 0 |  | Growth Rate Max Plant B RootOrder 1 |  | Growth Rate Max Plant B RootOrder 2   |
//      |         |               +-------------------------------------+  +-------------------------------------+  +---------------------------------------+
//      |         |               | Branch Lag Time RootOrder 0         |  | Branch Lag Time RootOrder 1         |  | Branch Lag Time RootOrder 2           |
//      |         |               +-------------------------------------+  +-------------------------------------+  +---------------------------------------+
//      |         |               | Water Flux Plant A RootOrder 0      |  | Water Flux Plant A RootOrder 1      |  | Water Flux Plant A RootOrder 2        |
//      |         |               +-------------------------------------+  +-------------------------------------+  +---------------------------------------+
//      |         |               | Water Flux Plant B RootOrder 0      |  | Water Flux Plant B RootOrder 1      |  | Water Flux Plant B RootOrder 2        |
//      |         |               +-------------------------------------+  +-------------------------------------+  +---------------------------------------+
//      |         |               |                                     |  |                                     |  |                                       |
//      |         |
//      |         +----------------------------------+
//      |         | PlantType SharedAttributeCluster |
//      |         +----------------------------------+
//      |             |
//      |             +---------------------------------+------------------------- - - -
//      |             |                                 |
//      |             +------------------------------+  +------------------------------+
//      |             | Germination Lag RM PlantType |  | Germination Lag XY PlantType |
//      |             +------------------------------+  +------------------------------+
//      |             | Full Cover RM PlantType      |  | Full Cover RM PlantType      |
//      |             +------------------------------+  +------------------------------+
//      |             | Ripening RM PlantType        |  | Ripening RM PlantType        |
//      |             +------------------------------+  +------------------------------+
//      |             | Harvest RM PlantType         |  | Harvest RM PlantType         |
//      |             +------------------------------+  +------------------------------+
//      |             | Ground Cover Max RM PlantType|  | Ground Cover Max RM PlantType|
//      |             +------------------------------+  +------------------------------+
//      |             |                              |  |                              |
//      |
//      +------------------------------+
//      | Plant SharedAttributeCluster |
//      +------------------------------+
//          |                                                                                    Index returned by
//          +------------------------------------------+------------------------------------ - - - 
//          |                                          |                                        SearchForClusterIndex
//          +---------------------------------------+  +---------------------------------------+
//          | Nitrate Uptake Plant A                |  | Nitrate Uptake Plant B                |        0
//          +---------------------------------------+  +---------------------------------------+
//          | Water Uptake Plant A                  |  | Water Uptake Plant B                  |        1
//          +---------------------------------------+  +---------------------------------------+
//          | Phosphorus Uptake Plant A             |  | Phosphorus Uptake Plant B             |        2
//          +---------------------------------------+  +---------------------------------------+
//          | Growth Rate Max Plant A RootOrder 0   |  | Growth Rate Max Plant B RootOrder 0   |        3
//          +---------------------------------------+  +---------------------------------------+
//          | Growth Rate Max Plant A RootOrder 1   |  | Growth Rate Max Plant B RootOrder 1   |        4
//          +---------------------------------------+  +---------------------------------------+
//          | Growth Rate Max Plant A RootOrder 2   |  | Growth Rate Max Plant B RootOrder 2   |        5
//          +---------------------------------------+  +---------------------------------------+
//          | Growth Rate Max Plant A RootOrder >=3 |  | Growth Rate Max Plant B RootOrder >=3 |        6
//          +---------------------------------------+  +---------------------------------------+
//          |                                       |  |                                       |
//
//  Note that for the purposes of clarity of the illustration, only 2 Plants'
//  lists are shown, and the placement of “Growth Rate Max Plant (M) RootOrder (N)”
//  in the RootOrder cluster has been omitted.
//  
//  It is possible that the Cluster might also need to be a map of Lists key'd
//  by Plant Name, however they are intended for iterating over, not random access.
//  If a specific Plant (or any other list in a cluster) is needed, then the
//  functionality provided by this IteratorAssistant is not required - just
//  retrieving the SA via the Manager is sufficient.
//
//  20030401 RvH In order to properly support iterating, it is necessary to make
//  sure that all SharedAttributes for a particular Plant are in one
//  SharedAttributeList.  To add a newly added SharedAttribute to the correct
//  list in DoVariationChange, we need to be able to access a List by its
//  VariationString, ie. Plant Name or PlantType name etcetera.  On the other
//  hand, having Lists in the Cluster key'd by VariationString would mean a
//  slight complication in using the GetCluster() for iterating, ie. :
//  
//      const SharedAttributeList & sa_list = (*cluster_iter).second;
//
//  instead of
//
//      const SharedAttributeList & sa_list = *cluster_iter;
//
//  which realistically is an unnecessary additional confusion.
//  
//
//
//  The alternative would be to have the non-map structure originally devised,
//  with an additional map mirroring that.  However then the original non-key'd
//  SharedAttributeLists would need to be held by pointer rather than by value
//  which would still add in an extra piece of indirection while using the
//  iterator AND be an extra level of complication at this implementation end.
//  Now while i have intended to keep as much of the complication at this end
//  of the usage chain, i don't think that much complication is warranted given
//  the benefits.
//  
//          +-----------------------------+-----------------------------+ - - -
//          | Plant A SharedAttributeList | Plant B SharedAttributeList | Plant
//          +-----------------------------+-----------------------------+ - - -
//              |
//  
//
//
//  20080123 RvH
//  Generally, a process module will need to know exactly the variations of the
//  attribute it is using. For instance, if a process searches for Growth Rate
//  Max based on Plant variation, it isn't a good thing if the attribute also
//  varies by RootOrder and the process doesn't know that.
//
//  However, for the purposes of Iterating, this structure still makes sense,
//  because it is not intended for searching. By the time the process uses the
//  GetCluster() method, it should have already found the attribute with
//  appropriate variations.
//  
//  The conclusion is that the SearchForClusterIndex routines must be improved.
//
//  20080613 RvH
//  The SearchForClusterIndex routines have been tightened in this class and
//  extended in the SASearchHelper and SAManager classes
//  
#include "simulation/process/shared_attributes/SharedAttributeCommon.h"
#include "core/scoreboard/ScoreboardBox.h"
#include "core/log/Logger.h"

#include <string>
#include <vector>
#include <set>

namespace rootmap
{
    //
    // A SharedAttributeCluster is a vector of SharedAttributeList instances.  Each
    // List has all the SharedAttributes for one instance a particular Variation type.
    // The Cluster would then hold the lists for all SharedAttributes by a Variation.
    //
    // For instance, a Cluster might hold all SharedAttributes as varied byPlant.
    // Each List within the Cluster holds all SharedAttributes for one Plant.
    typedef SharedAttributeCluster::const_iterator SharedAttributeClusterIterator;


    //
    // This map holds all the SharedAttributeClusters, keyed by the name of each
    // variation.  Hopefully, this will work like magic.  What we see here is fully
    // expanded as:
    //
    //        |-Bundle-----------------------------------------------------------------|
    //                            |-Cluster------------------------------------------|
    //                                                |-List-----------------------|
    // typdef std::map< std::string, std::map< std::string, std::vector<SharedAttribute *> > > SharedAttributeClusterBundle;
    //
    // A map key'd by variation string of a map key'd by of a vector of pointers to SharedAttributes.
    //
    typedef std::map<std::string, SharedAttributeCluster> SharedAttributeClusterBundle;

    class SharedAttributeIteratorAssistant
    {
    public:
        SharedAttributeIteratorAssistant(SharedAttributeManager* sam);
        virtual ~SharedAttributeIteratorAssistant();

        /**
         * Get an iterator for a variation (ie. Variation's Cluster)
         *
         * Returns the Cluster (vector of SharedAttributeList's) for a variation.
         *
         * See "GetCluster() Typical Usage" below for an example of typical usage
         */
        const SharedAttributeCluster& GetCluster(const std::string& variation_name) const;

        //
        // Typical usage of SharedAttributeIteratorAssistant::GetCluster()
        //
#ifdef SharedAttributeIteratorAssistant_Sample_Code
        /*

        20080127 RvH needed to place C-style comments; only #ifdefing this code was
                 confusing EA reverse engineering

        const SharedAttributeCluster & cluster = theAssistant->GetCluster("Plant");

        //
        // This is effectively a "for each Plant do..." loop
        for ( SharedAttributeCluster::const_iterator cluster_iter = cluster.begin() ;
              cluster_iter != cluster.end() ;
              ++cluster_iter
            )
        {
            const SharedAttributeList & sa_list = (*cluster_iter).second

            //
            // the variable saNitrateUptakePerPlantIndex is a data member that was
            // found during ::Initialise() by calling
            //      SharedAttributeUser::SearchForClusterIndex("Nitrate Uptake","Plant")
            // Similar for phosphorus.
            nitrate_uptake_sa = sa_list[saNitrateUptakePerPlantIndex];
            phosphorus_uptake_sa = sa_list[saPhosphorusUptakePerPlantIndex];

            //
            // See "Usage:" in "Scoreboard.h" for comments about iterating through
            // the scoreboard.
            for ( BoxIndex box_index = scoreboard->begin() ;
                  box_index < scoreboard->end() ;
                  box_index++
                )
            {
                double nitrate_uptake = nitrate_uptake_sa->GetValue(box_index);
                double phosphorus_uptake0 = phosphorus_uptake_sa->GetValue(box_index,0);
                double phosphorus_uptake1 = phosphorus_uptake_sa->GetValue(box_index,1);
                double phosphorus_uptake2 = phosphorus_uptake_sa->GetValue(box_index,2);
                double phosphorus_uptake3 = phosphorus_uptake_sa->GetValue(box_index,3);
            }
        }
        */
#endif // #ifdef SharedAttributeIteratorAssistant_Sample_Code
        // ////////////////////////////
        //
        // ////////////////////////////
        /**
         * Search for the index of an Attribute within the special "No variation" cluster
         */
        CharacteristicIndex SearchForClusterIndex(const std::string& attribute_name) const;

        /**
         * Search for the index of an Attribute within the variation cluster
         *
         * @param attribute_name the full name of the characteristic of the first SA
         *        in the family, eg.
         *        "Water Uptake Plant 1"
         *        "Water Uptake RootOrder0"
         *        "Water Uptake Plant 1 RootOrder0"
         *        Which is because this class only knows names not variations, so
         *        the full name must be spec'd to avoid confusion between SAs from
         *        Families with same base characteristic but different variations.
         *
         * @param cluster_name the variation cluster to search in
         */
        CharacteristicIndex SearchForClusterIndex(const std::string& attribute_name, const std::string& cluster_name) const;


        // ////////////////////////////
        //
        // ////////////////////////////
        /**
         * Cluster[ification]ises the SharedAttribute. THis means that the SA is placed
         * in all the appropriate clusters for which it has variations. For this method,
         * there are no variations so it is placed in a special "no variation" cluster.
         */
        void ClusterSharedAttribute(SharedAttribute* sa);

        /**
         * Clusterificationises the SharedAttribute. THis means that the SA is placed
         * in all the appropriate clusters for which it has variations.
         */
        void ClusterSharedAttribute(SharedAttribute* sa, const std::string& variation_name, const std::string& variation_desc);

        void Log() const;

        void LogBundle(const SharedAttributeClusterBundle& bundle) const;

        void LogBundleCluster(const std::string& bundleName, const SharedAttributeCluster& cluster) const;

        void LogBundleClusterList(const std::string& bundleName, const std::string& clusterName, const SharedAttributeList& bcList) const;

    private:
        RootMapLoggerDeclaration();
        //
        //
        SharedAttributeManager* myManager;

        //
        //
        SharedAttributeClusterBundle myAttributeClusterBundle;

        //
        static const SharedAttributeCluster sNullCluster;

        static const std::string sNoVariationClusterName;
    };
} /* namespace rootmap */

#endif  // #ifndef SharedAttributeIteratorAssistant_H
