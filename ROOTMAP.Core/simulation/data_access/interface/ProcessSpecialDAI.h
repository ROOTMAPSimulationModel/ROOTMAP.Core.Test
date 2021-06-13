#ifndef ProcessSpecialDAI_H
#define ProcessSpecialDAI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessSpecialDAI.h
// Purpose:     Declaration of the ProcessSpecialDAI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
// This interface encompasses an attempt to provide an interface that meets
// the following goals :
// - sufficiently generic to be applicable to a variety of process data
// - sufficiently rich structures to be able to access meaningful datasets
// - various access implementations can deal with the expected data
//   
// This tries to be met by offering some basic thru complex accessors
// - named lists of floating point
// - named lists of integers
// - named lists of strings
// - list of name-value pairs
// - named blobs
// - does not support arbitrary anonymous lists
/////////////////////////////////////////////////////////////////////////////

#include "core/common/Types.h"

#include <map>
#include <vector>

namespace rootmap
{
    class ProcessSpecialDAI
    {
    public:
        typedef std::map<std::string, double> NameFloatPairs;
        typedef std::map<std::string, long int> NameIntegerPairs;
        typedef std::map<std::string, std::string> NameStringPairs;

        typedef std::vector<double> FloatArray;
        typedef std::vector<long int> IntegerArray;
        typedef std::vector<std::string> StringArray;

        typedef std::vector<Byte> ByteArray;

        /**
         * ThisFunctionName
         *
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */
        virtual const NameFloatPairs& getNameFloatPairs() const = 0;
        virtual const NameIntegerPairs& getNameIntegerPairs() const = 0;
        virtual const NameStringPairs& getNameStringPairs() const = 0;

        virtual const FloatArray& getNamedFloatArray(const std::string& name) const = 0;
        virtual const IntegerArray& getNamedIntegerArray(const std::string& name) const = 0;
        virtual const StringArray& getNamedStringArray(const std::string& name) const = 0;

        virtual const ByteArray& getNamedBlob(const std::string& name) const = 0;

        virtual ~ProcessSpecialDAI()
        {
        }

    protected:
        ProcessSpecialDAI()
        {
        }

    private:

        ///
        /// member declaration
    }; // class ProcessSpecialDAI
} /* namespace rootmap */

#endif // #ifndef ProcessSpecialDAI_H
