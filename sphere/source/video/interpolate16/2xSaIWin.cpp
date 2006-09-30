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
/* Modified by Chad Austin on 11.17.1999 for use in the Sphere RPG Engine */
/*                                                                        */
/* http://www.nerv-un.net/~aegis/                                         */
/* aegis@nerv-un.net                                                      */
/*---------------------*/
/* 2xSaI, version 0.50 */
/*---------------------*/
#include "2xSaIWin.h"
inline int GetResult1(uint32 A, uint32 B, uint32 C, uint32 D, uint32 E)
{
    int x = 0;
    int y = 0;
    int r = 0;
    if (A == C) x++;
    else if (B == C) y++;

    if (A == D) x++;
    else if (B == D) y++;

    if (x <= 1) r++;
    if (y <= 1) r--;
    return r;
}
inline int GetResult2(uint32 A, uint32 B, uint32 C, uint32 D, uint32 E)
{
    int x = 0;
    int y = 0;
    int r = 0;
    if (A == C) x++;
    else if (B == C) y++;

    if (A == D) x++;
    else if (B == D) y++;

    if (x <= 1) r--;
    if (y <= 1) r++;
    return r;
}
#define GREEN_MASK_555 (0x03E003E0)
#define NOT_GREEN_MASK_555 (~GREEN_MASK_555)
#define GREEN_MASK_565 (0x07E007E0)
#define NOT_GREEN_MASK_565 (~GREEN_MASK_565)
uint32 GREEN_MASK;
uint32 NOT_GREEN_MASK;
inline uint32 RowSharpen (uint32 A, uint32 B, uint32 C)
{
    if (B != C || A != C)
    {
        register int32 x = (B & GREEN_MASK) >> 5;
        register int32 y = ((A & GREEN_MASK) + ((C & GREEN_MASK) >> 7));
        x = x + (x >> 1) - (y);
        if (x < 0)        x = 0;
        else if (x > 63)  x = 63;
        return ((B & NOT_GREEN_MASK) | (x << 5));
    }
    else
        return B;
}
#define COLOR_MASK_555 (0x7BDE7BDE)
#define COLOR_MASK_565 (0xF7DEF7DE)
uint32 COLOR_MASK;
uint32 NOT_COLOR_MASK;
inline uint32 INTERPOLATE(uint32 A, uint32 B)
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
#define Q_COLOR_MASK_555 (0x739C739C)
#define Q_COLOR_MASK_565 (0xE79CE79C)
uint32 Q_COLOR_MASK;
uint32 NOT_Q_COLOR_MASK;
inline uint32 Q_INTERPOLATE(uint32 A, uint32 B, uint32 C, uint32 D)
{
    register uint32 x = ((A & Q_COLOR_MASK) >> 2) +
                        ((B & Q_COLOR_MASK) >> 2) +
                        ((C & Q_COLOR_MASK) >> 2) +
                        ((D & Q_COLOR_MASK) >> 2);
    register uint32 y = (A & NOT_Q_COLOR_MASK) +
                        (B & NOT_Q_COLOR_MASK) +
                        (C & NOT_Q_COLOR_MASK) +
                        (D & NOT_Q_COLOR_MASK);
    y = (y >> 2) & NOT_Q_COLOR_MASK;
    return x + y;
}
#include <stdlib.h>
void _2xSaIBitmap(int bit_depth, word* src, int src_width, int src_height, word* dest, int dest_pitch)
{
    if (src_width <= 0 || src_height <= 0)
    {

        exit(5);
        return;
    }
    // calculate color masks
    if (bit_depth == 15)
    {
        GREEN_MASK = GREEN_MASK_555;
        NOT_GREEN_MASK = ~GREEN_MASK;
        COLOR_MASK = COLOR_MASK_555;
        NOT_COLOR_MASK = ~COLOR_MASK;
        Q_COLOR_MASK = Q_COLOR_MASK_555;
        NOT_Q_COLOR_MASK = ~Q_COLOR_MASK_555;
    }
    else if (bit_depth == 16)
    {
        GREEN_MASK = GREEN_MASK_565;
        NOT_GREEN_MASK = ~GREEN_MASK;
        COLOR_MASK = COLOR_MASK_565;
        NOT_COLOR_MASK = ~COLOR_MASK;
        Q_COLOR_MASK = Q_COLOR_MASK_565;
        NOT_Q_COLOR_MASK = ~Q_COLOR_MASK_565;
    }
    else
        return;
    uint8* srcPtr = (uint8*)src;  // pointer to beginning of source bitmap
    uint8* dstPtr = (uint8*)dest; // pointer to beginning of destination bitmap
    dest_pitch >>= 2; // not really the pitch; the amount of pixels, per line
    // pitch / bytes per pixel. 4, not 2, cuz we're writing 2 pixels at the time
    uint32* dP;     // pointer to the pixels we are writing
    uint16* bP;     // pointer to the pixels we are reading
    bP = (uint16*)srcPtr;
    while (src_height-- >= 0)
    {
        //bP = (uint16*)srcPtr + src_width;
        dP = (uint32*)dstPtr;
//    int finish = src_width;
        while (src_width-- >= 0)
        {
            register uint32 colorA, colorB;
            uint32 colorC = 0, colorD = 0,
                                        colorE = 0, colorF = 0, colorG = 0, colorH = 0,
                                                                         colorI = 0, colorJ = 0, colorK = 0, colorL = 0,
                                                                                                          colorM = 0, colorN = 0, colorO = 0, colorP = 0;
            uint32 product, product1, product2;
//---------------------------------------
// Map of the pixels:                    I|E F|J
//                                       G|A B|K
//                                       H|C D|L
//                                       M|N O|P
            if (src_height != 240)
            {
                colorI = *(bP - src_width - 1);
                colorE = *(bP - src_width);
                colorF = *(bP - src_width + 1);
                colorJ = *(bP - src_width + 2);
            }
            else
            {
                colorI = *(bP);
                colorI = *(bP);
                colorI = *(bP);
                colorI = *(bP);
            }
            colorG = *(bP - 1);
            colorA = *(bP);
            colorB = *(bP + 1);
            colorK = *(bP + 2);
            //if (finish
            colorH = *(bP + src_width - 1);
            colorC = *(bP + src_width);
            colorD = *(bP + src_width + 1);
            colorL = *(bP + src_width + 2);
            colorM = *(bP + src_width + src_width - 1);
            //colorM = *(bP);
            colorN = *(bP + src_width + src_width);
            //colorN = *(bP);
            colorO = *(bP + src_width + src_width + 1);
            //colorO = *(bP);
            colorP = *(bP + src_width + src_width + 2);
            //colorP = *(bP);
            if ((colorA == colorD) && (colorB != colorC))
            {
                if ( ((colorA == colorE) && (colorB == colorL)) ||
                        ((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ)) )
                {
                    product = colorA;
                }
                else
                {
                    product = INTERPOLATE(colorA, colorB);
                }
                if (((colorA == colorG) && (colorC == colorO)) ||
                        ((colorA == colorB) && (colorA == colorH) && (colorG != colorC) && (colorC == colorM)) )
                {
                    product1 = colorA;
                }
                else
                {
                    product1 = INTERPOLATE(colorA, colorC);
                }
                product2 = colorA;
            }
            else if ((colorB == colorC) && (colorA != colorD))
            {
                if (((colorB == colorF) && (colorA == colorH)) ||
                        ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI)) )
                {
                    product = colorB;
                }
                else
                {
                    product = INTERPOLATE(colorA, colorB);
                }
                if (((colorC == colorH) && (colorA == colorF)) ||
                        ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI)) )
                {
                    product1 = colorC;
                }
                else
                {
                    product1 = INTERPOLATE(colorA, colorC);
                }
                product2 = colorB;
            }
            else if ((colorA == colorD) && (colorB == colorC))
            {
                if (colorA == colorB)
                {
                    product = colorA;
                    product1 = colorA;
                    product2 = colorA;
                }
                else
                {
                    register int r = 0;
                    product1 = INTERPOLATE(colorA, colorC);
                    product = INTERPOLATE(colorA, colorB);
                    r += GetResult1(colorA, colorB, colorG, colorE, colorI);
                    r += GetResult2(colorB, colorA, colorK, colorF, colorJ);
                    r += GetResult2(colorB, colorA, colorH, colorN, colorM);
                    r += GetResult1(colorA, colorB, colorL, colorO, colorP);
                    if (r > 0)
                        product2 = colorA;
                    else if (r < 0)
                        product2 = colorB;
                    else
                        product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);
                }
            }
            else
            {
                product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);
                if ((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ))
                {
                    product = colorA;
                }
                else if ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI))
                {
                    product = colorB;
                }
                else
                {
                    product = INTERPOLATE(colorA, colorB);
                }
                if ((colorA == colorB) && (colorA == colorH) && (colorG != colorC) && (colorC == colorM))
                {
                    product1 = colorA;
                }
                else if ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI))
                {
                    product1 = colorC;
                }
                else
                {
                    product1 = INTERPOLATE(colorA, colorC);
                }
            }
            *(dP)             = colorA   | (product << 16);
            *(dP + src_width) = product1 | (product2 << 16);
            bP++;
            dP++;
        } // end for (finish = width; etc...)
        dstPtr += dest_pitch << 4;
        srcPtr += src_width << 1;
    }
    ; //end for (; src_height; src_height--)

}
