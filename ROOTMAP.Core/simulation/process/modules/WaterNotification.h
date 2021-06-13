#ifndef WaterNotification_H
#define WaterNotification_H
/////////////////////////////////////////////////////////////////////////////
// Name:        WaterNotification.h
// Purpose:     Declaration of the WaterNotification class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/common/Process_Dictionary.h"

namespace rootmap
{
    enum
    {
        kWaterDrained = kGeneralPurposeMessage0,
        kWaterRedistributed = kGeneralPurposeMessage1
    };

//     class WaterNotification
//     {
//     public:
//         WaterNotification();
//         virtual ~WaterNotification();
// 
//     private:
// 
//         ///
//         /// member declaration
//     }; // class WaterNotification

} // namespace rootmap

#endif // #ifndef WaterNotification_H
