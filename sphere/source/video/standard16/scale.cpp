
#include "scale.h"

void DirectScale(word* dst, int dst_pitch, word* src, int src_width, int src_height)
{
    /*
    +--+
    |E |
    +--+

    +--+--+
    |E0|E1|
    +--+--+
    |E2|E3|
    +--+--+

    E: src[0]

    E0: dst0[0]
    E2: dst1[0]
    */

    word* dst0 = dst;
    word* dst1 = dst + dst_pitch;

    int iy = src_height;
    int ix;

    while (iy--)
    {
        ix = src_width;

        while (ix--)
        {
            dst0[0] = src[0];
            dst0[1] = src[0];
            dst1[0] = src[0];
            dst1[1] = src[0];

            ++src;
            dst0 += 2;
            dst1 += 2;
        }

        dst0 += dst_pitch;
        dst1 += dst_pitch;
    }
}

////////////////////////////////////////////////////////////////////////////////
void Scale2x(word* dst, int dst_pitch, word* src, int src_width, int src_height)
{
    /*
    +--+--+--+
    |A |B |C |
    +--+--+--+
    |D |E |F |
    +--+--+--+
    |G |H |I |
    +--+--+--+

    +--+--+
    |E0|E1|
    +--+--+
    |E2|E3|
    +--+--+

    B: src0[0]
    E: src1[0]
    H: src2[0]

    E0: dst0[0]
    E2: dst1[0]
    */

    int src_pitch = src_width;

    word* dst0 = dst;
    word* dst1 = dst + dst_pitch;
    word* src0;
    word* src1 = src;
    word* src2;

    int iy = src_height;
    int ix;

    // left and right columns
    while (iy--)
    {
        dst0[0] = src1[0];
        dst1[0] = src1[0];
        dst0[1] = src1[0];
        dst1[1] = src1[0];

        src1 += src_pitch - 1;
        dst0 += dst_pitch - 2;
        dst1 += dst_pitch - 2;

        dst0[0] = src1[0];
        dst1[0] = src1[0];
        dst0[1] = src1[0];
        dst1[1] = src1[0];

        src1 += 1;
        dst0 += dst_pitch + 2;
        dst1 += dst_pitch + 2;
    }

    dst0 = dst + 2;
    dst1 = dst + 2 + dst_pitch;
    src1 = src + 1;

    int src_helper = src_pitch * (src_height - 1);
    int dst_helper = src_helper * 4;
    ix = src_pitch - 2;

    // top and bottom rows
    while (ix--)
    {
        dst0[0] = src1[0];
        dst1[0] = src1[0];
        dst0[1] = src1[0];
        dst1[1] = src1[0];

        src1 += src_helper;
        dst0 += dst_helper;
        dst1 += dst_helper;

        dst0[0] = src1[0];
        dst1[0] = src1[0];
        dst0[1] = src1[0];
        dst1[1] = src1[0];

        src1 -= src_helper - 1;
        dst0 -= dst_helper - 2;
        dst1 -= dst_helper - 2;
    }

    dst0 = dst + 2 + dst_pitch * 2;
    dst1 = dst + 2 + dst_pitch * 3;
    src0 = src + 1;
    src1 = src + 1 + src_pitch;
    src2 = src + 1 + src_pitch * 2;

    iy = src_height - 2;

    // middle
    while (iy--)
    {
        ix = src_pitch - 2;

        while (ix--)
        {
            if (src0[0] != src2[0] && src1[-1] != src1[1])
            {
                dst0[0] = src1[-1] == src0[0] ? src1[-1] : src1[0];
                dst0[1] = src0[0]  == src1[1] ? src1[1]  : src1[0];
                dst1[0] = src1[-1] == src2[0] ? src1[-1] : src1[0];
                dst1[1] = src2[0]  == src1[1] ? src1[1]  : src1[0];
            }
            else
            {
                dst0[0] = src1[0];
                dst0[1] = src1[0];
                dst1[0] = src1[0];
                dst1[1] = src1[0];
            }

            ++src0;
            ++src1;
            ++src2;
            dst0 += 2;
            dst1 += 2;
        }

        src0 += 2;
        src1 += 2;
        src2 += 2;
        dst0 += dst_pitch + 4;
        dst1 += dst_pitch + 4;
    }
}

////////////////////////////////////////////////////////////////////////////////
void Eagle(word* dst, int dst_pitch, word* src, int src_width, int src_height)
{
    int src_pitch = src_width;

    word* dst0 = dst;
    word* dst1 = dst + dst_pitch;
    word* src0;
    word* src1 = src;
    word* src2;

    int iy = src_height;
    int ix;

    // left and right columns
    while (iy--)
    {
        dst0[0] = src1[0];
        dst1[0] = src1[0];
        dst0[1] = src1[0];
        dst1[1] = src1[0];

        src1 += src_pitch - 1;
        dst0 += dst_pitch - 2;
        dst1 += dst_pitch - 2;

        dst0[0] = src1[0];
        dst1[0] = src1[0];
        dst0[1] = src1[0];
        dst1[1] = src1[0];

        src1 += 1;
        dst0 += dst_pitch + 2;
        dst1 += dst_pitch + 2;
    }

    dst0 = dst + 2;
    dst1 = dst + 2 + dst_pitch;
    src1 = src + 1;

    int src_helper = src_pitch * (src_height - 1);
    int dst_helper = src_helper * 4;
    ix = src_pitch - 2;

    // top and bottom rows
    while (ix--)
    {
        dst0[0] = src1[0];
        dst1[0] = src1[0];
        dst0[1] = src1[0];
        dst1[1] = src1[0];

        src1 += src_helper;
        dst0 += dst_helper;
        dst1 += dst_helper;

        dst0[0] = src1[0];
        dst1[0] = src1[0];
        dst0[1] = src1[0];
        dst1[1] = src1[0];

        src1 -= src_helper - 1;
        dst0 -= dst_helper - 2;
        dst1 -= dst_helper - 2;
    }

    dst0 = dst + 2 + dst_pitch * 2;
    dst1 = dst + 2 + dst_pitch * 3;
    src0 = src + 1;
    src1 = src + 1 + src_pitch;
    src2 = src + 1 + src_pitch * 2;

    iy = src_height - 2;

    // middle
    while (iy--)
    {
        ix = src_pitch - 2;

        while (ix--)
        {

            if (src0[0] == src0[-1] && src0[-1] == src1[-1])
                dst0[0] = src1[-1];
            else
                dst0[0] = src1[0];

            if (src0[0] == src0[1] && src0[1] == src1[1])
                dst0[1] = src1[1];
            else
                dst0[1] = src1[0];

            if (src2[0] == src2[-1] && src2[-1] == src1[-1])
                dst1[0] = src1[-1];
            else
                dst1[0] = src1[0];

            if (src2[0] == src2[1] && src2[1] == src1[1])
                dst1[1] = src1[1];
            else
                dst1[1] = src1[0];

            ++src0;
            ++src1;
            ++src2;
            dst0 += 2;
            dst1 += 2;
        }

        src0 += 2;
        src1 += 2;
        src2 += 2;
        dst0 += dst_pitch + 4;
        dst1 += dst_pitch + 4;
    }
}
