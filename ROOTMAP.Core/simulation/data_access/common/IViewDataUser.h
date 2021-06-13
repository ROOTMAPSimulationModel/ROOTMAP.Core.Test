#ifndef IViewDataUser_H
#define IViewDataUser_H

/////////////////////////////////////////////////////////////////////////////
// Name:        IViewDataUser.h
// Purpose:     Declaration of the IViewDataUser class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
// IViewDataUser is the interface for a class which is able to create
// View windows from the ViewDAI
/////////////////////////////////////////////////////////////////////////////

namespace rootmap
{
    class ViewDAI;

    class IViewDataUser
    {
    public:
        /**
         * Called by the DataAccessManager::visualiseViews for each view that
         * has configuration data
         *
         * @param
         */
        virtual void createView(ViewDAI& data) = 0;

        virtual ~IViewDataUser()
        {
        }

    protected:
        IViewDataUser()
        {
        }
    }; // class IViewDataUser
} /* namespace rootmap */

#endif // #ifndef IViewDataUser_H
