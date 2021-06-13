#ifndef Logger_H
#define Logger_H
/////////////////////////////////////////////////////////////////////////////
// Name:        Logger.h
// Purpose:     Declaration of the Logger class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
/** @file Logger.h
 *
 * Generic logging facility macros and Logger class declaration.
 *
 * Provides macro functions that accept streamable objects for logging.
 * Helper macros are provided to aid in adding the logger attribute to classes. * Macro functions are used so logging can be compiled in or out as necessary.
 *
 * Logging priorities are:
 * -# Trace
 * -# Debug
 * -# Info
 * -# Notice
 * -# Warn
 * -# Error
 * -# Crit
 * -# Alert
 * -# Fatal
 *
 * Enabling logging at compile time involves setting ROOTMAP_LOG_LEVEL to one of these levels.
 * Enabling a given priority implictly enables all higher priorities.
 * The special value ROOTMAP_LOG_LEVEL_DISABLE can be set to disable all logging.
 *
 * Note that the logging facility must still be configures at runtime,
 * either via a configuration file passed to RootMapLoggerConfigure()
 * or via hard coded calls to log4cpp.  The former is preferred since
 * hard coded calls will override any settings made via the configuration
 * file.
 */
 /////////////////////////////////////////////////////////////////////////////

#define ROOTMAP_LOG_LEVEL_TRACE  1
#define ROOTMAP_LOG_LEVEL_DEBUG  2
#define ROOTMAP_LOG_LEVEL_INFO   3
#define ROOTMAP_LOG_LEVEL_NOTICE 4
#define ROOTMAP_LOG_LEVEL_WARN   5
#define ROOTMAP_LOG_LEVEL_ERROR  6
#define ROOTMAP_LOG_LEVEL_CRIT   7
#define ROOTMAP_LOG_LEVEL_ALERT  8
#define ROOTMAP_LOG_LEVEL_FATAL  9
#define ROOTMAP_LOG_LEVEL_DISABLE 10


#define ROOTMAP_LOG_LEVEL ROOTMAP_LOG_LEVEL_DEBUG


// If NONE is set then log nothing
#ifdef ROOTMAP_LOG_NONE
#undef ROOTMAP_LOG_LEVEL
#define ROOTMAP_LOG_LEVEL ROOTMAP_LOG_LEVEL_DISABLE
#endif

// If ALL is set then log everything.  
// Note that this overrides LOG_NONE.
#ifdef ROOTMAP_LOG_ALL
#undef ROOTMAP_LOG_LEVEL
#define ROOTMAP_LOG_LEVEL ROOTMAP_LOG_LEVEL_TRACE
#endif

// Set the default log level, if not already set
#ifndef ROOTMAP_LOG_LEVEL
#define ROOTMAP_LOG_LEVEL ROOTMAP_LOG_LEVEL_DISABLE
#endif

// Now setup all the helper macros based on the log level.
/** Creates a Trace object that logs the entry into and exit from a method.
 *  Should be the first statement in the method.
 *  @param method is the name of the method as a string, e.g. "ClassName::FunctionName".
 */
#if ROOTMAP_LOG_LEVEL <= ROOTMAP_LOG_LEVEL_TRACE
#define RootMapLogTrace(method)   rootmap::Tracer _tracer(method,_logger, (const void*)this,__FILE__,__LINE__)  
#include "core/log/Tracer.h"
#else 
#define RootMapLogTrace(method)
#endif

 /** Logs the msg at debug priority.
  *  @param msg is a streamable object, or list or streamable objects
  *  e.g. a << b.
  *  To facilitate compiling out of logging code, no preparation of the
  *  objects to be streamed should be performed before the call.
  *  If preparation is absolutely necessary it should be wrapped as follows:
  *  - \#if ROOTMAP_LOG_LEVEL <= ROOTMAP_LOG_LEVEL_DEBUG
  *  - .... prepcode ....
  *  - \#endif
  */
#define LINECOMMENT /##/
#if ROOTMAP_LOG_LEVEL <= ROOTMAP_LOG_LEVEL_DEBUG 
#define RootMapLogDebug(msg)   _logger->debugStream() << msg
#define LOG_DEBUG              _logger->debugStream()
#else 
#define RootMapLogDebug(msg)
#define LOG_DEBUG LINECOMMENT
#endif

  /** Logs the msg at info priority.
   *  @see RootMapLogDebug.
   */
#if ROOTMAP_LOG_LEVEL <= ROOTMAP_LOG_LEVEL_INFO
#define RootMapLogInfo(msg)   _logger->infoStream() << msg
#define LOG_INFO              _logger->infoStream()
#else 
#define RootMapLogInfo(msg)
#define LOG_INFO LINECOMMENT
#endif

   /** Logs the msg at notice priority.
    *  @see RootMapLogDebug.
    */
#if ROOTMAP_LOG_LEVEL <= ROOTMAP_LOG_LEVEL_NOTICE
#define RootMapLogNotice(msg)   _logger->noticeStream() << msg
#define LOG_NOTE                _logger->noticeStream()
#else 
#define RootMapLogNotice(msg)
#define LOG_NOTE LINECOMMENT
#endif

    /** Logs the msg at warn priority.
     *  @see RootMapLogDebug.
     */
#if ROOTMAP_LOG_LEVEL <= ROOTMAP_LOG_LEVEL_WARN
#define RootMapLogWarn(msg)   _logger->warnStream() << msg
#define LOG_WARN              _logger->warnStream()
#else 
#define RootMapLogWarn(msg)
#define LOG_WARN LINECOMMENT
#endif

     /** Logs the msg at error priority.
      *  @see RootMapLogDebug.
      */
#if ROOTMAP_LOG_LEVEL <= ROOTMAP_LOG_LEVEL_ERROR
#define RootMapLogError(msg)   _logger->errorStream() << msg
#define LOG_ERROR              _logger->errorStream()
#else 
#define RootMapLogError(msg)
#define LOG_ERROR LINECOMMENT
#endif

      /** Logs the msg at crit priority.
       *  @see RootMapLogDebug.
       */
#if ROOTMAP_LOG_LEVEL <= ROOTMAP_LOG_LEVEL_CRIT
#define RootMapLogCrit(msg)   _logger->critStream() << msg
#define LOG_CRIT              _logger->critStream()
#else 
#define RootMapLogCrit(msg)
#define LOG_CRIT LINECOMMENT
#endif

       /** Logs the msg at alert priority.
        *  @see RootMapLogDebug.
        */
#if ROOTMAP_LOG_LEVEL <= ROOTMAP_LOG_LEVEL_ALERT
#define RootMapLogAlert(msg)   _logger->alertStream() << msg
#define LOG_ALERT              _logger->alertStream()
#else 
#define RootMapLogAlert(msg)
#define LOG_ALERT LINECOMMENT
#endif

        /** Logs the msg at fatal priority.
         *  @see RootMapLogDebug.
         */
#if ROOTMAP_LOG_LEVEL <= ROOTMAP_LOG_LEVEL_FATAL
#define RootMapLogFatal(msg)   _logger->fatalStream() << msg
#define LOG_FATAL              _logger->fatalStream()
#define ROOTMAP_LOGGING 1
#else 
#undef ROOTMAP_LOGGING
#define RootMapLogFatal(msg)
#define LOG_FATAL LINECOMMENT
#endif

         /** Declares the logger attribute used by RootMapLog macros.
          *  Should be placed as the first attribute of the class.
          */
#ifdef ROOTMAP_LOGGING
#define RootMapLoggerDeclaration()   static log4cpp::Category * _logger

          /** Enable LOG4CPP fix for namespace collision for 'DEBUG' and 'ERROR'.
           *  See log4cpp FAQ for details.
           */
#define LOG4CPP_FIX_ERROR_COLLISION 1
           // including wx/wx.h so that windows.h doesn't stuff wx later, when its is
           // included way down in log4cpp/threads/MSThreads.hh
#include "wx/wx.h"
#include "log4cpp/Category.hh"
#else
#define RootMapLoggerDeclaration()   
#endif

          /** Defines the logger attribute used by RootMapLog macros.
           *  Should be placed at the beginning of the .cc file just before the constructors.
           */
#ifdef ROOTMAP_LOGGING
#define RootMapLoggerDefinition(class) log4cpp::Category * class::_logger = 0
#else
#define RootMapLoggerDefinition(class)   
#endif

           /** Initialises the logger attribute used by RootMapLog macros.
            *  Must be the first statement in all constructors.
            *  Must be called before any RootMapLog macros can be used.
            */
#ifdef ROOTMAP_LOGGING
#define RootMapLoggerInitialisation(name)   if (_logger == 0) _logger = &(log4cpp::Category::getInstance(name))
#else
#define RootMapLoggerInitialisation(name)
#endif

            /** Configures the logging facility using the given configuration file.
             *  Should be used in the main before any logging is performed.
             *  Therefore it should be called before any classes that use logging are created.
             */
#ifdef ROOTMAP_LOGGING
#define RootMapLoggerConfigure(configFileName) rootmap::Logger::configure(configFileName)
#else
#define RootMapLoggerConfigure(configFileName)
#endif


#define LOG_LINE "(" << __LINE__ << ") "


namespace rootmap
{
    /** Provides the interface to configure the logging facility.
     * Not to be used directly - should only be used via the helper macro RootMapLoggerConfigure().
     *
     */
    class Logger
    {
    public:
        /** Configure the logging facility using the given configuration file.
         * Not to be used directly - should only be used via the helper macro RootMapLoggerConfigure().
         */
        static bool configure(const char* configFile);
    };
} /* namespace rootmap */

#endif // #ifndef Logger_H
