#ifndef TXScoreboardDA_H
#define TXScoreboardDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        TXScoreboardDA.h
// Purpose:     Declaration of the TXScoreboardDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/interface/ScoreboardDAI.h"

namespace rootmap
{
    class TXScoreboardDA : public ScoreboardDAI
    {
    public:

        // //////////////////////
        // Implementation of ScoreboardDAI
        // //////////////////////
        virtual std::string getName() const;
        virtual ScoreboardStratum getStratum() const;
        virtual BoundaryValueVector getXBoundaryValueVector() const;
        virtual BoundaryValueVector getYBoundaryValueVector() const;
        virtual BoundaryValueVector getZBoundaryValueVector() const;

        // //////////////////////
        // Set data
        // //////////////////////
        void setName(const std::string& name);
        void setStratum(const std::string& stratum);
        void setXBoundaryValueVector(const BoundaryValueVector& boundaries);
        void setYBoundaryValueVector(const BoundaryValueVector& boundaries);
        void setZBoundaryValueVector(const BoundaryValueVector& boundaries);
        void setBoundaryValueVector(const char* csv, Dimension dim);

        bool isValid() const;
        // //////////////////////
        // Construction/Destruction
        // //////////////////////
        virtual ~TXScoreboardDA();
        TXScoreboardDA();

    private:
        std::string m_name;
        ScoreboardStratum m_stratum;
        BoundaryValueVector m_xBoundaries;
        BoundaryValueVector m_yBoundaries;
        BoundaryValueVector m_zBoundaries;
    }; // class TXScoreboardDA
} /* namespace rootmap */

#endif // #ifndef TXScoreboardDA_H
