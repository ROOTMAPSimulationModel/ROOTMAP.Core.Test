#ifndef IView3DDataUser_H
#define IView3DDataUser_H
/////////////////////////////////////////////////////////////////////////////
// Name:        IView3DDataUser.h
// Purpose:     Declaration of the IView3DDataUser class
// Created:     18-03-2008 16:59:49
// Author:      RvH
// $Date: 2008-06-20 10:30:16 +0800 (Fri, 20 Jun 2008) $
// $Revision: 7 $
// Copyright:   ©2002-2008 University of Tasmania
//
// IView3DDataUser is the interface for a class which is able to create
// View windows from the ViewDAI
/////////////////////////////////////////////////////////////////////////////

namespace rootmap
{
    class ViewDAI;

    class IView3DDataUser
    {
    public:
        /**
         * Called by the DataAccessManager::visualiseViews for each view that
         * has configuration data
         *
         * @param
         */
        virtual void createView3D(ViewDAI & data) = 0;

        virtual ~IView3DDataUser() {}

    protected:
        IView3DDataUser() {}
    }; // class IView3DDataUser

} // namespace rootmap

#endif // #ifndef IView3DDataUser_H
