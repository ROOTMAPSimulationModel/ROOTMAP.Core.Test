/*
    Exceptions.h

    Author:            Robert van Hugten
    Description:    <describe the ProcessException class here>
*/

#ifndef ProcessExceptions_H
#define ProcessExceptions_H

#include <exception>
#include "core/macos_compatibility/macos_compatibility.h"
#include "core/common/Exceptions.h"

namespace rootmap
{
    class ProcessException : public RmException
    {
    public:
        ProcessException(const char* reason, const char* processName);
        ProcessException(const ProcessException&);

        virtual ~ProcessException()
        {
        }

        virtual const char* getProcessName() const;


    private:
        std::string m_processName;

        ProcessException& operator=(const ProcessException&);
    };

    inline const char* ProcessException::getProcessName() const
    {
        return m_processName.c_str();
    }
} /* namespace rootmap */

#endif    // ProcessExceptions_H
