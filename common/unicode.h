#ifndef __UNICODE_H
#define __UNICODE_H



#define wtext(s) (L##s)
#define wchr(c)  (L##c)

typedef unsigned short wchar;
typedef char           achar;



#ifdef UNICODE

  typedef wchar tchar;

  #define atot atow
  #define ttoa wtoa
  #define wtot wtow
  #define ttow wtow
  
  #define ttext(s) (L##s)
  #define tchr(c)  (L##c)

  #define TEOF      WEOF

  // input/output routines
  #define tfgetc    fgetwc
  #define tfgets    fgetws
  #define tfopen    wfopen
  #define tfprintf  fwprintf
  #define tfputc    fputwc
  #define tfputs    fputws
  #define tfscanf   fwscanf
  #define tgetc     getwc
  #define tgetchar  getwchar
  #define tgets     getws
  #define tprintf   wprintf
  #define tputc     putwc
  #define tputchar  putwchar
  #define tputs     _putws
  #define tscanf    wscanf
  #define tsprintf  swprintf
  #define tungetc   ungetwc
  #define tvfprintf vfwprintf
  #define tvprintf  vwprintf
  #define tvsprintf vswprintf

  // string routines
  #define tstrcat   wcscat
  #define tstrchr   wcschr
  #define tstrcmp   wcscmp
  #define tstrcpy   wcscpy
  #define tstrcspn  wcscspn
  #define tstrdup   wcsdup
  #define tstrlen   wcslen
  #define tstrncat  wcsncat
  #define tstrncmp  wcsncmp
  #define tstrncpy  wcsncmp
  #define tstrpbrk  wcspbrk
  #define tstrrchr  wcsrchr
  #define tstrspn   wcsspn
  #define tstrstr   wcsstr
  #define tstrtok   wcstok

  // character routines
  #define tisalnum  iswalnum
  #define tisalpha  iswalpha
  #define tiscntrl  iswcntrl
  #define tisdigit  iswdigit
  #define tisgraph  iswgraph
  #define tislower  iswlower
  #define tisprint  iswprint
  #define tispunct  iswpunct
  #define tisspace  iswspace
  #define tisupper  iswupper
  #define tisxdigit iswxdigit

  #define ttoupper  towupper
  #define ttolower  towlower

#else

  typedef achar tchar;

  #define atot atoa
  #define ttoa atoa
  #define wtot wtoa
  #define ttow atow

  #define ttext(s) (s)
  #define tchr(c)  (c)

  #define TEOF      EOF

  // input/output routines
  #define tfgetc    fgetc
  #define tfgets    fgets
  #define tfopen    fopen
  #define tfprintf  fprintf
  #define tfputc    fputc
  #define tfputs    fputs
  #define tfscanf   fscanf
  #define tgetc     getc
  #define tgetchar  getchar
  #define tgets     gets
  #define tprintf   printf
  #define tputc     putc
  #define tputchar  putchar
  #define tputs     puts
  #define tscanf    scanf
  #define tsprintf  sprintf
  #define tungetc   ungetc
  #define tvfprintf vfprintf
  #define tvprintf  vprintf
  #define tvsprintf vsprintf

  // string routines
  #define tstrcat   strcat
  #define tstrchr   strchr
  #define tstrcmp   strcmp
  #define tstrcpy   strcpy
  #define tstrcspn  strcspn
  #define tstrdup   strdup
  #define tstrlen   strlen
  #define tstrncat  strncat
  #define tstrncmp  strncmp
  #define tstrncpy  strncmp
  #define tstrpbrk  strpbrk
  #define tstrrchr  strrchr
  #define tstrspn   strspn
  #define tstrstr   strstr
  #define tstrtok   strtok

  // character routines
  #define tisalnum  isalnum
  #define tisalpha  isalpha
  #define tiscntrl  iscntrl
  #define tisdigit  isdigit
  #define tisgraph  isgraph
  #define tislower  islower
  #define tisprint  isprint
  #define tispunct  ispunct
  #define tisspace  isspace
  #define tisupper  isupper
  #define tisxdigit isxdigit

  #define ttoupper  toupper
  #define ttolower  tolower

#endif


#include "begin_c_prototypes.h"

  // conversion functions
  extern int atow(wchar* dest, const achar* src, int length); 
  extern int wtoa(achar* dest, const wchar* src, int length);
  extern int atoa(achar* dest, const achar* src, int length);
  extern int wtow(wchar* dest, const wchar* src, int length);

#include "end_c_prototypes.h"


#endif
