/////////////////////////////////////////////////////////////////////////////
// Name:        DataOutputFile.cpp
// Purpose:     Implementation of the DataOutputFile class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006-2007 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/file/output/DataOutputFile.h"
//#include "simulation/file/output/OutputAssociation.h"
#include "simulation/process/interprocess_communication/ProcessActionDescriptor.h"

#include "core/common/Constants.h"
#include "core/common/RmAssert.h"
//#include "core/log/ErrorCollection.h"
#include "core/utility/Utility.h"

// #include <stdio.h>
#include <strstream>
#include <stdexcept>


namespace rootmap
{
    DataOutputFile::DataOutputFile(wxOutputStream& s, const wxString& filename)
        : wxTextOutputStream(s)
        , m_filename(filename)
        , m_baseStream(s)
    {
    }

    // MSA 22.09.09 Adding copy constructor
    DataOutputFile::DataOutputFile(DataOutputFile& rhs)
        : wxTextOutputStream(rhs.m_baseStream)
        , m_filename(rhs.m_filename)
        , m_baseStream(rhs.m_baseStream)
    {
    }

    DataOutputFile::DataOutputFile(const wxString& filename, const DataOutputFile& file)
        : wxTextOutputStream(file.m_baseStream)
        , m_filename(filename)
        , m_baseStream(file.m_baseStream)
    {
    }

    DataOutputFile::~DataOutputFile()
    {
    }

#if 0
    void DataOutputFile::WriteString(const std::string & s)
    {
        wxString wxs(s.c_str());
        wxTextOutputStream::WriteString(wxs);
    }

    void DataOutputFile::WriteString(const char *s)
    {
        wxString wxs(s);
        wxTextOutputStream::WriteString(wxs);
    }

    void DataOutputFile::Write32D(long l, const char * trailing_delimiter)
    {
        wxTextOutputStream::Write32(l);
        wxTextOutputStream::WriteString(trailing_delimiter);
    }
#endif // #if 0

    void DataOutputFile::WriteDoubleFormat(double d, const char* format)
    {
        wxString str;

        str.Printf(format, d);
        WriteString(str);
    }
} /* namespace rootmap */


