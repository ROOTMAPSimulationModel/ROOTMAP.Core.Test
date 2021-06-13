#ifndef Dimension_H
#define Dimension_H
/////////////////////////////////////////////////////////////////////////////
// Name:        Dimension.h
// Purpose:     
// Created:     DD/MM/YYYY HH:MM:SS
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
//#include "core/common/Types.h"
//
#include <iosfwd>

namespace rootmap
{
    enum Dimension
    {
        X = 0,
        Y = 1,
        Z = 2,

        NoDimension = -1
    };


    const int NumberOfDimensions = 3;


    std::ostream& operator<<(std::ostream& ostr, Dimension dimension);
} /* namespace rootmap */


#endif // #ifndef Dimension_H
