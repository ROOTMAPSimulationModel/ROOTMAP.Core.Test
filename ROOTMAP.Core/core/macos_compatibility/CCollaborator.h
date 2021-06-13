/******************************************************************************
 CCollaborator.h

        Interface for Collaborator class.

    Copyright © 1991, 1994 Symantec Corporation. All rights reserved.

 ******************************************************************************/
#ifndef _TCL_CCollaborator_HH_
#define _TCL_CCollaborator_HH_

#include <set>


class CCollaborator;
typedef std::set<CCollaborator *> CCollaboratorList;


struct CCollaboratorInfo
{
    virtual ~CCollaboratorInfo()
    {
    }

    CCollaboratorInfo()
    {
    }

private:
    // don't want copying or assignment, we can't control the derived contents
    CCollaboratorInfo(const CCollaboratorInfo&);
    CCollaboratorInfo& operator=(const CCollaboratorInfo&);
};


class CCollaborator
{
public:

    CCollaborator();
    CCollaborator(const CCollaborator& original);
    virtual ~CCollaborator();

    virtual void DependUpon(CCollaborator* aProvider);
    virtual void CancelDependency(CCollaborator* aProvider);

    virtual void BroadcastChange(long reason, CCollaboratorInfo* info); // (2.0.4)

protected:

    CCollaboratorList itsProviders; // the objects a collaborator depends upon
    CCollaboratorList itsDependents; // the objects that depend upon it

    virtual void ProviderChanged(CCollaborator* aProvider, long reason, CCollaboratorInfo* info) = 0;

private:

    virtual void AddDependent(CCollaborator* aDependent);
    virtual void RemoveDependent(CCollaborator* aDependent);

    virtual void AddProvider(CCollaborator* aProvider);
    virtual void RemoveProvider(CCollaborator* aProvider);
};
#endif // #ifndef _TCL_CCollaborator_HH_
