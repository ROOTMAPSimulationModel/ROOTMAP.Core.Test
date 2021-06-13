#ifndef DataOutputFile_H
#define DataOutputFile_H
#include "core/common/Types.h"

#include "wx/ffile.h"
#include "wx/txtstrm.h"

namespace rootmap
{
    class ProcessActionDescriptor;

    class DataOutputFile : public wxTextOutputStream
    {
    public:
        // construction & destruction
        DataOutputFile(wxOutputStream& s, const wxString& filename);
        // MSA 09.09.22 Adding copy constructor
        DataOutputFile(DataOutputFile& rhs);
        // MSA 09.12.22 Adding copy-with-different-name constructor
        DataOutputFile(const wxString& filename, const DataOutputFile& file);
        virtual ~DataOutputFile();

        /**
         * Similar to WriteDouble, but with optional format specifier
         *
         * @param d the floating point value
         * @param format the string specifying the [s]printf format string
         */
        void WriteDoubleFormat(double d, const char* format);

        //virtual void WriteString(const std::string & s);
        //virtual void WriteString(const char * s);

        /**
         * with added "D" to indicate with trailing Delimiter
         */
         //virtual void WriteDoubleD(double d, const char * trailing_delimiter=__nullptr);
         //virtual void Write32D(long l, const char * trailing_delimiter=__nullptr);
         //virtual void WriteEol();

         // MSA added this to allow RaytracerData output strings to
         // know their own name
        const wxString& GetFilename();

    protected:
        wxOutputStream& m_baseStream;
    private:
        const wxString m_filename;
    };

    inline const wxString& DataOutputFile::GetFilename()
    {
        return m_filename;
    }
} /* namespace rootmap */

#endif // #ifndef DataOutputFile_H
