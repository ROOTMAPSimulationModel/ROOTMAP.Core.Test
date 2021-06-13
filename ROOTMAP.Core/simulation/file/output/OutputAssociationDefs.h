#ifndef OutputAssociationDefs_H
#define OutputAssociationDefs_H
/////////////////////////////////////////////////////////////////////////////
// Name:        OutputAssociationDefs.h
// Purpose:     Declaration of the OutputAssociationDefs class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

namespace rootmap
{
    class DataOutputFile;
    
    // WhenToExport
    typedef enum WhenToExport
    {
        wte_None=0,
        wte_Inititialisation,
        wte_Start_First,
        wte_Start,
        wte_End_First,
        wte_End,
        wte_End_Last,
        wte_Termination,
        wte_Timestamp
    };
    // typedef long int WhenToExport;

    // Multiple File Formatting Extras Rules
    typedef enum MultiFileNameFormat
    {
        None        = 0x00000000,
        Append      = 0x0000000F,
        Prepend     = 0x000000F0,
        Counter     = 0x00000F00,
        Timestamp   = 0x0000F000,
        Append_Same_File = None
    };
    // typedef long int MultiFileExtraRule;

    // 
    enum {
        mfef_None=0,
        mfef_RawTimestamp,
        mfef_Year,
        mfef_Month,
        mfef_Day,
        mfef_Hour,
        mfef_Minutes,
        mfef_Seconds,
        mfef_LastField = mfef_Seconds
    };
    // typedef char MultiFileExtraField;

    // 
    struct OutputFileInformation
    {
        RmString ofi_specifications1;
        RmString ofi_specifications2;
        DataOutputFile * ofi_outfile;
    };

} // namespace rootmap

#endif // #ifndef OutputAssociationDefs_H
