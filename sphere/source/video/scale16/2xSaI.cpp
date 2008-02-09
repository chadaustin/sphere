/*---------------------------------------------------------------------*
 * The following (piece of) code, (part of) the 2xSaI engine,          *
 * copyright (c) 1999 by Derek Liauw Kie Fa.                           *
 * Non-Commercial use of the engine is allowed and is encouraged,      *
 * provided that appropriate credit be given and that this copyright   *
 * notice will not be removed under any circumstance.                  *
 * You may freely modify this code, but I request                      *
 * that any improvements to the engine be submitted to me, so          *
 * that I can implement these improvements in newer versions of        *
 * the engine.                                                         *
 * If you need more information, have any comments or suggestions,     *
 * you can e-mail me. My e-mail: derek-liauw@usa.net.                  *
 *---------------------------------------------------------------------*/
/* Rewritten by Anatoli Steinmark on 02.02.2008                        */
/* for use in the Sphere RPG Engine.                                   */
/*---------------------------------------------------------------------*/
/* 2xSaI, version 0.50                                                 */
/*---------------------------------------------------------------------*/

#include <stdlib.h>
#include "2xSaI.h"

#define     GREEN_MASK_565  (0x07E007E0)
#define NOT_GREEN_MASK_565  (~GREEN_MASK_565)
#define     COLOR_MASK_565  (0xF7DEF7DE)
#define   Q_COLOR_MASK_565  (0xE79CE79C)

#define     GREEN_MASK_555  (0x03E003E0)
#define NOT_GREEN_MASK_555  (~GREEN_MASK_555)
#define     COLOR_MASK_555  (0x7BDE7BDE)
#define   Q_COLOR_MASK_555  (0x739C739C)

dword       GREEN_MASK;
dword   NOT_GREEN_MASK;
dword       COLOR_MASK;
dword   NOT_COLOR_MASK;
dword     Q_COLOR_MASK;
dword NOT_Q_COLOR_MASK;


inline dword INTERPOLATE(dword A, dword B)
{
    if (A !=B)
    {
        return ( ((A & COLOR_MASK) >> 1) +
                 ((B & COLOR_MASK) >> 1) +
                 (A & B & NOT_COLOR_MASK) );
    }
    else
        return A;
}

inline dword Q_INTERPOLATE(dword A, dword B, dword C, dword D)
{
    dword x = ((A & Q_COLOR_MASK) >> 2) +
              ((B & Q_COLOR_MASK) >> 2) +
              ((C & Q_COLOR_MASK) >> 2) +
              ((D & Q_COLOR_MASK) >> 2);
               
    dword y = (A & NOT_Q_COLOR_MASK) +
              (B & NOT_Q_COLOR_MASK) +
              (C & NOT_Q_COLOR_MASK) +
              (D & NOT_Q_COLOR_MASK);
                        
    y = (y >> 2) & NOT_Q_COLOR_MASK;
    
    return x + y;
}

inline int GetResult1(dword A, dword B, dword C, dword D, dword E)
{
    int x = 0;
    int y = 0;
    int r = 0;
    
    if      (A == C) x++;
    else if (B == C) y++;

    if      (A == D) x++;
    else if (B == D) y++;

    if (x <= 1) r++;
    if (y <= 1) r--;
        
    return r;
}

inline int GetResult2(dword A, dword B, dword C, dword D, dword E)
{
    int x = 0;
    int y = 0;
    int r = 0;
    
    if      (A == C) x++;
    else if (B == C) y++;

    if      (A == D) x++;
    else if (B == D) y++;

    if (x <= 1) r--;
    if (y <= 1) r++;
    
    return r;
}

void _2xSaI(word* dst, word* src, int src_width, int src_height, int bpp)
{
    
    /*
    +--+--+--+--+
    |I |E |F |J |
    +--+--+--+--+
    |G |A |B |K |
    +--+--+--+--+
    |H |C |D |L |
    +--+--+--+--+
    |M |N |O |P |
    +--+--+--+--+
    
    +--+--+
    |A0|A1|
    +--+--+
    |A2|A3|
    +--+--+
    
    E: src0[0]
    A: src1[0]
    C: src2[0]
    N: src3[0]
    
    A0: dst0[0]
    A2: dst1[0]
    */
    
    if (bpp == 15)
    {
            GREEN_MASK   =  GREEN_MASK_555;
        NOT_GREEN_MASK   = ~GREEN_MASK;
            COLOR_MASK   =  COLOR_MASK_555;
        NOT_COLOR_MASK   = ~COLOR_MASK;
            Q_COLOR_MASK =  Q_COLOR_MASK_555;
        NOT_Q_COLOR_MASK = ~Q_COLOR_MASK_555;
    }
    else if (bpp == 16)
    {
            GREEN_MASK   =  GREEN_MASK_565;
        NOT_GREEN_MASK   = ~GREEN_MASK;
            COLOR_MASK   =  COLOR_MASK_565;
        NOT_COLOR_MASK   = ~COLOR_MASK;
            Q_COLOR_MASK =  Q_COLOR_MASK_565;
        NOT_Q_COLOR_MASK = ~Q_COLOR_MASK_565;
    }
    
    int src_pitch = src_width;
    int dst_pitch = src_width * 2;
            
    dword product;
    dword product1;
    dword product2;
    
    word*  src0 = src + 1 - src_pitch;
    word*  src1 = src + 1 + src_pitch;
    word*  src2 = src + 1 + src_pitch * 2;
    word*  src3 = src + 1 + src_pitch * 3;
    dword* dst0 = (dword*)(dst + 2 + dst_pitch * 2);
    dword* dst1 = (dword*)(dst + 2 + dst_pitch * 3);
    
    int iy = src_height - 2;
    int ix;
    
    // middle
    while (iy--)
    {
        ix = src_width - 2;
        
        while (ix--)
        {
            if (src1[0] == src2[1] && src1[1] != src2[0])
            {
                if ( (src1[0] == src0[0] && src1[1] == src2[2]) ||
                     (src1[0] == src2[0] && src1[0] == src0[1] && src1[1] != src0[0] && src1[1] == src0[2]) )
                    product = src1[0];
                else
                    product = INTERPOLATE(src1[0], src1[1]);

                if ( (src1[0] == src1[-1] && src2[0] == src3[1]) ||
                     (src1[0] == src1[1]  && src1[0] == src2[-1] && src1[-1] != src2[0] && src2[0] == src3[-1]) )
                    product1 = src1[0];
                else
                    product1 = INTERPOLATE(src1[0], src2[0]);

                product2 = src1[0];
            }
            
            else if (src1[1] == src2[0] && src1[0] != src2[1])
            {
                if ( (src1[1] == src0[1] && src1[0] == src2[-1]) ||
                     (src1[1] == src0[0] && src1[1] == src2[1] && src1[0] != src0[1] && src1[0] == src0[-1]) )
                    product = src1[1];
                else
                    product = INTERPOLATE(src1[0], src1[1]);

                if ( (src2[0] == src2[-1] && src1[0] == src0[1]) ||
                     (src2[0] == src1[-1] && src2[0] == src2[1] && src1[0] != src2[-1] && src1[0] == src0[-1]) )
                    product1 = src2[0];
                else
                    product1 = INTERPOLATE(src1[0], src2[0]);

                product2 = src1[1];
            }
            
            else if (src1[0] == src2[1] && src1[1] == src2[0])
            {
                if (src1[0] == src1[1])
                {
                    product  = src1[0];
                    product1 = src1[0];
                    product2 = src1[0];
                }
                else
                {
                    int r = 0;
                    product1 = INTERPOLATE(src1[0], src2[0]);
                    product  = INTERPOLATE(src1[0], src1[1]);
                    
                    r += GetResult1(src1[0], src1[1], src1[-1], src0[0], src0[-1]);
                    r += GetResult2(src1[1], src1[0], src1[2],  src0[1], src0[2]);
                    r += GetResult2(src1[1], src1[0], src2[-1], src3[0], src3[-1]);
                    r += GetResult1(src1[0], src1[1], src2[2],  src3[1], src3[2]);
                    
                    if (r > 0)
                        product2 = src1[0];
                    else if (r < 0)
                        product2 = src1[1];
                    else
                        product2 = Q_INTERPOLATE(src1[0], src1[1], src2[0], src2[1]);
                }
            }
            
            else
            {
                product2 = Q_INTERPOLATE(src1[0], src1[1], src2[0], src2[1]);
                
                if (src1[0] == src2[0] && src1[0] == src0[1] && src1[1] != src0[0] && src1[1] == src0[2])
                    product = src1[0];
                else if (src1[1] == src0[0] && src1[1] == src2[1] && src1[0] != src0[1] && src1[0] == src0[-1])
                    product = src1[1];
                else
                    product = INTERPOLATE(src1[0], src1[1]);

                if (src1[0] == src1[1] && src1[0] == src2[-1] && src1[-1] != src2[0] && src2[0] == src3[-1])
                    product1 = src1[0];
                else if (src2[0] == src1[-1] && src2[0] == src2[1] && src1[0] != src2[-1] && src1[0] == src0[-1])
                    product1 = src2[0];
                else
                    product1 = INTERPOLATE(src1[0], src2[0]);
            }
            
            dst0[0] = src1[0]  | (product  << 16);
            dst1[0] = product1 | (product2 << 16);
            
            ++src0;
            ++src1;
            ++src2;
            ++src3;
            ++dst0;
            ++dst1;
        }
        
        src0 += 2;
        src1 += 2;
        src2 += 2;
        src3 += 2;
        dst0 += src_width + 2;
        dst1 += src_width + 2;
    }
}
