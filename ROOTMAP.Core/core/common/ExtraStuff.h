#ifndef CS_EXTRASTUFF_HH
#define CS_EXTRASTUFF_HH
//
// LINE_STRING
// Building blocks for the HERE macro
//
#define LINE_STRING_1(x) #x
#define LINE_STRING_2(x) LINE_STRING_1 (x)
#define LINE_STRING LINE_STRING_2 (__LINE__)

//
// HERE
//
// Macro for raising a compiler-generated message in a format which resembles
// the MS VC++ warning messages. In this way, the user can double click on the
// message and be taken to that point in the file.
//
// USAGE :
//#pragma message(HERE " fix function")
//
#define WARNING __FILE__ "(" LINE_STRING ") : warning "
#define HERE __FILE__ "(" LINE_STRING ") : "

#define LINE_HERE "(" LINE_STRING ") : "


//
// standard announcement of issues within code. This may do nothing, or
// it may cause compile-time warnings.
//
// USAGE : 
// eg. ISSUE(100)
#define PRAGMA #pragma

#define RmDelete(p) delete p; p=__nullptr
#define RmDeleteA(p) delete [] p; p=__nullptr

#endif // #ifndef CS_EXTRASTUFF_HH
