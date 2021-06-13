//
// Filename:
//  SharedAttributeVariation.h
//
// Author:
//  Robert van Hugten
//
// Description:
//  The SharedAttributeVariation class is currently used for encapsulation
//  of the name and number of variations available for an instance of a
//  SharedAttribute.  I'm certain more will come of it soon.
//
//  Other classes declared and defined in this file are:
//      RootOrderSharedAttributeVariation
//
//  Related subclasses are:
//      PlantSharedAttributeVariation
//      PlantTypeSharedAttributeVariation
//          (both in "PlantSharedAttributeSupport.h")
//
//    MSA 11.01.16 Update: Adding a new SharedAttributeVariation:
//        VolumeObjectVariation
//    Declared and defined with the others.

#ifndef SharedAttributeVariation_H
#define SharedAttributeVariation_H

#include "simulation/process/shared_attributes/SharedAttributeCommon.h"

namespace rootmap
{
    enum SharedAttributeVariationType
    {
        SAV_Fixed,
        SAV_Variable
    };

    class SharedAttributeVariation
    {
    public:
        /**
         *
         */
        virtual const std::string& GetName() const;

        /**
         *
         */
        virtual long int GetNumberOf() = 0;

        /**
         *
         */
        virtual SharedAttributeVariationType GetType() = 0;

        /**
         *
         */
        virtual void GetVariationString(long variation, std::string& s);

        /**
         *
         */
        virtual void AppendVariationString(long variation, std::string& s) = 0;


        /**
         *
         */
        SharedAttributeVariation(const char* name);

        /**
         *
         */
        virtual ~SharedAttributeVariation();

        /**
         * virtual methods, used in the equality operator (operator==) because this
         * is an abstract base class (and so cannot predict how derived concrete
         * classes will want to determine equality.
         */
        virtual bool isEqual(const SharedAttributeVariation& rhs) const;
        virtual bool isEqual(const std::string& rhs) const;


        /**
         *
         */
        bool operator==(const SharedAttributeVariation& rhs) const;
        bool operator==(const std::string& rhs) const;

    private:
        //
        // subclasses (which are required due to the pure virtual above) may have a
        // "myName" data member, so we'll call this one something more original.
        std::string mySharedAttributeVariationName;
    };


    inline bool SharedAttributeVariation::operator==(const SharedAttributeVariation& rhs) const
    {
        return (isEqual(rhs));
    }

    inline bool SharedAttributeVariation::operator==(const std::string& rhs) const
    {
        return (isEqual(rhs));
    }
} /* namespace rootmap */

#endif    // SharedAttributeVariation_H
