//
// Filename:
//  ProcessUtility.h
//
// Author:
//  Robert van Hugten
//
// Description:
//  Contains utility functions for Processes, applicable to many classes.
//
//
// Working Notes:
//
//
//
// Modifications:
// 
#ifndef ProcessUtility_H
#define ProcessUtility_H
#include "core/macos_compatibility/macos_compatibility.h"
#include "core/common/Exceptions.h"
#include "core/log/Logger.h"
#include <cmath>
//#include <boost/stacktrace.hpp>

namespace rootmap
{
    namespace ProcessUtility
    {
        inline static double guardedPow(double base, double exponent, log4cpp::Category * _logger)
        {
            double result = pow(base, exponent);
            if (isnan(result) != 0)
            {
                std::string msg = "Attempted to raise " + std::to_string(base) + " to the power of " + std::to_string(exponent) + "; result was NaN.";
                _logger->error(msg);
                //_logger->error(boost::stacktrace::stacktrace()); TODO in new ROOTMAP.
                throw new RmException(msg.c_str());
            }
            return result;
        }
    } // namespace ProcessUtility
} /* namespace rootmap */

#endif // #ifndef ProcessUtility_H
