/////////////////////////////////////////////////////////////////////////////
// Name:        TXScoreboardDA.cpp
// Purpose:     Definition of the TXScoreboardDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/tinyxmlconfig/TXScoreboardDA.h"

#include "core/common/RmAssert.h"
#include "core/utility/StringParserUtility.h"


namespace rootmap
{
    using StringParserUtility::ParseCommaSeparatedDoubles;

    std::string TXScoreboardDA::getName() const
    {
        return m_name;
    }


    ScoreboardStratum TXScoreboardDA::getStratum() const
    {
        return m_stratum;
    }


    BoundaryValueVector TXScoreboardDA::getXBoundaryValueVector() const
    {
        return m_xBoundaries;
    }


    BoundaryValueVector TXScoreboardDA::getYBoundaryValueVector() const
    {
        return m_yBoundaries;
    }


    BoundaryValueVector TXScoreboardDA::getZBoundaryValueVector() const
    {
        return m_zBoundaries;
    }

    void TXScoreboardDA::setName(const std::string& name)
    {
        m_name = name;
    }

    void TXScoreboardDA::setStratum(const std::string& stratum)
    {
        long sval = atol(stratum.c_str());
        m_stratum = sval;
    }

    void TXScoreboardDA::setXBoundaryValueVector(const BoundaryValueVector& boundaries)
    {
        m_xBoundaries = boundaries;
    }

    void TXScoreboardDA::setYBoundaryValueVector(const BoundaryValueVector& boundaries)
    {
        m_xBoundaries = boundaries;
    }

    void TXScoreboardDA::setZBoundaryValueVector(const BoundaryValueVector& boundaries)
    {
        m_xBoundaries = boundaries;
    }

    void TXScoreboardDA::setBoundaryValueVector(const char* csv, Dimension dim)
    {
        char* unconst_csv = new char[strlen(csv) + 1];
        strcpy_s(unconst_csv, strlen(csv) + 1, csv);

        switch (dim)
        {
        case X:
            ParseCommaSeparatedDoubles(unconst_csv, m_xBoundaries);
            break;
        case Y:
            ParseCommaSeparatedDoubles(unconst_csv, m_yBoundaries);
            break;
        case Z:
            ParseCommaSeparatedDoubles(unconst_csv, m_zBoundaries);
            break;
        case NoDimension:
        default:
            RmAssert(false, "Invalid dimension for scoreboard boundary values");
        }

        delete[] unconst_csv;
    }

    bool TXScoreboardDA::isValid() const
    {
        if (m_name.empty()) return false;
        if (m_stratum == ScoreboardStratum::NONE) return false;
        if (m_xBoundaries.empty()) return false;
        if (m_yBoundaries.empty()) return false;
        if (m_zBoundaries.empty()) return false;

        return true;
    }

    TXScoreboardDA::~TXScoreboardDA()
    {
    }


    TXScoreboardDA::TXScoreboardDA()
        : m_name("")
        , m_stratum(ScoreboardStratum::NONE)
    {
    }
} /* namespace rootmap */

