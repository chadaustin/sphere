//
//  Generates the alpha table for existing (old)
//  pixel image.  Use something like:
//
//  make_old_clut.exe >../common/alpha_old.table
//
//  To apply previous alpha use:
//  cur_value.chan=alpha_old[cur_value.chan][target_alpha];
//
#include <stdio.h>

int main() {
  int v,a;

  for (v=0;v<256;++v) {
    printf("{");
    for (a=255;a>=0;--a) {
      printf("0x%02x",((v*a)/255));
      if (a>0) printf(",");
    }
    printf("}");
    if (v<255) printf(",");
    printf("\n");
  }

  return 0;
}
