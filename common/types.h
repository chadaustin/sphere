// portable type definitions
// chad austin
// 7.26.1999


#ifndef __TYPES_H__
#define __TYPES_H__


// boolean (VC++'s bool is 1 byte)
#ifndef __cplusplus
#define bool  unsigned char
#define true  1
#define false 0
#endif


// sized types
typedef unsigned char    byte;
typedef signed   char    sbyte;
typedef unsigned char    ubyte;

typedef unsigned short   word;
typedef signed   short   sword;
typedef unsigned short   uword;

typedef unsigned long    dword;
typedef signed   long    sdword;
typedef unsigned long    udword;

#ifdef _MSC_VER

typedef unsigned __int64 qword;
typedef signed   __int64 sqword;
typedef unsigned __int64 uqword;

typedef unsigned short   ushort;
typedef signed   short   sshort;

typedef unsigned long    ulong;
typedef signed   long    slong;

typedef          __int8  int8;
typedef          __int16 int16;
typedef          __int32 int32;
typedef          __int64 int64;

typedef signed   int     sint;
typedef signed   __int8  sint8;
typedef signed   __int16 sint16;
typedef signed   __int32 sint32;

typedef unsigned int     uint;
typedef unsigned __int8  uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;

typedef signed   char    schar;
typedef unsigned char    uchar;

typedef          char    char8;
typedef signed   char    schar8;
typedef unsigned char    uchar8;

typedef          short   char16;
typedef signed   short   schar16;
typedef unsigned short   uchar16;

typedef          float   float32;
typedef          double  float64;

#endif /* _MSC_VER */


#endif /* __TYPES_H */
