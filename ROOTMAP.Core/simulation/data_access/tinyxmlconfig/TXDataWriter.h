#ifndef TXDataWriter_H
#define TXDataWriter_H

#include "core/log/Logger.h"

// Classes not in rootmap namespace
class TiXmlDocument;
class wxFileName;

namespace rootmap
{
    class TXConfigurationDataStore;

    class TXDataWriter
    {
    public:
        TXDataWriter(const std::string& directoryPath);
        ~TXDataWriter();

        /*
         *    Makes a copy of each XML file referenced by the main config file (rootmap.xml) referenced by the parameter TXConfigurationDataStore
         *  (writes them to the member directory, of course)
         */
        bool CloneConfigTree(const TXConfigurationDataStore& txcds);
        /*
         *    Makes a copy of each XML file referenced by the main config file (rootmap.xml) parameter
         *  (writes them to the member directory, of course)
         */
        bool CloneConfigTree(wxFileName sourceRootmapDotXml);
        /*
         *    Makes a copy of each XML file referenced by the main config file (rootmap.xml) used by the parameter TXDataWriter
         *  (writes them to the member directory, of course)
         */
        bool CloneConfigTree(const TXDataWriter* sourceDataWriter);

        /*
         *    Returns a pointer to an XML document in this TXDataWriter's cloned config tree, or NULL if such a document does not exist.
         *    Use this to make changes to the document, then call WriteDocument() to write those changes to file.
         */
        TiXmlDocument* GetDocument(const std::string& relativePath) const;
        /*
         *    Writes the XML DOM pointed to by the first parameter to the relative path defined by the second parameter.
         *    The path parameter should be relative to the base config directory (i.e. the location of rootmap.xml).
         */
        bool WriteDocument(TiXmlDocument* doc, const std::string& relativePath);

        bool HasIdenticalData(TXConfigurationDataStore& txcds) const;
        bool HasIdenticalData(wxFileName rootmapDotXml) const;

        bool DeleteConfigTree();

        std::string GetDir() const;

    private:
        RootMapLoggerDeclaration();
        // Do not use.
        TXDataWriter();

        std::string m_dir;
    };

    inline std::string TXDataWriter::GetDir() const
    {
        return m_dir;
    }
}
#endif // ifndef TXDataWriter_H
