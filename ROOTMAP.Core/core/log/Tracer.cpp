/////////////////////////////////////////////////////////////////////////////
// Name:        Tracer.cpp
// Purpose:     Implementation of the Tracer class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "log4cpp/Category.hh"
#include "core/log/Tracer.h"

namespace rootmap
{
    Tracer::Tracer
    (const char* method,
        log4cpp::Category* logger,
        const void* object,
        const char* filename,
        int line)
        : _logger(logger)
        , _method(method)
        , _object(object)
        , _filename(filename)
        , _line(line)
    {
        _logger->debugStream() << "ObjPtr(" << _object << ") TRACE --> " << _method << " in " << _filename << " line " << _line;
    }

    Tracer::~Tracer()
    {
        _logger->debugStream() << "ObjPtr(" << _object << ") TRACE <-- " << _method << " in " << _filename << " near line " << _line;
    }
} /* namespace rootmap */
