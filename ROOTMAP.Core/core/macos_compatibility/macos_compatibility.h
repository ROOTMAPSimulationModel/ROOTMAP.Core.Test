#ifndef macos_compatibility_H
#define macos_compatibility_H

// from MacTypes.h

/********************************************************************************

    Base integer types for all target OS's and CPU's

        UInt8            8-bit unsigned integer
        SInt8            8-bit signed integer
        UInt16          16-bit unsigned integer
        SInt16          16-bit signed integer
        UInt32          32-bit unsigned integer
        SInt32          32-bit signed integer
        UInt64          64-bit unsigned integer
        SInt64          64-bit signed integer

*********************************************************************************/
typedef unsigned char UInt8;
typedef signed char SInt8;
typedef unsigned short UInt16;
typedef signed short SInt16;
typedef unsigned long UInt32;
typedef signed long SInt32;

struct wide
{
    UInt32 lo;
    SInt32 hi;
};

typedef struct wide wide;

struct UnsignedWide
{
    UInt32 lo;
    UInt32 hi;
};

typedef struct UnsignedWide UnsignedWide;

/********************************************************************************

    Base fixed point types

        Fixed           16-bit signed integer plus 16-bit fraction
        UnsignedFixed   16-bit unsigned integer plus 16-bit fraction
        Fract           2-bit signed integer plus 30-bit fraction
        ShortFixed      8-bit signed integer plus 8-bit fraction

*********************************************************************************/
typedef long Fixed;
typedef Fixed* FixedPtr;
typedef long Fract;
typedef Fract* FractPtr;
typedef unsigned long UnsignedFixed;
typedef UnsignedFixed* UnsignedFixedPtr;
typedef short ShortFixed;
typedef ShortFixed* ShortFixedPtr;

/********************************************************************************

    MacOS Memory Manager types

        Ptr             Pointer to a non-relocatable block
        Handle          Pointer to a master pointer to a relocatable block
        Size            The number of bytes in a block (signed for historical reasons)

*********************************************************************************/
typedef char* Ptr;
typedef Ptr* Handle;
typedef long Size;

/********************************************************************************

    Higher level basic types

        OSErr                   16-bit result error code
        OSStatus                32-bit result error code
        LogicalAddress          Address in the clients virtual address space
        ConstLogicalAddress     Address in the clients virtual address space that will only be read
        PhysicalAddress         Real address as used on the hardware bus
        BytePtr                 Pointer to an array of bytes
        ByteCount               The size of an array of bytes
        ByteOffset              An offset into an array of bytes
        ItemCount               32-bit iteration count
        OptionBits              Standard 32-bit set of bit flags
        PBVersion               ?
        Duration                32-bit millisecond timer for drivers
        AbsoluteTime            64-bit clock
        ScriptCode              A particular set of written characters (e.g. Roman vs Cyrillic) and their encoding
        LangCode                A particular language (e.g. English), as represented using a particular ScriptCode
        RegionCode              Designates a language as used in a particular region (e.g. British vs American
                                English) together with other region-dependent characteristics (e.g. date format)
        FourCharCode            A 32-bit value made by packing four 1 byte characters together
        OSType                  A FourCharCode used in the OS and file system (e.g. creator)
        ResType                 A FourCharCode used to tag resources (e.g. 'DLOG')

*********************************************************************************/
typedef SInt16 OSErr;
typedef SInt32 OSStatus;
typedef void* LogicalAddress;
typedef const void* ConstLogicalAddress;
typedef void* PhysicalAddress;
typedef UInt8* BytePtr;
typedef UInt32 ByteCount;
typedef UInt32 ByteOffset;
typedef SInt32 Duration;
typedef UnsignedWide AbsoluteTime;
typedef UInt32 OptionBits;
typedef UInt32 ItemCount;
typedef UInt32 PBVersion;
typedef SInt16 ScriptCode;
typedef SInt16 LangCode;
typedef SInt16 RegionCode;
typedef unsigned long FourCharCode;
typedef FourCharCode OSType;
typedef FourCharCode ResType;
typedef OSType* OSTypePtr;
typedef ResType* ResTypePtr;

typedef unsigned char Str255[256];
typedef unsigned char Str63[64];
typedef unsigned char Str32[33];
typedef unsigned char Str31[32];
typedef unsigned char Str27[28];
typedef unsigned char Str15[16];

typedef Str255 StrFileName;
typedef unsigned char* StringPtr;
typedef StringPtr* StringHandle;
typedef const unsigned char* ConstStringPtr;
typedef const unsigned char* ConstStr255Param;
typedef const unsigned char* ConstStr63Param;
typedef const unsigned char* ConstStr32Param;
typedef const unsigned char* ConstStr31Param;
typedef const unsigned char* ConstStr27Param;
typedef const unsigned char* ConstStr15Param;

/********************************************************************************

    Quickdraw Types

        Point               2D Quickdraw coordinate, range: -32K to +32K
        Rect                Rectangular Quickdraw area
        Style               Quickdraw font rendering styles
        StyleParameter      Style when used as a parameter (historical 68K convention)
        StyleField          Style when used as a field (historical 68K convention)
        CharParameter       Char when used as a parameter (historical 68K convention)

    Note:   The original Macintosh toolbox in 68K Pascal defined Style as a SET.
            Both Style and CHAR occupy 8-bits in packed records or 16-bits when
            used as fields in non-packed records or as parameters.

*********************************************************************************/
struct Point
{
    short v;
    short h;
};

typedef struct Point Point;
typedef Point* PointPtr;

struct Rect
{
    short top;
    short left;
    short bottom;
    short right;
};

typedef struct Rect Rect;
typedef Rect* RectPtr;

struct FixedPoint
{
    Fixed x;
    Fixed y;
};

typedef struct FixedPoint FixedPoint;

struct FixedRect
{
    Fixed left;
    Fixed top;
    Fixed right;
    Fixed bottom;
};

typedef struct FixedRect FixedRect;

typedef short CharParameter;

enum
{
    normal = 0,
    bold = 1,
    italic = 2,
    underline = 4,
    outline = 8,
    shadow = 0x10,
    condense = 0x20,
    extend = 0x40
};

typedef bool Boolean;

/*********************************************************************************

    Old names for types

*********************************************************************************/
typedef UInt8 Byte;
typedef SInt8 SignedByte;
typedef wide* WidePtr;
typedef UnsignedWide* UnsignedWidePtr;
//typedef Float80                         extended80;
//typedef Float96                         extended96;
typedef SInt8 VHSelect;

#define noErr 0
#define TCL_DECLARE_CLASS

typedef struct Point Cell;

// This VHSelect-related enum is no longer in MacTypes.h (which is what
// Types.h became in MacOSX-isation of the Universal Interfaces) !!!
//RvH 20090710 Removed! No longer so much of a convenience enum as an
//INconvenience because of how easy it is to misplace a 'v' or an 'h'
//in the code.
//enum
//{
//    v = 0,
//    h = 1
//};


struct SFReply
{
    Str255 fName;
    OSType fType; // unsigned long
    int vRefNum;
    int version;
    bool good;
    bool copy;
};

typedef struct SFReply SFReply;

struct Dialog
{
    int stuff;
};

typedef struct Dialog Dialog;
typedef Dialog* DialogPtr;

struct Window
{
    int stuff;
};

typedef struct Window Window;
typedef Window* WindowPtr;

struct Menu
{
    int stuff;
};

typedef struct Menu Menu;
typedef Menu* MenuPtr;
typedef MenuPtr* MenuHandle;

#define TCLForgetPtr(x) delete x; x=0
#define TCLForgetObject(x) delete x; x=0

//#define FailNIL(p) throw std::runtime_error("FailNIL(p)")
//#define FailNILRes(p) throw std::runtime_error("FailNILRes(p)")
//#define FailMemError() throw std::runtime_error("FailMemError()")
//#define Debugger() throw std::logic_error("Debugger()")
#define FailNIL(p) RmAssert(p!=0, "FailNIL(p)")
#define FailNILRes(p) RmAssert(p!=0, "FailNILRes(p)")
#define FailMemError() RmAssert(false, "FailMemError() not supported")
#define Debugger() RmAssert(false, "Debugger() not supported")

struct RGBColor
{
    unsigned short red;
    unsigned short green;
    unsigned short blue;
};
#endif // #ifndef macos_compatibility_H
