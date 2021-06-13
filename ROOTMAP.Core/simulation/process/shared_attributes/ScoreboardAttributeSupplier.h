/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardAttributeSupplier.h
// Purpose:     Declaration of the ScoreboardAttributeSupplier class
// Created:     25/10/2002
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#ifndef ScoreboardAttributeSupplier_H
#define ScoreboardAttributeSupplier_H

#include "simulation/process/shared_attributes/SharedAttributeCommon.h"
#include "simulation/process/shared_attributes/SharedAttributeSupplier.h"

#include "core/log/Logger.h"

namespace rootmap
{
    struct CharacteristicDescriptor;
    class Scoreboard;

    /**
     * Concrete implementation of SharedAttributeSupplier that registers as
     * supplier of name "Scoreboard". Creates a ScoreboardSharedAttribute and
     * allocates storage in a specified Scoreboard
     */
    class ScoreboardAttributeSupplier : public SharedAttributeSupplier
    {
    public:
        /**
         * Creates a ScoreboardSharedAttribute and allocates storage for the
         * values in the specified scoreboard.
         */
        virtual SharedAttribute* CreateSharedAttribute(CharacteristicDescriptor* cd, long int variant1Max, long int variant2Max);

        /**
         * accessor for the mySupplierName data member. Used by the SharedAttributeRegistrar
         */
        virtual const std::string& GetSharedAttributeSupplierName();

        ScoreboardAttributeSupplier(Scoreboard* sb);

        virtual ~ScoreboardAttributeSupplier();

    private:
        RootMapLoggerDeclaration();

        /**
         * The scoreboard instance in which the SharedAttribute values will be stored
         */
        Scoreboard* myScoreboard;

        /**
         * the "name" of this supplier, used for registering with the Registrar
         */
        std::string mySupplierName;
    };
} /* namespace rootmap */

#endif    // ScoreboardAttributeSupplier_H
