#ifndef TXProcessSpecialDA_H
#define TXProcessSpecialDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        TXProcessSpecialDA.h
// Purpose:     Declaration of the TXProcessSpecialDA class
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
#include "simulation/data_access/interface/ProcessSpecialDAI.h"
#include "core/log/Logger.h"

namespace rootmap
{
    class TXProcessSpecialDA : public ProcessSpecialDAI
    {
    public:
        //typedef std::map<std::string, double> NameFloatPairs;
        //typedef std::map<std::string, long int>  NameIntegerPairs;
        //typedef std::map<std::string, std::string>  NameStringPairs;
        //typedef std::vector<double> FloatArray;
        //typedef std::vector<long int> IntegerArray;
        //typedef std::vector<std::string> StringArray;
        //typedef std::vector<Byte> ByteArray;


        // ///////////////////////////////////////
        //
        //  Overridden ProcessSpecialDAI Accessors
        //
        // ///////////////////////////////////////
        const ProcessSpecialDAI::NameFloatPairs& getNameFloatPairs() const;
        const ProcessSpecialDAI::NameIntegerPairs& getNameIntegerPairs() const;
        const ProcessSpecialDAI::NameStringPairs& getNameStringPairs() const;
        const ProcessSpecialDAI::FloatArray& getNamedFloatArray(const std::string& name) const;
        const ProcessSpecialDAI::IntegerArray& getNamedIntegerArray(const std::string& name) const;
        const ProcessSpecialDAI::StringArray& getNamedStringArray(const std::string& name) const;
        const ProcessSpecialDAI::ByteArray& getNamedBlob(const std::string& name) const;

        // ///////////////////////////////////////
        //
        //  Mutators - used to set the values found in the XML file
        //
        // ///////////////////////////////////////
        void addNameFloatPair(const std::string& name, double value);
        void addNameIntegerPair(const std::string& name, long int value);
        void addNameStringPair(const std::string& name, const std::string& value);
        void addNamedFloatArray(const ProcessSpecialDAI::FloatArray& farray, const std::string& name);
        void addNamedIntegerArray(const ProcessSpecialDAI::IntegerArray& iarray, const std::string& name);
        void addNamedStringArray(const ProcessSpecialDAI::StringArray& sarray, const std::string& name);
        void addNamedBlob(const ProcessSpecialDAI::ByteArray& blob, const std::string& name);

        ~TXProcessSpecialDA();
        TXProcessSpecialDA();

    private:
        RootMapLoggerDeclaration();

        ///
        /// member declaration
        ProcessSpecialDAI::NameFloatPairs m_floatPairs;
        ProcessSpecialDAI::NameIntegerPairs m_integerPairs;
        ProcessSpecialDAI::NameStringPairs m_stringPairs;

        /// dangerous stuff - vectors by pointer in a list
        std::map<std::string, ProcessSpecialDAI::FloatArray*> m_floatArrays;
        std::map<std::string, ProcessSpecialDAI::IntegerArray*> m_integerArrays;
        std::map<std::string, ProcessSpecialDAI::StringArray*> m_stringArrays;
        std::map<std::string, ProcessSpecialDAI::ByteArray*> m_blobs;
    }; // class TXProcessSpecialDA
} /* namespace rootmap */

#endif // #ifndef TXProcessSpecialDA_H
