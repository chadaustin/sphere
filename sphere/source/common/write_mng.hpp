#ifndef WRITE_MNG_H
#define WRITE_MNG_H

#include <libmng.h>
class CImage32;

const char* mng_get_error_message(mng_retcode code);

typedef mng_bool (*GetImage)(int image_index, CImage32& image, void* data);
typedef mng_uint32 (*GetDelay)(int index, void* data);
typedef mng_bool (*ContinueProcessingImages)(int image_index, int total);

mng_retcode
SaveMNGAnimationFromImages(const char* filename,
                           GetImage get_image,
                           GetDelay get_delay,
                           ContinueProcessingImages should_continue,
                           void* data);

#endif WRITE_MNG_H
