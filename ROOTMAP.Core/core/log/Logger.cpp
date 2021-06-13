/////////////////////////////////////////////////////////////////////////////
// Name:        Logger.cpp
// Purpose:     Implementation of the Logger class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "log4cpp/PropertyConfigurator.hh"
#include "core/log/Logger.h"
#include <iostream>

namespace rootmap
{
    bool Logger::configure(const char* configFileName)
    {
        log4cpp::Category& root = log4cpp::Category::getRoot();
        if (configFileName == 0)
        {
            root.removeAllAppenders();
            root.shutdown();
            return true;
        }
        try
        {
            log4cpp::PropertyConfigurator::configure(configFileName);
        }
        catch (log4cpp::ConfigureFailure& f)
        {
            root.errorStream() << "logging configure failure - " << f.what();
            root.removeAllAppenders();
            root.shutdown();
            return false;
        }
        return true;
    }
} /* namespace rootmap */
