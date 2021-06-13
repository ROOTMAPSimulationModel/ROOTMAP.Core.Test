//
//
//
//
#include "simulation/process/shared_attributes/PlantSharedAttributeSupport.h"
#include "simulation/process/plant/PlantCoordinator.h"
#include "simulation/process/plant/PlantType.h"
#include "simulation/process/plant/Plant.h"
#include "simulation/process/shared_attributes/SharedAttribute.h"
#include "simulation/process/shared_attributes/SharedAttributeSupplier.h"

#include "core/utility/Utility.h"
#include "core/common/RmAssert.h"


namespace rootmap
{
    long int PlantSharedAttributeVariation::GetNumberOf()
    {
        return myPlantCoordinator->GetNumPlants();
    }

    SharedAttributeVariationType PlantSharedAttributeVariation::GetType()
    {
        return SAV_Variable;
    }

    void PlantSharedAttributeVariation::AppendVariationString(long variation, std::string& s)
    {
        const long num_plants = myPlantCoordinator->GetNumPlants();

        // only do this if more plants available than current variation number
        // (eg. if 1 plant, can only handle variation #0)
        // (    if 2 plants, can only handle variations #0 #1)
        if (num_plants > variation)
        {
            Plant* p = myPlantCoordinator->GetPlant(variation);

            std::string p_name = p->GetProcessName();

            s += p_name;
        }
        // otherwise if there are no plants at all, which there might be,
        else if (num_plants < 1)
        {
            s += "<NO PLANTS>";
        }
        // otherwise if greater variation requested than number of plants,
        else
        {
            // assert something we know is false - this is more indicative when shown
            // in the assert dialog than just "false"
            RmAssert(num_plants > variation, "Greater number of variations requested than Plants available");
        }
    }

    PlantSharedAttributeVariation::PlantSharedAttributeVariation
    (const char* name,
        PlantCoordinator* pl
    )
        : SharedAttributeVariation(name)
        , myPlantCoordinator(pl)
    {
    }


    //
    //
    //
    //
    long int PlantTypeSharedAttributeVariation::GetNumberOf()
    {
        return myPlantCoordinator->GetNumPlantTypes();
    }

    SharedAttributeVariationType PlantTypeSharedAttributeVariation::GetType()
    {
        return SAV_Variable;
    }

    void PlantTypeSharedAttributeVariation::AppendVariationString(long variation, std::string& s)
    {
        const long num_planttypes = myPlantCoordinator->GetNumPlantTypes();

        // only do this if more plants available than current variation number
        // (eg. if 1 plant, can only handle variation #0)
        // (    if 2 plants, can only handle variations #0 #1)
        if (num_planttypes > variation)
        {
            PlantType* pt = myPlantCoordinator->GetPlantType(variation);

            std::string pt_name;
            pt->GetName(pt_name);

            s += pt_name;
        }
        // otherwise if there are no plants at all, which there might be,
        else if (num_planttypes < 1)
        {
            s += "<NO PLANTTYPES>";
        }
        // otherwise if greater variation requested than number of plants,
        else
        {
            // assert something we know is false - this is more indicative when shown
            // in the assert dialog than just "false"
            RmAssert(num_planttypes > variation, "Greater number of variations requested than PlantTypes available");
        }
    }

    PlantTypeSharedAttributeVariation::PlantTypeSharedAttributeVariation
    (const char* name,
        PlantCoordinator* pl
    )
        : SharedAttributeVariation(name)
        , myPlantCoordinator(pl)
    {
    }

    void RootOrderSharedAttributeVariation::AppendVariationString(long variation, std::string& s)
    {
        s += GetName();

        if (variation >= (myNumberOfRootOrders - 1))
        {
            variation = myNumberOfRootOrders - 1;
            s.append(">=");
        }

        s += Utility::ToString(variation);
    }

    void VolumeObjectSharedAttributeVariation::AppendVariationString(long variation, std::string& s)
    {
        AppendVariationString((const size_t)variation, s);
    }

    void VolumeObjectSharedAttributeVariation::AppendVariationString(const size_t variation, std::string& s)
    {
        if (variation > 0)
        {
            s += GetName() + " " + Utility::ToString(variation);
        }
        else
        {
            wxString temp = s;
            s.assign(temp.Trim());
        }
    }

    long int RootOrderSharedAttributeVariation::GetNumberOf()
    {
        return myNumberOfRootOrders;
    }

    long int VolumeObjectSharedAttributeVariation::GetNumberOf()
    {
        return myNumberOfSpatialSubsections;
    }

    SharedAttributeVariationType RootOrderSharedAttributeVariation::GetType()
    {
        return SAV_Fixed;
    }

    SharedAttributeVariationType VolumeObjectSharedAttributeVariation::GetType()
    {
        return SAV_Fixed;
    }

    RootOrderSharedAttributeVariation::RootOrderSharedAttributeVariation
    (const char* name,
        long int number_of_root_orders
    )
        : SharedAttributeVariation(name)
        , myNumberOfRootOrders(number_of_root_orders)
    {
    }

    VolumeObjectSharedAttributeVariation::VolumeObjectSharedAttributeVariation
    (const char* name,
        long int number_of_spatial_subsections
    )
        : SharedAttributeVariation(name)
        , myNumberOfSpatialSubsections(number_of_spatial_subsections)
    {
    }
} /* namespace rootmap */

