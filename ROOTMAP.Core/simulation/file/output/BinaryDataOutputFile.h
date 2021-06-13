#ifndef BinaryDataOutputFile_H
#define BinaryDataOutputFile_H

/////////////////////////////////////////////////////////////////////////////
// Name:        BinaryDataOutputFile.cpp
// Purpose:     Implementation of the BinaryDataOutputFile class
// Created:     22/09/2009
// Author:      MSA
// $Date: 2009-09-22 02:07:36 +0800 (Tue, 22 Sep 2009) $
// $Revision: 1 $
// Copyright:   ©2009 University of Tasmania/University of Western Australia
/////////////////////////////////////////////////////////////////////////////

#include "simulation/file/output/DataOutputFile.h"
#include "wx/datstrm.h"
#include "wx/bitmap.h"

namespace rootmap
{
    class ProcessActionDescriptor;

    class BinaryDataOutputFile : public DataOutputFile
        , public wxDataOutputStream
    {
    public:
        // construction & destruction
        BinaryDataOutputFile(DataOutputFile& dof);
        virtual ~BinaryDataOutputFile();

        void WriteBitmap(const wxBitmap* bitmap, const wxString& fileExt);
    };
} /* namespace rootmap */

#endif // #ifndef BinaryDataOutputFile_H
