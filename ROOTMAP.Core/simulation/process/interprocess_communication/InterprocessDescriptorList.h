#ifndef InterprocessDescriptorList_H
#define InterprocessDescriptorList_H

namespace rootmap
{
    class InterprocessDataDescriptor;

    class InterprocessDescriptorList
    {
    public:
        /// Construction and destruction
        InterprocessDescriptorList();
        ~InterprocessDescriptorList();

        /// adds a descriptor of type Request
        void AddRequest(InterprocessDataDescriptor* request);
        /// General adding and taking
        void AddReceival(InterprocessDataDescriptor* receival);

        /// removes the first idd from the list, and returns the pointer to it
        InterprocessDataDescriptor* Pop(void);

        /// removes by index
        InterprocessDataDescriptor* RemoveFirstValidReceival(long int time);

        /// merely returns the pointer to the first idd
        InterprocessDataDescriptor* GetFirst();

        /**
         *
         */
        size_t GetNumItems() const;

        /**
         *
         */
        bool IsEmpty() const;

    private:
        ///
        ///
        bool descriptor_was_found;

        ///
        ///
        InterprocessDataDescriptor* FindRequest(InterprocessDataDescriptor* request);

        ///
        ///
        InterprocessDataDescriptor* FindReceival(InterprocessDataDescriptor* receival);

        ///
        ///
        InterprocessDataDescriptor* head;

        ///
        ///
        InterprocessDataDescriptor* tail;

        ///
        ///
        size_t numItems;

#if defined ROOTMAP_TODO // use std::list not this self-maintaining list
        ///
        ///
        typedef std::list< InterprocessDataDescriptor * > InterprocessDataDescriptorCollectionType;

        ///
        ///
        InterprocessDataDescriptorCollectionType m_dataDescriptors;
#endif // #if defined ROOTMAP_TODO
    };

    inline InterprocessDataDescriptor* InterprocessDescriptorList::GetFirst() { return (head); }
    inline size_t InterprocessDescriptorList::GetNumItems() const { return numItems; }
    inline bool InterprocessDescriptorList::IsEmpty() const { return (numItems == 0); }
} /* namespace rootmap */

#endif // #ifndef InterprocessDescriptorList_H
