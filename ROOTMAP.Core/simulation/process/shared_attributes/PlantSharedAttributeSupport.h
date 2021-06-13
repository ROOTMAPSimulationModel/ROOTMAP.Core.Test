//
//
//
//
#ifndef PlantSharedAttributeSupport_H
#define PlantSharedAttributeSupport_H

#include "simulation/process/shared_attributes/SharedAttributeVariation.h"
#include "simulation/process/shared_attributes/SharedAttributeOwner.h"

namespace rootmap
{
    class PlantCoordinator;

    class PlantSharedAttributeVariation
        : public SharedAttributeVariation
    {
    public:
        PlantSharedAttributeVariation(const char* name, PlantCoordinator* pl);

        ~PlantSharedAttributeVariation()
        {
        }

        long int GetNumberOf();
        SharedAttributeVariationType GetType();
        void AppendVariationString(long variation, std::string& s);

    private:
        PlantCoordinator* myPlantCoordinator;
    };


    class PlantTypeSharedAttributeVariation
        : public SharedAttributeVariation
    {
    public:
        PlantTypeSharedAttributeVariation(const char* name, PlantCoordinator* pl);

        ~PlantTypeSharedAttributeVariation()
        {
        }

        long int GetNumberOf();
        SharedAttributeVariationType GetType();
        void AppendVariationString(long variation, std::string& s);

    private:
        PlantCoordinator* myPlantCoordinator;
    };


    /**
     * Concrete derivation of SharedAttributeVariation, to describe the number of
     * RootOrders a variation will employ.
     */
    class RootOrderSharedAttributeVariation
        : public SharedAttributeVariation
    {
    public:
        RootOrderSharedAttributeVariation(const char* name, long int number_of_root_orders);

        ~RootOrderSharedAttributeVariation()
        {
        }

        long int GetNumberOf();
        SharedAttributeVariationType GetType();
        void AppendVariationString(long variation, std::string& s);

    private:
        long int myNumberOfRootOrders;
    };

    /**
     * Concrete derivation of SharedAttributeVariation, to describe
     * the spatial subsections a variation will use (coincident with VolumeObject 'x', coincident with VolumeObject 'y', [etc,] coincident with no VolumeObjects)
     */
    class VolumeObjectSharedAttributeVariation
        : public SharedAttributeVariation
    {
    public:
        VolumeObjectSharedAttributeVariation(const char* name, long int number_of_spatial_subsections);

        ~VolumeObjectSharedAttributeVariation()
        {
        }

        long int GetNumberOf();
        SharedAttributeVariationType GetType();
        void AppendVariationString(long variation, std::string& s);
        void AppendVariationString(const size_t variation, std::string& s);

    private:
        long int myNumberOfSpatialSubsections;
    };
} /* namespace rootmap */

#endif // #ifndef PlantSharedAttributeSupport_H
