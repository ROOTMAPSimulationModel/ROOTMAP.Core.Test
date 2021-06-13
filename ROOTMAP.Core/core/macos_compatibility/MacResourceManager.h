#ifndef MacResourceManager_H
#define MacResourceManager_H
/////////////////////////////////////////////////////////////////////////////
// Name:        MacResourceManager.h
// Purpose:     Contains functions used by CropSim available only in the Mac
//              Toolbox. This is mostly GetString and GetIndString, which are
//              really internationalisation config issues. The intention is to
//              fix the functionality that uses these to call configuration
//              access functions.
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////


#include <stdexcept>

namespace rootmap
{
    namespace MacResourceManager
    {
        /**
         * GetIndString
         *
         * A replacement for the Macintosh ResourceManager function of the same
         * name. WARNING: The stringListNumber is validated, the stringIndex is NOT.
         *
         * @throw MacResourceManagerException if a string was not found at the
         *        specified stringListNumber
         * @param s the returned string
         * @param stringListNumber the number of the string list
         * @param stringIndex the index of the required string within the list
         */
        void GetIndString(std::string& s, long stringListNumber, long stringIndex);

        /**
         * GetIndString
         *
         * A replacement for the Macintosh ResourceManager function of
         * the same name.
         *
         * @throw MacResourceManagerException if a string was not found at that index.
         * @param s the returned string
         * @param stringListNumber the number of the string list
         * @param stringIndex the index of the required string within the list
         */
        std::string GetString(long stringIndex);

        /**
         * Initialises the MacResourceManager
         */
        void Initialise();
    } // namespace MacResourceManager

    class MacResourceManagerException : public std::exception
    {
    public:
        MacResourceManagerException(const char* reason)
            : myReason(reason)
        {
        }

        MacResourceManagerException(const MacResourceManagerException&)
        {
        }

        MacResourceManagerException& operator=(const MacResourceManagerException&) { return *this; }

        virtual ~MacResourceManagerException()
        {
        }

        virtual const char* what() const { return myReason.c_str(); }
    private:
        std::string myReason;
    };
} /* namespace rootmap */

#endif // #ifndef MacResourceManager_H
