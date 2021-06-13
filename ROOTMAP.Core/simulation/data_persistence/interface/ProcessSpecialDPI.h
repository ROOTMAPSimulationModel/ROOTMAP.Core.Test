#ifndef ProcessSpecialDPI_H
#define ProcessSpecialDPI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessSpecialDPI.h
// Purpose:     Declaration of the ProcessSpecialDPI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
// @file Defines the ProcessSpecialDPI interface class.
/////////////////////////////////////////////////////////////////////////////

#include "core/common/Types.h"

#include "simulation/data_access/interface/ProcessSpecialDAI.h"

#include <map>
#include <vector>

namespace rootmap
{
    /**
     * This abstract interface class encompasses an attempt to provide an
     * interface that meets the following goals :
     * - sufficiently generic to be applicable to a variety of process data
     * - sufficiently rich structures to be able to access meaningful datasets
     * - various access implementations can deal with the expected data
     *
     * This tries to be met by offering a variety of structures to store data in:
     * - Arrays of name-value pairs
     * - Named arrays of floating point, integer and string values
     * - Named blobs (array of bytes, but can specify compression and encoding)
     */
    class ProcessSpecialDPI
    {
    public:
        /** Persist pairs of name-float values */
        virtual storeNameFloatPairs(const ProcessSpecialDAI::NameFloatPairs& nameFloatPairs) const = 0;

        /** Persist pairs of name-integer values */
        virtual storeNameIntegerPairs(const ProcessSpecialDAI::NameIntegerPairs& nameIntPairs) const = 0;

        /** Persist pairs of name-string values */
        virtual storeNameStringPairs(const ProcessSpecialDAI::NameStringPairs& nameStrPairs) const = 0;

        /** Persist an array of floating-point values */
        virtual storeNamedFloatArray(const std::string& name, const ProcessSpecialDAI::FloatArray& floatArray) const = 0;

        /** Persist an array of integer values */
        virtual storeNamedIntegerArray(const std::string& name, const ProcessSpecialDAI::IntegerArray& intArray) const = 0;

        /** Persist an array of string values */
        virtual storeNamedStringArray(const std::string& name, const ProcessSpecialDAI::StringArray& stringArray) const = 0;

        /** Persist a "binary large object", a.k.a. an arbitrary array of bytes.  */
        virtual storeNamedBlob(const std::string& name, const ProcessSpecialDAI::ByteArray& blobArray) const = 0;

        virtual ~ProcessSpecialDPI()
        {
        }

    protected:
        ProcessSpecialDPI()
        {
        }

    private:

        ///
        /// member declaration
    }; // class ProcessSpecialDPI
} /* namespace rootmap */

#endif // #ifndef ProcessSpecialDPI_H
