#include <stdio.h>
#include <setjmp.h>
extern "C" {
  #include <jpeglib.h>
}
#include "jpegwrap.h"


typedef unsigned char byte;


// internal data type implementations

struct JPEG_context
{
  JPEG_FileOpenCallback  open;
  JPEG_FileCloseCallback close;
  JPEG_FileReadCallback  read;

  void* data;  // private
};

struct JPEG_image
{
  int        width;
  int        height;
  JPEG_RGBA* pixels;
};


const int JPEG_BUFFER_SIZE = 1024;

struct JPEG_internal_struct
{
  struct
  {
    jpeg_error_mgr mgr;
    jmp_buf setjmp_buffer;
  } error_mgr;

  JPEG_context* context;

  void* file;
  byte buffer[JPEG_BUFFER_SIZE];
};


static void* JPEG_CALL JPEG_standard_open(JPEG_CONTEXT context, const char* filename);
static void  JPEG_CALL JPEG_standard_close(void* file);
static int   JPEG_CALL JPEG_standard_read(void* file, int bytes, void* buffer);

static void    JPEG_init_source(j_decompress_ptr cinfo);
static boolean JPEG_fill_input_buffer(j_decompress_ptr cinfo);
static void    JPEG_skip_input_data(j_decompress_ptr cinfo, long num_bytes);
static boolean JPEG_resync_to_restart(j_decompress_ptr cinfo, int desired);
static void    JPEG_term_source(j_decompress_ptr cinfo);
static void    JPEG_error_exit(j_common_ptr cinfo);
static void    JPEG_emit_message(j_common_ptr cinfo, int msg_level);


#define JPEG_FUNCTION(ret, name) ret JPEG_CALL name


#if defined(_WIN32)
  #include <windows.h>
  typedef DWORD dword;
#elif defined(unix)
  #include <sys/types.h>
  typedef u_int32_t dword;
#endif

// little-endian
inline dword MAKE_RGBA(byte red, byte green, byte blue, byte alpha) {
  return (alpha << 24) + (blue << 16) + (green << 8) + red;
}


////////////////////////////////////////////////////////////////////////////////

JPEG_FUNCTION(JPEG_CONTEXT, JPEG_CreateContext)()
{
  JPEG_context* context = new JPEG_context;
  context->open  = JPEG_standard_open;
  context->close = JPEG_standard_close;
  context->read  = JPEG_standard_read;
  context->data  = NULL;
  return context;
}

////////////////////////////////////////////////////////////////////////////////

JPEG_FUNCTION(void, JPEG_DestroyContext)(JPEG_CONTEXT context)
{
  JPEG_context* c = (JPEG_context*)context;
  delete c;
}

////////////////////////////////////////////////////////////////////////////////

JPEG_FUNCTION(void, JPEG_SetContextPrivate)(JPEG_CONTEXT context, void* data)
{
  JPEG_context* c = (JPEG_context*)context;
  c->data = data;
}

////////////////////////////////////////////////////////////////////////////////

JPEG_FUNCTION(void*, JPEG_GetContextPrivate)(JPEG_CONTEXT context)
{
  JPEG_context* c = (JPEG_context*)context;
  return c->data;
}

////////////////////////////////////////////////////////////////////////////////

JPEG_FUNCTION(void, JPEG_SetFileCallbacks)(JPEG_CONTEXT context, JPEG_FileOpenCallback open, JPEG_FileCloseCallback close, JPEG_FileReadCallback read)
{
  JPEG_context* c = (JPEG_context*)context;
  if (open == NULL || close == NULL || read == NULL) {
    // reset io callbacks
    c->open  = JPEG_standard_open;
    c->close = JPEG_standard_close;
    c->read  = JPEG_standard_read;
  } else {
    c->open  = open;
    c->close = close;
    c->read  = read;
  }
}

////////////////////////////////////////////////////////////////////////////////

JPEG_FUNCTION(JPEG_IMAGE, JPEG_LoadImage)(JPEG_CONTEXT context, const char* filename)
{
  JPEG_context* c = (JPEG_context*)context;

  JPEG_internal_struct is;
  is.context = (JPEG_context*)context;
  is.file = c->open(context, filename);
  if (is.file == NULL) {
    return NULL;
  }

  // initialize the source manager
  jpeg_source_mgr mgr;
  mgr.bytes_in_buffer   = 0;
  mgr.next_input_byte   = NULL;
  mgr.init_source       = JPEG_init_source;
  mgr.fill_input_buffer = JPEG_fill_input_buffer;
  mgr.skip_input_data   = JPEG_skip_input_data;
  mgr.resync_to_restart = JPEG_resync_to_restart;
  mgr.term_source       = JPEG_term_source;

  // initialize the decompression object
  jpeg_decompress_struct cinfo;
  jpeg_create_decompress(&cinfo);
  cinfo.client_data = &is;

  // set up error-handling
  cinfo.err = jpeg_std_error(&is.error_mgr.mgr);
  is.error_mgr.mgr.error_exit = JPEG_error_exit;

  JPEG_image* image = NULL;
  
  if (setjmp(is.error_mgr.setjmp_buffer)) {

    // free the image if it's been allocated
    if (image) {
      if (image->pixels) {
        delete[] image->pixels;
      }
      delete image;
    }

    jpeg_destroy_decompress(&cinfo);
    c->close(is.file);
    return NULL;
  }

  cinfo.src = &mgr;
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  // make a one-row-high sample array that will go away when done with image
  int row_stride = cinfo.output_width * cinfo.output_components;
  JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

  // allocate image
  image = new JPEG_image;
  image->width = cinfo.output_width;
  image->height = cinfo.output_height;
  image->pixels = new JPEG_RGBA[image->width * image->height];

  JPEG_RGBA* pixels = image->pixels;

  // read the scanlines
  while (cinfo.output_scanline < cinfo.output_height) {
    jpeg_read_scanlines(&cinfo, buffer, 1);

    // copy scanline into pixel buffer
    if (cinfo.output_components == 1) {        // greyscale
      for (int x = 0; x < image->width; x++) {
        byte i = (*buffer)[x];
        pixels[x] = MAKE_RGBA(i, i, i, 255);
      }
    } else if (cinfo.output_components == 3) { // RGB
      for (int x = 0; x < image->width; x++) {
        byte r = (*buffer)[x * 3 + 0];
        byte g = (*buffer)[x * 3 + 1];
        byte b = (*buffer)[x * 3 + 2];
        pixels[x] = MAKE_RGBA(r, g, b, 255);
      }
    }

    // go down a scanline
    pixels += image->width;
  }

  // finish up
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  c->close(is.file);

  return image;
}

////////////////////////////////////////////////////////////////////////////////

JPEG_FUNCTION(void, JPEG_DestroyImage)(JPEG_IMAGE image)
{
  JPEG_image* i = (JPEG_image*)image;
  delete[] i->pixels;
  delete i;
}

////////////////////////////////////////////////////////////////////////////////

JPEG_FUNCTION(int, JPEG_GetImageWidth)(JPEG_IMAGE image)
{
  JPEG_image* i = (JPEG_image*)image;
  return i->width;
}

////////////////////////////////////////////////////////////////////////////////

JPEG_FUNCTION(int, JPEG_GetImageHeight)(JPEG_IMAGE image)
{
  JPEG_image* i = (JPEG_image*)image;
  return i->height;
}

////////////////////////////////////////////////////////////////////////////////

JPEG_FUNCTION(JPEG_RGBA*, JPEG_GetImagePixels)(JPEG_IMAGE image)
{
  JPEG_image* i = (JPEG_image*)image;
  return i->pixels;
}

////////////////////////////////////////////////////////////////////////////////

void* JPEG_CALL
JPEG_standard_open(JPEG_CONTEXT /*context*/, const char* filename)
{
  FILE* file = fopen(filename, "rb");
  return file;
}

////////////////////////////////////////////////////////////////////////////////

void JPEG_CALL
JPEG_standard_close(void* file)
{
  FILE* f = (FILE*)file;
  fclose(f);
}

////////////////////////////////////////////////////////////////////////////////

int JPEG_CALL
JPEG_standard_read(void* file, int bytes, void* buffer)
{
  FILE* f = (FILE*)file;
  return fread(buffer, 1, bytes, f);
}

////////////////////////////////////////////////////////////////////////////////

void JPEG_init_source(j_decompress_ptr cinfo)
{
  // no initialization required
}

////////////////////////////////////////////////////////////////////////////////

boolean JPEG_fill_input_buffer(j_decompress_ptr cinfo)
{
  JPEG_internal_struct* is = (JPEG_internal_struct*)cinfo->client_data;

  cinfo->src->bytes_in_buffer = is->context->read(is->file, JPEG_BUFFER_SIZE, is->buffer);
  cinfo->src->next_input_byte = is->buffer;
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void JPEG_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
  if (num_bytes > 0) {
    while (num_bytes > (long)cinfo->src->bytes_in_buffer) {
      num_bytes -= (long)cinfo->src->bytes_in_buffer;
      JPEG_fill_input_buffer(cinfo);
    }
    cinfo->src->next_input_byte += (size_t)num_bytes;
    cinfo->src->bytes_in_buffer -= (size_t)num_bytes;
  }
}

////////////////////////////////////////////////////////////////////////////////

boolean JPEG_resync_to_restart(j_decompress_ptr cinfo, int desired)
{
  return jpeg_resync_to_restart(cinfo, desired);
}

////////////////////////////////////////////////////////////////////////////////

void JPEG_term_source(j_decompress_ptr cinfo)
{
  // we don't really need to do anything here...
}

////////////////////////////////////////////////////////////////////////////////

void JPEG_error_exit(j_common_ptr cinfo)
{
  JPEG_internal_struct* is = (JPEG_internal_struct*)cinfo->client_data;
  longjmp(is->error_mgr.setjmp_buffer, 1);
}

////////////////////////////////////////////////////////////////////////////////

void JPEG_emit_message(j_common_ptr cinfo, int msg_level)
{
  // ignore error messages
}

////////////////////////////////////////////////////////////////////////////////
