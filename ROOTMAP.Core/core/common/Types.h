#ifndef CoreTypes_H
#define CoreTypes_H
#include "core/macos_compatibility/macos_compatibility.h"
#include <map>

namespace rootmap
{
    ///
    typedef signed long int RootMapError_t;

    ///
    enum CSStreamModifier
    {
        endl = -1,
        hex = -2,
        dec = -3,
        bin = -4,
        clr = -5
    };

    enum BooleanStateChange
    {
        Off = 0x00,
        On = 0x01,
        Toggle = 0xF0
    };

    enum FailureBehaviour
    {
        Nothing = 0,
        LogMinor = 1,
        LogMajor = LogMinor << 1,
        Throw = LogMajor << 1,
        Assert = Throw << 1
    };

    typedef std::map<std::string, std::string> DataElementMap;
} /* namespace rootmap */

#endif // #ifndef CoreTypes_H
