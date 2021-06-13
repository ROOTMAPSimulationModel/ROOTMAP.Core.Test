/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessAttributeSupplier.h
// Purpose:     Declaration of the ProcessAttributeSupplier class
// Created:     25/10/2002
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#ifndef ProcessAttributeSupplier_H
#define ProcessAttributeSupplier_H

#include "simulation/process/shared_attributes/SharedAttributeCommon.h"
#include "simulation/process/shared_attributes/SharedAttributeSupplier.h"

#include "core/common/Types.h"
#include "core/scoreboard/ScoreboardBox.h"
#include "core/log/Logger.h"

namespace rootmap
{
    struct CharacteristicDescriptor;
    class Process;

    /**
     * Concrete implementation of SharedAttributeSupplier that registers as
     * supplier of name "Process". Creates a ProcessSharedAttribute and
     * allocates storage in a vector owned by this class
     */
    class ProcessAttributeSupplier : public SharedAttributeSupplier
    {
    public:
        /**
         *  This function does four (4) things:
         *
         *  1.  Allocate memory to store the value of this SharedAttribute
         *
         *  2.  Allocate a CharacteristicIndex to it and assign it to the
         *      "ScoreboardIndex" field of the CharacteristicDescriptor.
         *
         *  3.  Create and return an instance of the appropriate concrete class
         *      derived from SharedAttribute.
         *
         *  4.  Add the SharedAttribute to an internal list.  This is for future
         *      gui functionality, ie. so that the SharedAttributes supplied by
         *      this class might be displayed and viewed in a list.
         */
        SharedAttribute* CreateSharedAttribute(CharacteristicDescriptor* cd, long int variant1Max, long int variant2Max);

        /**
         *  Accessor for the myName data member
         */
        const std::string& GetSharedAttributeSupplierName();

    public:

        /**
         * called by
         *  ProcessSharedAttribute::DoGetValue1
         *  ProcessSharedAttribute::DoGetValue2
         */
        double GetAttributeValue(CharacteristicIndex ci);

        /**
         * called by
         *  ProcessSharedAttribute::DoSetValue1
         *  ProcessSharedAttribute::DoSetValue2
         */
        void SetAttributeValue(CharacteristicIndex ci, double value);


        ProcessAttributeSupplier(Process* p, const char* name);


        virtual ~ProcessAttributeSupplier();

    private:
        RootMapLoggerDeclaration();

        /**
         * Used by CreateSharedAttribute to allocate a new index to the
         * ScoreboardIndex field of the CharacteristicDescriptor.
         */
        long GetNewAttributeIndex();


        /**
         * Used by GetNewAttributeIndex()
         */
        long myNextUnusedAttributeIndex;

        /**
         *  The data member returned by GetSharedAttributeSupplierName().
         *  Initialised to "Process::GetProcessName()" during construction.
         */
        std::string mySupplierName;


        /**
         * Storage for the SharedAttribute values.
         */
        std::vector<CharacteristicValue> myAttributeValues;

        /**
         * List of SharedAttribute instances, owned by this class.
         */
        SharedAttributeList mySharedAttributes;

        /**
         *  Required for creating the ProcessSharedAttribute.
         */
        Process* myProcess;
    };


    inline void ProcessAttributeSupplier::SetAttributeValue(CharacteristicIndex ci, double value)
    {
        myAttributeValues[ci] = value;
    }


    inline double ProcessAttributeSupplier::GetAttributeValue(CharacteristicIndex ci)
    {
        return (myAttributeValues[ci]);
    }
} /* namespace rootmap */

#endif    // ProcessAttributeSupplier_H
