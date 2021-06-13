/******************************************************************************
 CCollaborator.c

        Abstract class that represents objects that collaborate
        with other objects. A Collaborator maintains a list of provider
        objects and a list of dependent objects. When a Collaborator
        changes, it sends itself the BroadcastChange message. Each
        dependent then receives the ProviderChanged message.

        CCollaborator can be used as a base class or as a mixin
        to classes that want to collaborate.

    Copyright © 1991, 1994 Symantec Corporation. All rights reserved.

 ******************************************************************************/

#include "core/macos_compatibility/CCollaborator.h"
 // #include "Global.h"
 // #include "CPtrArray.h"
 // #include "CVoidPtrArrayIterator.h"

 // A derived class is needed because of cross-dependencies
 // between CCollaborator and CCollection. Typedefs
 // cannot be forward declared.


 /******************************************************************************
  CCollaborator

      Constructor
 ******************************************************************************/

CCollaborator::CCollaborator()
{
}


/******************************************************************************
 CCollaborator

     Copy constructor. Don't copy dependent/provider pointers.
******************************************************************************/

CCollaborator::CCollaborator(const CCollaborator& /* source */)
{
    itsProviders.erase(itsProviders.begin(), itsProviders.end());
    itsDependents.erase(itsDependents.begin(), itsDependents.end());
}


/******************************************************************************
 ~CCollaborator

     Destructor. Remove dependencies.

******************************************************************************/

CCollaborator::~CCollaborator()
{
    for (CCollaboratorList::iterator iter1 = itsDependents.begin();
        iter1 != itsDependents.end();
        ++iter1
        )
    {
        (*iter1)->RemoveProvider(this);
    }

    for (CCollaboratorList::iterator iter2 = itsProviders.begin();
        iter2 != itsProviders.end();
        ++iter2
        )
    {
        (*iter2)->RemoveDependent(this);
    }
}


/******************************************************************************
 DependUpon

     Initiate a dependency by adding a provider to this object's provider list,
     and adding this object to the provider's dependent list.

******************************************************************************/

void CCollaborator::DependUpon(CCollaborator* aProvider)
{
    // an object should depend on another only
    // once, as it will be removed from the
    // list only once (TCL 2.0)

    // if (!itsProviders || !itsProviders.Includes(aProvider))
    // if (itsProviders.find(aProvider) != itsProviders.end())
    // {
    // RvH 20060601 itsProviders is now a by-value stl set so 
    // a) we don't need to check if itsProviders !=0 and
    // b) don't care if its already there
    AddProvider(aProvider);
    aProvider->AddDependent(this);
    // }
}


/******************************************************************************
 CancelDependency

     Terminate a dependency by removing a provider from this object's
     provider list, and removing this object from the provider's
     dependent list.
******************************************************************************/

void CCollaborator::CancelDependency(CCollaborator* aProvider)
{
    RemoveProvider(aProvider);
    aProvider->RemoveDependent(this);
}


/******************************************************************************
 AddDependant

     Add aDependent to the dependent list. Called by DependUpon.
******************************************************************************/

void CCollaborator::AddDependent(CCollaborator* aDependent)
{
    itsDependents.insert(aDependent);
}


/******************************************************************************
 RemoveDependant

     Remove a dependent from the dependency list. The dependent will cease
     receiving ProviderChanged messages. Called by CancelDependency.
     TCL 1.1.3:
         - As the last dependent is removed, dispense with the list entirely.
******************************************************************************/

void CCollaborator::RemoveDependent(CCollaborator* aDependent)
{
    itsDependents.erase(aDependent);
}


/******************************************************************************
 AddProvider

     Add aProvider to a Collaborators provider list. Called by DependUpon
******************************************************************************/

void CCollaborator::AddProvider(CCollaborator* aProvider)
{
    itsProviders.insert(aProvider);
}


/******************************************************************************
 RemoveProvider

     Remove aProvider from a Collaborators provider list. Called by
     CancelDependency
     TCL 1.1.3:
         - As the last provider is removed, dispense with the list entirely.
******************************************************************************/

void CCollaborator::RemoveProvider(CCollaborator* aProvider)
{
    itsProviders.erase(aProvider);
}


/******************************************************************************
 BroadcastChange

     Notify all dependents that a provider has changed. It is the responsibility
     of derived classes to implement a change protocol useful to dependents. The
     reason parameter should enumerate the types of changes, and the info parameter
     can be a pointer to any additional information needed to describe the change.

******************************************************************************/

void CCollaborator::BroadcastChange(long reason, CCollaboratorInfo* info)
{
    for (CCollaboratorList::iterator iter = itsDependents.begin();
        iter != itsDependents.end();
        ++iter
        )
    {
        (*iter)->ProviderChanged(this, reason, info);
    }
}


void CCollaborator::ProviderChanged(CCollaborator* /* aProvider */, long /* reason */, CCollaboratorInfo* /* info */)
{
}
