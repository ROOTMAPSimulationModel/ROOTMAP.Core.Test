#include "simulation/process/shared_attributes/SharedAttributeVariation.h"
// #include "simulation/process/shared_attributes/SharedAttributeManager.h"
#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"


namespace rootmap
{
    // 
    // a couple of quick functions, could be done inline in the header.
    //
    const std::string& SharedAttributeVariation::GetName() const
    {
        return mySharedAttributeVariationName;
    }

    SharedAttributeVariation::SharedAttributeVariation(const char* name)
        : mySharedAttributeVariationName(name)
    {
        SharedAttributeRegistrar::RegisterVariation(this);
    }


    SharedAttributeVariation::~SharedAttributeVariation()
    {
    }


    bool SharedAttributeVariation::isEqual(const SharedAttributeVariation& rhs) const
    {
        return (mySharedAttributeVariationName == rhs.mySharedAttributeVariationName);
    }

    bool SharedAttributeVariation::isEqual(const std::string& rhs) const
    {
        return (mySharedAttributeVariationName == rhs);
    }


    void SharedAttributeVariation::GetVariationString(long variation, std::string& s)
    {
        AppendVariationString(variation, s);
    }
} /* namespace rootmap */


