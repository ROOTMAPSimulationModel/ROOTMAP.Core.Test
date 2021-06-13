//
// File:
//  IdentifierUtility.h
//
// Description:
//  contains constants and functions for maintaining various ids, such as -
//  * the next process characteristic id for processes in each volume
//  * the next plant id
//  * the next plant characteristic (summary) id
//
// Working Notes:
//  This is a temporary measure. These functions and constants have been
//  taken from GroundVolumeTypes.h, so that they are able to depend on
//  ScoreboardStratum instead.  Since the functionality and data goes
//  together, and should only be accessed via the functions, it seems right
//  fair that they become a class. Encapsulation and all that.
//
#ifndef IdentifierUtility_H
#define IdentifierUtility_H

#include "core/macos_compatibility/macos_compatibility.h"
#include "core/scoreboard/ScoreboardStratum.h"
#include "simulation/common/Types.h"
#include "core/common/Types.h"
#include "core/log/Logger.h"

namespace rootmap
{
    class IdentifierUtility
    {
    public:
        ///
        /// Each of these functions retrieves the next available Identifier
        /// of the given variety and marks it as being in use.
        CharacteristicIdentifier useNextCharacteristicIdentifier(const ScoreboardStratum& ss);
        ProcessIdentifier useNextProcessIdentifier(const ScoreboardStratum& ss);

        /** Plants are a type of process, however are implicitly part of the Global
         * ScoreboardStratum so require no scoreboard
         */
        PlantIdentifier useNextPlantIdentifier();
        PlantTypeIdentifier useNextPlantTypeIdentifier();
        PlantSummaryIdentifier useNextPlantSummaryIdentifier(const ScoreboardStratum& ss);
        PlantElementIdentifier useNextPlantElementIdentifier();

        ///
        /// Each of these functions retrieves the next available Identifier
        /// of the given variety but leaves it unused
        CharacteristicIdentifier peekNextCharacteristicIdentifier(const ScoreboardStratum& ss);
        ProcessIdentifier peekNextProcessIdentifier(const ScoreboardStratum& ss);
        PlantIdentifier peekNextPlantIdentifier();
        PlantTypeIdentifier peekNextPlantTypeIdentifier();
        PlantSummaryIdentifier peekNextPlantSummaryIdentifier(const ScoreboardStratum& ss);
        PlantElementIdentifier peekNextPlantElementIdentifier();

        static IdentifierUtility& instance();

        static const SignedIdentifier InvalidIdentifier;
        static const UnsignedIdentifier InvalidUIdentifier;

    private:

        IdentifierUtility();
        virtual ~IdentifierUtility();

        RootMapLoggerDeclaration();

        ///
        /// the next available characteristic identifier
        CharacteristicIdentifier m_nextCharacteristicIdentifier;

        ///
        /// the next available process identifier
        ProcessIdentifier m_nextProcessIdentifier;

        ///
        /// the next available plant identifier
        PlantIdentifier m_nextPlantIdentifier;

        ///
        /// the next available plant type identifier
        PlantTypeIdentifier m_nextPlantTypeIdentifier;

        ///
        /// the next available plant summary identifier
        PlantSummaryIdentifier m_nextPlantSummaryIdentifier;

        ///
        /// the next available plant element identifier
        PlantElementIdentifier m_nextPlantElementIdentifier;

        ///
        /// how much of the total identifier spectrum each stratum's
        /// CharacteristicIdentifiers utilises
        long int m_perStratumCharacteristicIdentifierBandwidth;

        ///
        /// how much of the total identifier spectrum each stratum's
        /// ProcessIdentifiers utilises
        long int m_perStratumProcessIdentifierBandwidth;

        ///
        /// how much of the total identifier spectrum each stratum's
        /// PlantSummaryIdentifiers utilises
        long int m_perStratumPlantSummaryIdentifierBandwidth;
    };
} /* namespace rootmap */

#endif // #ifndef IdentifierUtility_H
