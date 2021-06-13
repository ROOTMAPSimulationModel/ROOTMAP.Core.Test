/////////////////////////////////////////////////////////////////////////////
// Name:        TXProcessSpecialDA.cpp
// Purpose:     Implementation of the TXProcessSpecialDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/tinyxmlconfig/TXProcessSpecialDA.h"
#include "core/common/Exceptions.h"


namespace rootmap
{
    RootMapLoggerDefinition(TXProcessSpecialDA);

    TXProcessSpecialDA::TXProcessSpecialDA()
    {
        RootMapLoggerInitialisation("rootmap.TXProcessSpecialDA");
    }

    TXProcessSpecialDA::~TXProcessSpecialDA()
    {
        for (std::map<std::string, ProcessSpecialDAI::FloatArray*>::iterator iter1 = m_floatArrays.begin();
            iter1 != m_floatArrays.end(); ++iter1)
        {
            delete iter1->second;
        }
        for (std::map<std::string, ProcessSpecialDAI::IntegerArray*>::iterator iter2 = m_integerArrays.begin();
            iter2 != m_integerArrays.end(); ++iter2)
        {
            delete iter2->second;
        }
        for (std::map<std::string, ProcessSpecialDAI::StringArray*>::iterator iter3 = m_stringArrays.begin();
            iter3 != m_stringArrays.end(); ++iter3)
        {
            delete iter3->second;
        }
        for (std::map<std::string, ProcessSpecialDAI::ByteArray*>::iterator iter4 = m_blobs.begin();
            iter4 != m_blobs.end(); ++iter4)
        {
            delete iter4->second;
        }
    }

    const ProcessSpecialDAI::NameFloatPairs& TXProcessSpecialDA::getNameFloatPairs() const
    {
        return m_floatPairs;
    }

    const ProcessSpecialDAI::NameIntegerPairs& TXProcessSpecialDA::getNameIntegerPairs() const
    {
        return m_integerPairs;
    }

    const ProcessSpecialDAI::NameStringPairs& TXProcessSpecialDA::getNameStringPairs() const
    {
        return m_stringPairs;
    }

    const ProcessSpecialDAI::FloatArray& TXProcessSpecialDA::getNamedFloatArray(const std::string& name) const
    {
        std::map<std::string, ProcessSpecialDAI::FloatArray*>::const_iterator fnd = m_floatArrays.find(name);
        if (fnd != m_floatArrays.end())
        {
            return (*((*fnd).second));
        }
        else
        {
            throw new RmException("Could not find named FloatArray");
        }
    }

    const ProcessSpecialDAI::IntegerArray& TXProcessSpecialDA::getNamedIntegerArray(const std::string& name) const
    {
        std::map<std::string, ProcessSpecialDAI::IntegerArray*>::const_iterator fnd = m_integerArrays.find(name);
        if (fnd != m_integerArrays.end())
        {
            return (*((*fnd).second));
        }
        else
        {
            throw new RmException("Could not find named IntegerArray");
        }
    }

    const ProcessSpecialDAI::StringArray& TXProcessSpecialDA::getNamedStringArray(const std::string& name) const
    {
        std::map<std::string, ProcessSpecialDAI::StringArray*>::const_iterator fnd = m_stringArrays.find(name);
        if (fnd != m_stringArrays.end())
        {
            return (*((*fnd).second));
        }
        else
        {
            throw new RmException("Could not find named StringArray");
        }
    }

    const ProcessSpecialDAI::ByteArray& TXProcessSpecialDA::getNamedBlob(const std::string& name) const
    {
        std::map<std::string, ProcessSpecialDAI::ByteArray*>::const_iterator fnd = m_blobs.find(name);
        if (fnd != m_blobs.end())
        {
            return (*((*fnd).second));
        }
        else
        {
            throw new RmException("Could not find named ByteArray");
        }
    }

    void TXProcessSpecialDA::addNameFloatPair(const std::string& name, double value)
    {
        m_floatPairs[name] = value;
    }

    void TXProcessSpecialDA::addNameIntegerPair(const std::string& name, long int value)
    {
        m_integerPairs[name] = value;
    }

    void TXProcessSpecialDA::addNameStringPair(const std::string& name, const std::string& value)
    {
        m_stringPairs[name] = value;
    }

    void TXProcessSpecialDA::addNamedFloatArray(const ProcessSpecialDAI::FloatArray& farray, const std::string& name)
    {
        std::map<std::string, ProcessSpecialDAI::FloatArray*>::iterator fnd = m_floatArrays.find(name);
        if (fnd != m_floatArrays.end())
        {
            LOG_WARN << "Found pre-existing FloatArray with name " << name << ", renaming latest to \"" << name << "+\"";
            std::string copyofname = name + "+";

            addNamedFloatArray(farray, copyofname);
        }
        else
        {
            m_floatArrays.insert(std::map<std::string, ProcessSpecialDAI::FloatArray*>::value_type(name, new ProcessSpecialDAI::FloatArray(farray)));
        }
    }

    void TXProcessSpecialDA::addNamedIntegerArray(const ProcessSpecialDAI::IntegerArray& iarray, const std::string& name)
    {
        std::map<std::string, ProcessSpecialDAI::IntegerArray*>::iterator fnd = m_integerArrays.find(name);
        if (fnd != m_integerArrays.end())
        {
            LOG_WARN << "Found pre-existing IntegerArray with name " << name << ", renaming latest to \"" << name << "+\"";
            std::string copyofname = name + "+";

            addNamedIntegerArray(iarray, copyofname);
        }
        else
        {
            m_integerArrays.insert(std::map<std::string, ProcessSpecialDAI::IntegerArray*>::value_type(name, new ProcessSpecialDAI::IntegerArray(iarray)));
        }
    }

    void TXProcessSpecialDA::addNamedStringArray(const ProcessSpecialDAI::StringArray& sarray, const std::string& name)
    {
        std::map<std::string, ProcessSpecialDAI::StringArray*>::iterator fnd = m_stringArrays.find(name);
        if (fnd != m_stringArrays.end())
        {
            LOG_WARN << "Found pre-existing StringArray with name " << name << ", renaming latest to \"" << name << "+\"";
            std::string copyofname = name + "+";

            addNamedStringArray(sarray, copyofname);
        }
        else
        {
            m_stringArrays.insert(std::map<std::string, ProcessSpecialDAI::StringArray*>::value_type(name, new ProcessSpecialDAI::StringArray(sarray)));
        }
    }

    void TXProcessSpecialDA::addNamedBlob(const ProcessSpecialDAI::ByteArray& blob, const std::string& name)
    {
        std::map<std::string, ProcessSpecialDAI::ByteArray*>::iterator fnd = m_blobs.find(name);
        if (fnd != m_blobs.end())
        {
            LOG_WARN << "Found pre-existing ByteArray with name " << name << ", renaming latest to \"" << name << "+\"";
            std::string copyofname = name + "+";

            addNamedBlob(blob, copyofname);
        }
        else
        {
            m_blobs.insert(std::map<std::string, ProcessSpecialDAI::ByteArray*>::value_type(name, new ProcessSpecialDAI::ByteArray(blob)));
        }
    }
} /* namespace rootmap */

