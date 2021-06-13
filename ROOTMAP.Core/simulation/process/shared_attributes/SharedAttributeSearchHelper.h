/////////////////////////////////////////////////////////////////////////////
// Name:        SharedAttributeSearchHelper.h
// Purpose:     Declaration of the SharedAttributeSearchHelper class
// Created:     10/01/2008
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#ifndef SharedAttributeSearchHelper_H
#define SharedAttributeSearchHelper_H

#include "simulation/process/shared_attributes/SharedAttributeCommon.h"
#include "core/scoreboard/ScoreboardBox.h"
#include "core/log/Logger.h"

#include <map>

namespace rootmap
{
    /**
     * Helps process modules to search for SharedAttributes, replaces the
     * SharedAttributeUser class (which was pretty useless and poorly named
     * too).
     *
     * Main functions:
     * 1. converts variation_name(s) to actual SharedAttributeVariations so that the
     *    other classes (Manager, Family, Owner) can use to SearchFor
     *
     * 2. Initially searches in the given Owner, if specified, before searching
     *    in the Manager
     */
    class SharedAttributeSearchHelper
    {
    public:

        /**
         * Searches for an attribute with no variations.
         *
         * @return the index of the attribute within the Cluster
         */
        CharacteristicIndex SearchForClusterIndex(const char* attribute_name);


        /**
         * Variation on the function below which accepts the variation names
         * as a comma-seperated string list
         *
         * @param attribute_name the base name of the characteristic
         * @param cluster_name the variation cluster to search in. This should also
         *          be the only variation for this attribute
         * @param variation_names all the variations for the attribute
         *
         * @return the index of the attribute within the Cluster
         */
        CharacteristicIndex SearchForClusterIndex(const char* attribute_name, const char* cluster_name, const char* variation_names);

        /**
         *    MSA 09.10.05 Overload of the above, allows for throwing an RmException
         *    instead of an assert failure if ClusterIndex not found (currently only used for constructing NonSpatialDataOutputStrategies)
         */
        CharacteristicIndex SearchForClusterIndex(const char* attribute_name, const char* cluster_name, const char* variation_names, const bool& throwExceptions);


        /**
         * Searches for an attribute with a number of variations. Returns
         * the index of the first attribute with these variations, otherwise
         * InvalidCharacteristicIndex
         *
         * @param attribute_name the base name of the characteristic
         * @param cluster_name the variation cluster to search in. This should also
         *          be the only variation for this attribute
         * @param variation_names all the variations for the attribute
         *
         * @return the index of the attribute within the Cluster
         */
        CharacteristicIndex SearchForClusterIndex(const char* attribute_name, const char* cluster_name, const std::vector<std::string>& variation_names);


        /**
         * Search for the attribute with the specific name. By which is meant,
         * the whole expanded-with-variation-names attribute. This also allows
         * searching for attributes with no expected variations.
         */
        SharedAttribute* SearchForAttribute(const char* attribute_name);


        /**
         * Search for the attribute with the given base attribute (characteristic
         * descriptor) name, with the given variation(s).
         *
         * Multiple variations are specified as a comma-separated list.
         */
        SharedAttribute* SearchForAttribute(const char* attribute_name, const char* variation_name);


        /**
         * Search for the attribute with the given base attribute (characteristic
         * descriptor) name, with the given variation(s). The third parameter
         * is the description (actual value) of an expected variation.
         *
         * eg. if the variation_name is specified as "Plant", you might
         * want to search for "Plant 1".
         *
         * @param attribute_name the base characteristic descriptor name
         * @param variation_name expected variations
         * @param variation_desc name of a specific variation
         */
        SharedAttribute* SearchForAttribute(const char* attribute_name, const char* variation_name, const char* variation_desc);

        /**
         * Search for the attribute with the given base attribute (characteristic
         * descriptor) name, with the given variation(s).
         */
        SharedAttribute* SearchForAttribute(const char* attribute_name, const std::vector<std::string>& variation_names);


        /**
         * Searches for multiple variations,
         */
        SharedAttribute* SearchForAttribute(const char* attribute_name, const std::vector<std::string>& variation_names, const char* variation_desc);

        /**
         * Note that the owner is optional. If it is given, it is searched first
         */
        SharedAttributeSearchHelper(SharedAttributeManager* sam, SharedAttributeOwner* owner = __nullptr);

    private:
        RootMapLoggerDeclaration();

        /**
         *
         */
        SharedAttributeManager* mySharedAttributeManager;

        /**
         *
         */
        SharedAttributeOwner* mySharedAttributeOwner;
    };
} /* namespace rootmap */

#endif    // SharedAttributeSearchHelper_H
