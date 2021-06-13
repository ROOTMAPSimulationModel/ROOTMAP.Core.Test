/////////////////////////////////////////////////////////////////////////////
// Name:        OutputRule.cpp
// Purpose:     Implementation of the OutputRule class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-10 06:43:25 +0800 (Fri, 10 Jul 2009) $
// $Revision: 69 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/file/output/OutputRule.h"
#include "simulation/file/output/OutputStrategy.h"
#include "simulation/file/output/DataOutputFile.h"
#include "simulation/file/output/OutputTimingStrategy.h"


#include "simulation/common/SimulationEngine.h"
#include "simulation/data_access/interface/OutputRuleDAI.h"
#include "simulation/process/interprocess_communication/ProcessActionDescriptor.h"

#include "core/common/Exceptions.h"

#include <wx/wfstream.h>
#include <wx/regex.h>
#include <wx/datetime.h>

#include <stack>

#define WXFILETYPE WXFILE

//#define WXFILETYPE WXFFILE
namespace rootmap
{
    OutputRule::OutputRule(const OutputRuleDAI& data, SimulationEngine& engine)
        : m_type(data.GetType().c_str())
        , m_source_name(data.GetProcessName().c_str())
        , m_source_variation(data.GetName().c_str())
        , m_source_stratum(data.GetStratum())
        , m_outputTimingStrategy(__nullptr)
        , m_fileNameFormat(data.GetFileName().c_str())
        , m_lastFileName("")
        , m_reopen_strategy(overwrite)
        , m_outputStrategy(__nullptr)
        , m_now(0)
        , m_destination_file(0)
        , m_simulationEngine(engine)
        , m_wxFileType(JustFile)
        , m_directory(data.GetDirectory().c_str())
    {
        try
        {
            m_outputTimingStrategy = OutputTimingStrategyFactory::createOutputTimingStrategy(data, engine);
        }
        catch (RmException& /*rme*/)
        {
            delete m_outputTimingStrategy;
            throw;
        }

        /**
         * MSA Note: I put null-pointer checks around all subsequent references to m_outputStrategy.
         * This is because I used the createOutputStrategy() method's ability to return a null ptr
         * gracefully (only a logged warning) in the event of creation failure to provide a way to
         * "suspend" raytracer output. See comments in OutputRules.xml for more information.
         *
         * This was just a convenience thing - allows users (or developers) to switch raytracer
         * output off, for increased speed or to avoid overwriting/appending to old .POV files,
         * without having to remove or comment out the whole output rule.
         * If it's too kludgy, feel free to remove it.
         */
        try
        {
            m_outputStrategy = OutputStrategyFactory::createOutputStrategy(data, engine);
        }
        catch (RmException& /*rme*/)
        {
            delete m_outputTimingStrategy;
            delete m_outputStrategy;
            throw;
        }

        if ("append" == data.GetReopenStrategy())
        {
            m_reopen_strategy = append;
        }
    }

    OutputRule::~OutputRule()
    {
        delete m_outputTimingStrategy;
        delete m_outputStrategy;
        delete m_destination_file;
    }

    void OutputRule::UpdateTime(ProcessTime_t t)
    {
        m_outputTimingStrategy->UpdateTime(t);
    }

    void TestFileExistence(const wxString& absoluteFilePath)
    {
        // Warning: Windows-specific
        DWORD fileAttributes = GetFileAttributes(absoluteFilePath.wc_str());
        // If file doesn't exist, try creating it.
        if (fileAttributes == INVALID_FILE_ATTRIBUTES)
        {
            // First, ensure the directory exists...
            wxString workingPath = absoluteFilePath.BeforeLast('\\');
            std::stack<wxString> subdirs;
            while (true)
            {
                DWORD dirAttributes = GetFileAttributes(workingPath.wc_str());
                if (dirAttributes != INVALID_FILE_ATTRIBUTES)
                {
                    // Most deeply-nested existing directory found.
                    break;
                }
                wxString subdir = workingPath.AfterLast('\\');
                if (subdir == workingPath)
                {
                    if (GetFileAttributes(workingPath.wc_str()) == INVALID_FILE_ATTRIBUTES)
                    {
                        throw RmException("Bad directory " + workingPath);
                    }
                    break;
                }
                subdirs.push(subdir);
                workingPath = workingPath.BeforeLast('\\');
            }

            while (!subdirs.empty())
            {
                wxString subdir = subdirs.top();
                workingPath << '\\' << subdir << '\\';
                if (!CreateDirectory(workingPath.wc_str(), __nullptr))
                {
                    throw RmException("Could not create subdirectory " + workingPath);
                }
                subdirs.pop();
            }
            HANDLE h = CreateFile(absoluteFilePath.wc_str(), // name of the file
                GENERIC_WRITE, // open for writing
                0, // sharing mode, none in this case
                0, // use default security descriptor
                CREATE_NEW,
                FILE_ATTRIBUTE_NORMAL,
                0);
            if (h != INVALID_HANDLE_VALUE)
            {
                CloseHandle(h);
            }
            else
            {
                wxString errString = "Could not find or create file ";
                errString << absoluteFilePath << "\n" << GetLastError();
                throw RmException(errString);
            }
        }
    }

    DataOutputFile* OutputRule::MakeFile(const wxString& filePath)
    {
        wxOutputStream* ostr = __nullptr;

        //
        // currently using wxFile and wxFileOutputStream. This is because the
        // latter class is buffered. The only way to open one of these in append
        // mode is to make a wxFile with that attribute and pass it to the
        // wxFileOutputStream
        if (JustFile == m_wxFileType)
        {
            wxFile::OpenMode mode = wxFile::write;
            if (append == m_reopen_strategy)
            {
                mode = wxFile::write_append;
            }
            // MSA 10.02.04 This line seems to leak 8 bytes every time. Why?
            wxFile* file = new wxFile(filePath.c_str(), mode);
            // And this one leaks 20 bytes per.
            ostr = new wxFileOutputStream(*file);
        }
        else
        {
            wxString mode = "w+b";
            if (append == m_reopen_strategy)
            {
                mode = "a+b";
            }
            ostr = new wxFFileOutputStream(filePath.c_str(), mode);
        }

        DataOutputFile* data_file = new DataOutputFile(*ostr, filePath);
        if (0 == data_file)
        {
            throw RmException("Could not create output file");
        }

        return (data_file);
    }

    void OutputRule::MakeFileName(wxString& s)
    {
        wxString file_name(m_fileNameFormat);
        file_name.Replace("/", "\\"); // Ensure path is all backslashes, no forward slashes

        // pre-scan the format string for special non-strftime indicators that we
        // handle internally
        //
        // A format specification, which consists of optional and required fields, has the following form:
        // %[flags] [width] [.precision] [{h | l | ll | I | I32 | I64}]type
        // 
        // However:
        // * we are only interested in type=='i' or type=='d'
        // * our count format will not support any of the hlI32 extensions
        //
        // Tested with the assistance of wxWidgets-2.6.2/samples/console/console.cpp
        wxRegEx regex("%[-+0 #]?[0-9]*(\\.[0-9]*)?[iu]");
        if (regex.Matches(m_fileNameFormat))
        {
            // get the format specifier to use in printf
            wxString match = regex.GetMatch(m_fileNameFormat);

            // wxString::Format does an as-safe-as-possible printf-ish thing
            wxString count_str = wxString::Format(match.c_str(), m_outputTimingStrategy->GetCurrentCount());

            // use the newly formatted count string to replace the format specifier
            /*int num_matches = */
            regex.Replace(&file_name, count_str);
        }

        // Replacement for previous char[] code
        wxDateTime current_timestamp(m_now);
        s = current_timestamp.Format(file_name.c_str());

        // If directory has been provided, assume the filename to be relative.
        if (!m_directory.empty())
        {
            s = m_directory + "\\" + s;
        }

        TestFileExistence(s);
    }


    ProcessTime_t OutputRule::GetNextTime(ProcessTime_t t)
    {
        return (m_outputTimingStrategy->GetNextTime(t));
    }

    // Function :        DoThisTime
    // 
    // Description :    checks various data members to assess whether we have been set up
    //                to export at this timestamp
    // 
    // Working Notes :    checks if a) we export once, and this is that once, b) if we export
    //                multiple occurrences and this is one of them and c) if we've exported
    //                already during this timestamp
    // 
    // Input Paramaters :
    //                t -    the time we are being asked to account for
    // 
    // Return Value :    "true" if we are expecting to export/output at this time
    // 
    // Modifications :
    // 1999.03.06        check to see if we've already been asked to export during this timestamp
    bool OutputRule::DoesThisTime(ProcessTime_t t)
    {
        return (m_outputTimingStrategy->DoesThisTime(t));
    }


    //
    // Function:
    //  ArrangeOutput
    //
    //
    // Description:
    //  Requests of the OutputStrategy to Arrange for Output. As an example,
    //  ScoreboardDataOutputStrategy sends a kSpecialOutputDataMessage to the
    //  process. This is sent to the process at the end of the current timestamp,
    //  and causes the process to call OutputRule->OutputToFile() at that instant.
    //
    // Working Notes:
    //  Called by OutputFileAlarm::InvokeOutputRules.
    //
    void OutputRule::ArrangeOutput(ProcessActionDescriptor* action)
    {
        if (0 != m_outputStrategy)
        {
            m_now = action->GetTime();
            m_outputStrategy->ArrangeOutput(action, this);
        }
    }

    // 
    // Called whenever the protocol decided by the strategy requests. In the case
    // ScoreboardData, the Process::DoSpecialOutput calls this
    //
    // Note that making the filename and file is Just-In-Time
    void OutputRule::OutputToFile(ProcessActionDescriptor* action)
    {
        if (0 != m_outputStrategy)
        {
            m_now = action->GetTime();
            wxString filename;
            MakeFileName(filename);

            // TODO: Ensure directory exists.


            // it is assumed that if the current filename is the same as the last,
            // there is no need to remake the file itself
            if ((0 == m_destination_file)
                || (filename != m_lastFileName)
                || (overwrite == m_reopen_strategy)
                )
            {
                delete m_destination_file;
                m_destination_file = __nullptr;
                m_destination_file = MakeFile(filename);
            }
            else
            {
            }

            if (__nullptr == m_destination_file)
            {
                throw RmException("OutputRule destination file is NULL");
            }
            m_outputStrategy->OutputToFile(*m_destination_file);

            m_outputTimingStrategy->IncrementCount();
            m_lastFileName = filename;

            delete m_destination_file;
            m_destination_file = 0;
        } // end if(0!=m_outputStrategy)
    }
} /* namespace rootmap */

