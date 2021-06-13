/////////////////////////////////////////////////////////////////////////////
// Name:        BinaryDataOutputFile.cpp
// Purpose:     Implementation of the BinaryDataOutputFile class
// Created:     22/09/2009
// Author:      MSA
// $Date: 2009-09-22 02:07:36 +0800 (Tue, 22 Sep 2009) $
// $Revision: 1 $
// Copyright:   ©2009 University of Tasmania/University of Western Australia
/////////////////////////////////////////////////////////////////////////////
#include "simulation/file/output/BinaryDataOutputFile.h"
#include "simulation/process/interprocess_communication/ProcessActionDescriptor.h"

#include "core/common/Constants.h"
#include "core/common/RmAssert.h"
#include "core/utility/Utility.h"

#include "wx/image.h"


namespace rootmap
{
    BinaryDataOutputFile::BinaryDataOutputFile(DataOutputFile& dof)
        : DataOutputFile(dof)
        , wxDataOutputStream(m_baseStream)
    {
        // This wxDataOutputStream function ensures that we write byte order correctly according to the local architecture
        BigEndianOrdered(!Utility::IsLittleEndian());
    }

    BinaryDataOutputFile::~BinaryDataOutputFile()
    {
    }

    void BinaryDataOutputFile::WriteBitmap(const wxBitmap* bitmap, const wxString& fileExt)
    {
        // MSA D'oh.
        // After all that, turns out wxImage has a SaveFile(wxOutputStream &, int /*type*/) function
        // that would (in theory) work with DataOutputFile, as
        //        wxOutputStream    [abstract]
        //             |
        //            \/
        //        wxTextOutputStream
        //             |
        //            \/
        //        DataOutputFile.
        //
        //    Ah well. That'll teach me to read the API first.
        //    Still, BinaryDataOutputFile may be useful down the track (for writing DF3s perhaps)
        //  so I'll leave it in.

        wxImage img = bitmap->ConvertToImage();

        wxString type = "bmp"; // default is bmp

        if (!fileExt.CmpNoCase("png")) // if(fileExt=="png" || fileExt=="PNG" || fileExt=="PnG"  etc...)
        {
            //            img.AddHandler(&wxPNGHandler());
            type = "png";
        }
        else if (!fileExt.CmpNoCase("jpg") || !fileExt.CmpNoCase("jpeg"))
        {
            wxJPEGHandler handler = wxJPEGHandler();
            img.AddHandler(&handler);
            type = "jpg";
        }
        else if (!fileExt.CmpNoCase("tif") || !fileExt.CmpNoCase("tiff"))
        {
            wxTIFFHandler handler = wxTIFFHandler();
            img.AddHandler(&handler);
            type = "tif";
        }

        img.SaveFile(m_baseStream, type);
    }
} /* namespace rootmap */


