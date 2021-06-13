#ifndef Tracer_H
#define Tracer_H

/////////////////////////////////////////////////////////////////////////////
// Name:        LogHelper.h
// Purpose:     Declaration of the Tracer class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
// Provides a simple class that logs when constructed and destructed.
// Not intended to be used directly.
// Used by the RootMapLogTrace macro in Logger.h to provide tracing.
//
/////////////////////////////////////////////////////////////////////////////


// Forward declarations.
namespace log4cpp
{
    class Category;
}

namespace rootmap
{
    /** Provides the method tracing via debug logs.
     * Logs an entry trace when a Tracer object is created.
     * Logs an exit trace when a Tracer object is deleted,
     * generally when the method containing the Tracer returns
     * and the Tracer falls out of scope.
     *
     * This class is not intended to be used directly.
     * Use the RootMapLogTrace or LOG_TRACE macro in Logger.h instead.
     */
    class Tracer
    {
    public:
        /** the fully qualified constructor */
        Tracer(const char* method,
            log4cpp::Category* logger,
            const void* object,
            const char* filename,
            int line);
        ~Tracer();
    private:
        /** the logger that the trace logs are directed to */
        log4cpp::Category* _logger;
        /** the name of the method being traced */
        const char* _method;
        /** the address of the object instance being traced */
        const void* _object;
        /** the file containing the traced method */
        const char* _filename;
        /** the line number of the beginning of the method being traced */
        int _line;
    };
} /* namespace rootmap */
#endif // #ifndef Tracer_H
