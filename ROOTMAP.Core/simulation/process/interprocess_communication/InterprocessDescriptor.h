/* 
    InterprocessDescriptor.h

    Author:            Robert van Hugten
    Description:    methinks this was originally intended to be the class that
                    is now called SpecialProcessData. It isn't used anywhere.
*/

#ifndef InterprocessDescriptor_H
#define InterprocessDescriptor_H

namespace rootmap
{

class Process;

class InterprocessDescriptor
{
public:
    InterprocessDescriptor();
    virtual ~InterprocessDescriptor();

};

} // namespace rootmap

#endif    // InterprocessDescriptor_H
