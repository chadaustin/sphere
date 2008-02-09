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

inline BGR INTERPOLATE(BGR A, BGR B)
{
    if (A !=B)
    {
        BGR temp;

        temp.red   = (A.red   + B.red)   >> 1;
        temp.green = (A.green + B.green) >> 1;
        temp.blue  = (A.blue  + B.blue)  >> 1;
        
        return temp;
    }
    else
        return A;
}

inline dword INTERPOLATE(dword A, dword B)
{
    if (A !=B)
    {
        return ( ((A & 0xFEFEFEFE) >> 1) +
                 ((B & 0xFEFEFEFE) >> 1) +
                 (A & B & 0x01010101) );
    }
    else
        return A;
}

inline BGR Q_INTERPOLATE(BGR A, BGR B, BGR C, BGR D)
{
    BGR temp;
    
    temp.red   = (A.red   + B.red   + C.red   + D.red)   >> 2;
    temp.green = (A.green + B.green + C.green + D.green) >> 2;
    temp.blue  = (A.blue  + B.blue  + C.blue  + D.blue)  >> 2;
    
    return temp;
}

inline dword Q_INTERPOLATE(dword A, dword B, dword C, dword D)
{
    dword x = ((A & 0xFCFCFCFC) >> 2) +
              ((B & 0xFCFCFCFC) >> 2) +
              ((C & 0xFCFCFCFC) >> 2) +
              ((D & 0xFCFCFCFC) >> 2);
               
    dword y = (A & 0x03030303) +
              (B & 0x03030303) +
              (C & 0x03030303) +
              (D & 0x03030303);
                        
    y = (y >> 2) & 0x03030303;
    
    return x + y;
}

template<typename T>
inline int GetResult1(T A, T B, T C, T D, T E)
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

template<typename T>
inline int GetResult2(T A, T B, T C, T D, T E)
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

void _2xSaI_32(dword* dst, dword* src, int src_width, int src_height)
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
    
    int src_pitch = src_width;
    int dst_pitch = src_width * 2;
    
    dword product;
    dword product1;
    dword product2;
    
    dword*  src0 = src + 1 - src_pitch;
    dword*  src1 = src + 1 + src_pitch;
    dword*  src2 = src + 1 + src_pitch * 2;
    dword*  src3 = src + 1 + src_pitch * 3;
    dword*  dst0 = dst + 2 + dst_pitch * 2;
    dword*  dst1 = dst + 2 + dst_pitch * 3;
    
    int iy = src_height - 2;
    int ix, r;
    
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
                    r = 0;
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
            
            dst0[0] = src1[0];
            dst0[1] = product;
            dst1[0] = product1;
            dst1[1] = product2;
            
            ++src0;
            ++src1;
            ++src2;
            ++src3;
            dst0 += 2;
            dst1 += 2;
        }
        
        src0 += 2;
        src1 += 2;
        src2 += 2;
        src3 += 2;
        dst0 += dst_pitch + 4;
        dst1 += dst_pitch + 4;
    }
}

void _2xSaI_24(BGR* dst, BGR* src, int src_width, int src_height)
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
    
    int src_pitch = src_width;
    int dst_pitch = src_width * 2;
    
    BGR product;
    BGR product1;
    BGR product2;
    
    BGR*  src0 = src + 1 - src_pitch;
    BGR*  src1 = src + 1 + src_pitch;
    BGR*  src2 = src + 1 + src_pitch * 2;
    BGR*  src3 = src + 1 + src_pitch * 3;
    BGR*  dst0 = dst + 2 + dst_pitch * 2;
    BGR*  dst1 = dst + 2 + dst_pitch * 3;
    
    int iy = src_height - 2;
    int ix, r;
    
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
                    r = 0;
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
            
            dst0[0] = src1[0];
            dst0[1] = product;
            dst1[0] = product1;
            dst1[1] = product2;
            
            ++src0;
            ++src1;
            ++src2;
            ++src3;
            dst0 += 2;
            dst1 += 2;
        }
        
        src0 += 2;
        src1 += 2;
        src2 += 2;
        src3 += 2;
        dst0 += dst_pitch + 4;
        dst1 += dst_pitch + 4;
    }
}
