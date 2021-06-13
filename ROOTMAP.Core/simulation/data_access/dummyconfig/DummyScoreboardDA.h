#ifndef DummyScoreboardDA_H
#define DummyScoreboardDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DummyScoreboardDA.h
// Purpose:     Declaration of the DummyScoreboardDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/interface/ScoreboardDAI.h"

namespace rootmap
{
    class DummyScoreboardDA : public ScoreboardDAI
    {
    public:
        virtual std::string getName() const;
        virtual ScoreboardStratum getStratum() const;
        virtual BoundaryValueVector getXBoundaryValueVector() const;
        virtual BoundaryValueVector getYBoundaryValueVector() const;
        virtual BoundaryValueVector getZBoundaryValueVector() const;

        virtual ~DummyScoreboardDA();

        DummyScoreboardDA();
    }; // class DummyScoreboardDA
} /* namespace rootmap */

#endif // #ifndef DummyScoreboardDA_H
