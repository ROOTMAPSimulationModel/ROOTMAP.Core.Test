#include "simulation/common/IdentifierUtility.h"
#include "core/common/Structures.h"
#include "core/common/RmAssert.h"

namespace rootmap
{
    RootMapLoggerDefinition(IdentifierUtility);

    IdentifierUtility& IdentifierUtility::instance()
    {
        // MSA 11.03.28 Not thread-safe per MSVC compiler warning C4640
        static IdentifierUtility iu; // the one-and-only instance
        return iu;
    }


    IdentifierUtility::IdentifierUtility()
        : m_nextCharacteristicIdentifier(0)
        , m_nextProcessIdentifier(0)
        , m_nextPlantIdentifier(0)
        , m_nextPlantTypeIdentifier(0)
        , m_nextPlantSummaryIdentifier(0)
        , m_perStratumCharacteristicIdentifierBandwidth(0)
        , m_perStratumProcessIdentifierBandwidth(0)
        , m_perStratumPlantSummaryIdentifierBandwidth(0)
    {
        RootMapLoggerInitialisation("rootmap.IdentifierUtility");
    }

    IdentifierUtility::~IdentifierUtility()
    {
        LOG_NOTE << "IdentityUtility destructor; NEXT values "
            << "{CharacteristicId:" << m_nextCharacteristicIdentifier
            << ", ProcessId:" << m_nextProcessIdentifier
            << ", PlantId:" << m_nextPlantIdentifier
            << ", PlantTypeId:" << m_nextPlantTypeIdentifier
            << ", PlantSummaryId:" << m_nextPlantSummaryIdentifier
            << "}";
    }

    // ///////////////////////////////
    //      Use
    // ///////////////////////////////
    CharacteristicIdentifier IdentifierUtility::useNextCharacteristicIdentifier(const ScoreboardStratum& /* ss */)
    {
        // CharacteristicIdentifier is a signed value.
        // We don't want negative values (they are invalid).
        if ((m_nextCharacteristicIdentifier + 1) < 0)
        {
            m_nextCharacteristicIdentifier = 0;
        }
        return ++m_nextCharacteristicIdentifier;
    }

    ProcessIdentifier IdentifierUtility::useNextProcessIdentifier(const ScoreboardStratum& /* ss */)
    {
        // prevent signed value overflow
        if ((m_nextProcessIdentifier + 1) < 0)
        {
            m_nextProcessIdentifier = 0;
        }
        return ++m_nextProcessIdentifier;
    }

    PlantIdentifier IdentifierUtility::useNextPlantIdentifier()
    {
        // prevent signed value overflow
        if ((m_nextPlantIdentifier + 1) < 0)
        {
            m_nextPlantIdentifier = 0;
        }
        return ++m_nextPlantIdentifier;
    }

    PlantTypeIdentifier IdentifierUtility::useNextPlantTypeIdentifier()
    {
        // prevent signed value overflow
        if ((m_nextPlantTypeIdentifier + 1) < 0)
        {
            m_nextPlantTypeIdentifier = 0;
        }
        return ++m_nextPlantTypeIdentifier;
    }

    PlantSummaryIdentifier IdentifierUtility::useNextPlantSummaryIdentifier(const ScoreboardStratum& /* ss */)
    {
        // prevent signed value overflow
        if ((m_nextPlantSummaryIdentifier + 1) < 0)
        {
            m_nextPlantSummaryIdentifier = 0;
        }
        return ++m_nextPlantSummaryIdentifier;
    }

    PlantElementIdentifier IdentifierUtility::useNextPlantElementIdentifier()
    {
        // prevent signed value overflow
        RmAssert(((m_nextPlantElementIdentifier + 1) < InvalidUIdentifier), "Reached limit of PlantElementIdentifier values");

        //if ( (m_nextPlantElementIdentifier+1)>=InvalidUIdentifier )
        //{
        //    m_nextPlantElementIdentifier = 0;
        //}
        return ++m_nextPlantElementIdentifier;
    }

    // ///////////////////////////////
    //      Peek
    // ///////////////////////////////
    CharacteristicIdentifier IdentifierUtility::peekNextCharacteristicIdentifier(const ScoreboardStratum& /* ss */)
    {
        return m_nextCharacteristicIdentifier;
    }

    ProcessIdentifier IdentifierUtility::peekNextProcessIdentifier(const ScoreboardStratum& /* ss */)
    {
        return m_nextProcessIdentifier;
    }

    PlantIdentifier IdentifierUtility::peekNextPlantIdentifier()
    {
        return m_nextPlantIdentifier;
    }

    PlantTypeIdentifier IdentifierUtility::peekNextPlantTypeIdentifier()
    {
        return m_nextPlantTypeIdentifier;
    }

    PlantSummaryIdentifier IdentifierUtility::peekNextPlantSummaryIdentifier(const ScoreboardStratum& /* ss */)
    {
        return m_nextPlantSummaryIdentifier;
    }

    PlantElementIdentifier IdentifierUtility::peekNextPlantElementIdentifier()
    {
        return m_nextPlantElementIdentifier;
    }

    const SignedIdentifier IdentifierUtility::InvalidIdentifier = -1;
    const UnsignedIdentifier IdentifierUtility::InvalidUIdentifier = std::numeric_limits<UnsignedIdentifier>::max();
} /* namespace rootmap */


