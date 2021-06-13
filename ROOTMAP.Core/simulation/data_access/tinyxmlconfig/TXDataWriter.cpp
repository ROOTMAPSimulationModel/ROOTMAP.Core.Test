#include "simulation/data_access/tinyxmlconfig/TXDataWriter.h"
#include "simulation/data_access/tinyxmlconfig/TXUtility.h"
#include "simulation/data_access/tinyxmlconfig/TXConfigurationDataStore.h"
#include "wx/filename.h"
#include "tinyxml.h"
#include "core/common/RmAssert.h"
#include "core/common/ExtraStuff.h"
#include "core/utility/Utility.h"

/*
    MSA TODO 11.05.19 Test all this and handle for security/permissions/writeability exceptions.
*/

namespace rootmap
{
    RootMapLoggerDefinition(TXDataWriter);

    // Do not use.
    TXDataWriter::TXDataWriter()
    {
        RootMapLoggerInitialisation("rootmap.TXDataWriter");
    }

    TXDataWriter::TXDataWriter(const std::string& directoryPath)
        : m_dir(directoryPath)
    {
        RootMapLoggerInitialisation("rootmap.TXDataWriter");

        wxFileName fn(m_dir);
        fn.Normalize();
        m_dir = fn.GetPath();
        bool OK = true;
        if (!wxFileName::DirExists(m_dir))
        {
            OK = OK && wxFileName::Mkdir(m_dir);
        }
        OK = OK && wxFileName::IsDirReadable(m_dir);
        OK = OK && wxFileName::IsDirWritable(m_dir);
        std::string err = "ERROR: Failed to create or cannot read/write new directory " + m_dir;
        // MSA TODO Release-configuration error report
        //RmAssert(OK, err.c_str());
    }

    TXDataWriter::~TXDataWriter()
    {
    }

    bool TXDataWriter::CloneConfigTree(const TXConfigurationDataStore& txcds)
    {
        return CloneConfigTree(wxFileName(txcds.getConfigDir() + "/" + txcds.getConfigFile()));
    }

    bool TXDataWriter::CloneConfigTree(const TXDataWriter* sourceDataWriter)
    {
        const wxString rootmapDotXml = sourceDataWriter->GetDir() + "/" + "rootmap.xml";
        return CloneConfigTree(wxFileName(rootmapDotXml));
    }

    bool TXDataWriter::CloneConfigTree(wxFileName sourceRootmapDotXml)
    {
        TiXmlDocument* configDoc = new TiXmlDocument();

        sourceRootmapDotXml.Normalize();

        wxString sourceRootmapDotXmlPath = sourceRootmapDotXml.GetFullPath();
        sourceRootmapDotXmlPath.Replace("\\", "/");

        if (!configDoc->LoadFile(sourceRootmapDotXmlPath.c_str()))
        {
            delete configDoc;
            return false;
        }

        wxString rootmapDotXml = (m_dir + "/rootmap.xml");
        // Save a copy of the root document.
        if (!configDoc->SaveFile(rootmapDotXml))
        {
            RmAssert(false, "Failed to write rootmap.xml to file");
            // MSA TODO Release-configuration error report
            delete configDoc;
            return false;
        }

        // Now iterate through all files referenced in the root document, and copy them to this directory.

        TiXmlElement* rootElement = configDoc->RootElement();
        const char* root_value = rootElement->Value();
        RmAssert(0 == strcmp(root_value, "rootmap"), "Root node of any rootmap xml file must be \"rootmap\"");

        TiXmlNode* configNode = 0;

        // for each configuration item
        while (configNode = rootElement->IterateChildren("configuration", configNode))
        {
            // The configuration elements are name, type, owner, location
            //
            TiXmlElement* configElem = configNode->ToElement();
            if (configElem != 0)
            {
                std::string name = TXUtility::getChildElementText(configElem, "name");
                std::string type = TXUtility::getChildElementText(configElem, "type");
                std::string owner = TXUtility::getChildElementText(configElem, "owner");
                std::string location = TXUtility::getChildElementText(configElem, "location");

                wxString sourceDocPath = sourceRootmapDotXmlPath.BeforeLast('/') + "/" + location;
                wxString destDocPath = m_dir + "/" + location;

                // Converting all separators to slash, for simplicity of handling
                destDocPath.Replace("\\", "/");

                bool madeDirsOK = true;

                wxString workingRelativeDestPath = location;
                wxString workingAbsoluteDestDir = m_dir;
                // Converting all separators to slash, for simplicity of handling
                workingAbsoluteDestDir.Replace("\\", "/");
                while (workingRelativeDestPath.Contains("/") && madeDirsOK)
                {
                    wxString temp = workingRelativeDestPath.BeforeFirst('/');
                    workingAbsoluteDestDir += ("/" + temp);
                    workingRelativeDestPath = workingRelativeDestPath.AfterFirst('/');

                    if (!wxFileName::DirExists(workingAbsoluteDestDir))
                    {
                        madeDirsOK = madeDirsOK && wxFileName::Mkdir(workingAbsoluteDestDir);
                    }
                }


                TiXmlDocument* sourceDoc = new TiXmlDocument(sourceDocPath);
                bool loadOk = sourceDoc->LoadFile();

                if (loadOk)
                {
                    RootMapLogDebug("Successfully loaded RootMap config file \"" << sourceDocPath << "\".");
                }
                else
                {
                    RootMapLogError("Could not load RootMap config file \"" << sourceDocPath << "\", error=\"" << sourceDoc->ErrorDesc() << "\"");
                    delete sourceDoc;
                    continue;
                }

                bool savedOK = madeDirsOK ? sourceDoc->SaveFile(destDocPath.c_str()) : false;

                if (savedOK)
                {
                    RootMapLogDebug("Successfully saved RootMap config file \"" << destDocPath << "\".");
                }
                else
                {
                    if (madeDirsOK) RootMapLogError("Could not save RootMap config file \"" << destDocPath << "\", error=\"" << sourceDoc->ErrorDesc() << "\"");
                    else RootMapLogError("Could not make directory(ies) \"" << destDocPath << "\", error=\"" << sourceDoc->ErrorDesc() << "\"");
                    //TODO: throw a ConfigurationDataException with same string
                    delete sourceDoc;
                    continue;
                }

                if (name.empty() || type.empty() || owner.empty() || location.empty())
                {
                    // LOG badly formed configuration section
                    RootMapLogError(HERE << "Bad configuration in file " << rootmapDotXml << ", <Row,Col>=<" << configNode->Column() << "," << configNode->Row() << ">");
                }
                delete sourceDoc;
            }
            else
            {
                // LOG could not convert configuration node to element
                RootMapLogError(HERE << "Could not convert configuration node to element in file " << rootmapDotXml << ", <Row,Col>=<" << configNode->Column() << "," << configNode->Row() << ">");
            }
        }
        delete configDoc;
        return true;
    }

    TiXmlDocument* TXDataWriter::GetDocument(const std::string& relativePath) const
    {
        std::string p = m_dir + "/" + relativePath;
        TiXmlDocument* doc = new TiXmlDocument(p.c_str());
        bool loadOk = doc->LoadFile();

        if (loadOk)
        {
            return doc;
        }
        delete doc;
        return __nullptr;
    }

    bool TXDataWriter::WriteDocument(TiXmlDocument* doc, const std::string& relativePath)
    {
        std::string p = m_dir + "/" + relativePath;
        return doc->SaveFile(p.c_str());
    }

    bool TXDataWriter::HasIdenticalData(TXConfigurationDataStore& txcds) const
    {
        wxFileName rhsDir(txcds.getConfigDir());
        rhsDir.Normalize();
        wxString fullPath = rhsDir.GetFullPath();
        if (fullPath.CmpNoCase(m_dir) == 0) return false; // Don't want to compare this directory against itself

        TiXmlDocument* configDoc = txcds.getTxDocument();

        TiXmlElement* rootElement = configDoc->RootElement();
        const char* root_value = rootElement->Value();
        RmAssert(0 == strcmp(root_value, "rootmap"), "Root node of any rootmap xml file must be \"rootmap\"");

        TiXmlNode* configNode = 0;

        // for each configuration item
        while (configNode = rootElement->IterateChildren("configuration", configNode))
        {
            // The configuration elements are name, type, owner, location
            //
            TiXmlElement* configElem = configNode->ToElement();
            if (configElem != 0)
            {
                std::string location = TXUtility::getChildElementText(configElem, "location");

                wxString sourceDocPath = txcds.getConfigDir() + "/" + location;
                wxString destDocPath = m_dir + "/" + location;

                // Converting all separators to slash, for simplicity of handling
                sourceDocPath.Replace("\\", "/");
                destDocPath.Replace("\\", "/");

                wxFileName sourceFile = wxFileName(sourceDocPath);
                wxFileName destFile = wxFileName(destDocPath);

                sourceFile.Normalize();
                destFile.Normalize();

                wxString s1 = sourceFile.GetFullPath();
                wxString ss1 = sourceFile.GetFullName();
                wxString s2 = destFile.GetFullPath();
                wxString ss2 = destFile.GetFullName();

                SHA1Fingerprint f1 = Utility::GetFingerprint(sourceFile);
                SHA1Fingerprint f2 = Utility::GetFingerprint(destFile);

                if (f1 != f2) return false;
            }
        }

        return true;
    }

    bool TXDataWriter::HasIdenticalData(wxFileName rootmapDotXml) const
    {
        rootmapDotXml.Normalize();
        wxString rhsDir(rootmapDotXml.GetPath());
        if (rhsDir.CmpNoCase(m_dir) == 0) return false; // Don't want to compare this directory against itself

        TiXmlDocument* configDoc = new TiXmlDocument();

        if (!configDoc->LoadFile(rootmapDotXml.GetFullPath().c_str()))
        {
            delete configDoc;
            return false;
        }

        TiXmlElement* rootElement = configDoc->RootElement();
        const char* root_value = rootElement->Value();
        RmAssert(0 == strcmp(root_value, "rootmap"), "Root node of any rootmap xml file must be \"rootmap\"");

        TiXmlNode* configNode = 0;

        // for each configuration item
        while (configNode = rootElement->IterateChildren("configuration", configNode))
        {
            // The configuration elements are name, type, owner, location
            //
            TiXmlElement* configElem = configNode->ToElement();
            if (configElem != 0)
            {
                std::string location = TXUtility::getChildElementText(configElem, "location");

                wxString sourceDocPath = rootmapDotXml.GetPath() + "/" + location;
                wxString destDocPath = m_dir + "/" + location;

                // Converting all separators to slash, for simplicity of handling
                sourceDocPath.Replace("\\", "/");
                destDocPath.Replace("\\", "/");

                wxFileName sourceFile = wxFileName(sourceDocPath);
                wxFileName destFile = wxFileName(destDocPath);

                sourceFile.Normalize();
                destFile.Normalize();

                SHA1Fingerprint f1 = Utility::GetFingerprint(sourceFile);
                SHA1Fingerprint f2 = Utility::GetFingerprint(destFile);

                if (f1 != f2)
                {
                    delete configDoc;
                    return false;
                }
            }
        }
        delete configDoc;
        return true;
    }

    bool TXDataWriter::DeleteConfigTree()
    {
        TiXmlDocument* configDoc = new TiXmlDocument();

        wxString rootmapDotXml = (m_dir + "/rootmap.xml");
        // Save a copy of the root document.
        if (!configDoc->LoadFile(rootmapDotXml))
        {
            delete configDoc;
            return false; // How should this case be handled?
        }

        TiXmlElement* rootElement = configDoc->RootElement();
        const char* root_value = rootElement->Value();
        RmAssert(0 == strcmp(root_value, "rootmap"), "Root node of any rootmap xml file must be \"rootmap\"");

        TiXmlNode* configNode = 0;

        // for each configuration item
        while (configNode = rootElement->IterateChildren("configuration", configNode))
        {
            // The configuration elements are name, type, owner, location
            //
            TiXmlElement* configElem = configNode->ToElement();
            if (configElem != 0)
            {
                std::string location = TXUtility::getChildElementText(configElem, "location");

                wxString docPath = m_dir + "/" + location;

                // Converting all separators to slash, for simplicity of handling
                docPath.Replace("\\", "/");

                remove(docPath.c_str());

                // Now try to remove the subdirectory that contained this file, if there is one.
                // If any other files still exist in this subdir, the attempt to remove the subdir will fail.

                wxString workingRelativeDestDir = location;
                // Converting all separators to slash, for simplicity of handling
                workingRelativeDestDir.Replace("\\", "/");
                while (workingRelativeDestDir.Contains("/"))
                {
                    workingRelativeDestDir = workingRelativeDestDir.BeforeLast('/');

                    wxFileName tempDir = wxFileName(m_dir + "/" + workingRelativeDestDir + "/");
                    tempDir.Normalize();

                    wxString tempDirPath = tempDir.GetPath();

                    if (wxFileName::DirExists(tempDirPath))
                    {
                        wxFileName::Rmdir(tempDirPath);
                    }
                }
            }
        }

        delete configDoc;
        remove(rootmapDotXml.c_str());

        // Finally, try to erase the top level directory. This will only succeed if the directory is empty.
        // For this to be the case, the directory must have only contained files referenced by this rootmap.xml (i.e. the config files).
        wxFileName::SetCwd(m_dir + "/../");
        return wxFileName::Rmdir(m_dir);
    }
}
