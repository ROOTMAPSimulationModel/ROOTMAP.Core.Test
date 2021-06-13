/////////////////////////////////////////////////////////////////////////////
// Name:        Dimension.cpp
// Purpose:     Implementation of Dimension
// Created:     2003
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/scoreboard/Dimension.h"

#include <sstream>

namespace rootmap
{
    std::ostream& operator<<(std::ostream& ostr, Dimension dimension)
    {
        switch (dimension)
        {
        case X: ostr << "X";
            break;
        case Y: ostr << "Y";
            break;
        case Z: ostr << "Z";
            break;
        case NoDimension:
        default: ostr << "?";
        }
        return ostr;
    }
} /* namespace rootmap */
