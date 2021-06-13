#ifndef PlantFwd_H
#define PlantFwd_H
/////////////////////////////////////////////////////////////////////////////
// Name:        PlantFwd.h
// Purpose:     Declaration of the PlantFwd class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <set>
#include <map>

namespace rootmap
{
    class Plant;
    class PlantType;
    class PlantSummary;
    class PlantSummaryRegistration;

    typedef std::vector< Plant * > PlantArray;
    typedef std::vector< PlantType * > PlantTypeArray;
    typedef std::vector< PlantSummary * > PlantSummaryArray;
    typedef std::multimap<long, PlantSummary*> PlantSummaryMultimap;
    typedef std::vector< PlantSummaryRegistration * > PlantSummaryRegistrationArray;

    typedef std::set< Plant * > PlantSet;
    typedef std::set< PlantType * > PlantTypeSet;
    typedef std::set< PlantSummary * > PlantSummarySet;
    typedef std::set< PlantSummaryRegistration * > PlantSummaryRegistrationSet;
} // namespace rootmap

#endif // #ifndef PlantFwd_H
