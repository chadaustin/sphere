#ifndef JPEGWRAP_H
#define JPEGWRAP_H


#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif


#if defined(_WIN32)
#define JPEG_CALL __stdcall
#elif defined(unix)
#define JPEG_CALL
#endif


typedef void*         JPEG_CONTEXT;
typedef void*         JPEG_IMAGE;
typedef unsigned long JPEG_RGBA;  // RGBA little-endian

typedef void* (JPEG_CALL * JPEG_FileOpenCallback)(JPEG_CONTEXT context, const char* filename);
typedef void  (JPEG_CALL * JPEG_FileCloseCallback)(void* file);
typedef int   (JPEG_CALL * JPEG_FileReadCallback)(void* file, int bytes, void* buffer);


EXTERN_C JPEG_CONTEXT JPEG_CALL JPEG_CreateContext(void);
EXTERN_C void         JPEG_CALL JPEG_DestroyContext(JPEG_CONTEXT context);
EXTERN_C void         JPEG_CALL JPEG_SetContextPrivate(JPEG_CONTEXT context, void* data);
EXTERN_C void*        JPEG_CALL JPEG_GetContextPrivate(JPEG_CONTEXT context);
EXTERN_C void         JPEG_CALL JPEG_SetFileCallbacks(JPEG_CONTEXT context, JPEG_FileOpenCallback open, JPEG_FileCloseCallback close, JPEG_FileReadCallback read);
EXTERN_C JPEG_IMAGE   JPEG_CALL JPEG_LoadImage(JPEG_CONTEXT context, const char* filename);
EXTERN_C void         JPEG_CALL JPEG_DestroyImage(JPEG_IMAGE image);
EXTERN_C int          JPEG_CALL JPEG_GetImageWidth(JPEG_IMAGE image);
EXTERN_C int          JPEG_CALL JPEG_GetImageHeight(JPEG_IMAGE image);
EXTERN_C JPEG_RGBA*   JPEG_CALL JPEG_GetImagePixels(JPEG_IMAGE image);


#undef EXTERN_C


#endif
