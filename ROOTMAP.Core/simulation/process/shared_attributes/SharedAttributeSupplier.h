/////////////////////////////////////////////////////////////////////////////
// Name:        SharedAttributeSupplier.h
// Purpose:     Declaration of the SharedAttributeSupplier class
// Created:     22/10/2002
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#ifndef SharedAttributeSupplier_H
#define SharedAttributeSupplier_H

#include "simulation/process/shared_attributes/SharedAttributeCommon.h"

namespace rootmap
{
    struct CharacteristicDescriptor;

    /**
     * The SharedAttributeSupplier class is an abstract creator, used by the
     * SharedAttributeFamily to create SharedAttributes. Derived supplier types
     * must create an appropriate derived SharedAttribute, and allocate memory
     * to store the value[s] of the SharedAttribute.
     */
    class SharedAttributeSupplier
    {
    public:
        /**
         *  As stated in the main class description, there are two (2) activities
         *  that MUST be undertaken by concrete implementations of this function:
         *
         *  1.  Allocate memory to store the value of this SharedAttribute
         *
         *  2.  Create and return an instance of the appropriate concrete class
         *      derived from SharedAttribute.
         *
         *
         *  Additionally, there are some optional behaviours that concrete
         *  derivations may want to undertake:
         *
         *  3>  Adjust the CharacteristicDescriptor so that the derived
         *      SharedAttribute class might function correctly.  For example, for
         *      the PlantCoordinator and Scoreboard supplier derived classes, this means
         *      allocating a CharacteristicIndex to the attribute and assigning it
         *      to the "ScoreboardIndex" field of the CharacteristicDescriptor.
         *
         *  4>  Optionally add the SharedAttribute to an internal list.
         */
        virtual SharedAttribute* CreateSharedAttribute(CharacteristicDescriptor* cd, long int variant1Max, long int variant2Max) = 0;


        /**
         *  This function should return a string which identifies this supplier.
         *
         *  It is used to match this supplier with the name given in the xml file.
         */
        virtual const std::string& GetSharedAttributeSupplierName() = 0;

        virtual ~SharedAttributeSupplier();

    protected:
        /**
         * Default constructor protected, though pure virtual methods will
         * prevent direct construction
         */
        SharedAttributeSupplier();
    };
} /* namespace rootmap */

#endif    // SharedAttributeSupplier_H
