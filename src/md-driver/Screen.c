/*

Windows 2000 Display Driver Model (XDDM) mirror driver for the Graphics Device Interface (GDI).

Copyright © Microsoft Corporation 1992-1998.
Copyright © SharpVNC Limited 2021. Use is permitted under license from SharpVNC Limited.

SharpVNC Limited trading as SharpVNC is a private limited company registered in England & Wales.
Registration number: 01234567. Registered office: 169 Picadilly, Mayfair, London, W1J 9EH United Kingdom.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include "Mirror.h"

#define     SYSTM_LOGFONT                   { 16, 7, 0, 0, 700, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,    VARIABLE_PITCH  | FF_DONTCARE, L"System"        }
#define     HELVE_LOGFONT                   { 12, 9, 0, 0, 400, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_STROKE_PRECIS,  PROOF_QUALITY,      VARIABLE_PITCH  | FF_DONTCARE, L"MS Sans Serif" }
#define     COURI_LOGFONT                   { 12, 9, 0, 0, 400, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_STROKE_PRECIS,  PROOF_QUALITY,      FIXED_PITCH     | FF_DONTCARE, L"Courier"       }
#define     NUM_PALETTE_COLORS              256
#define     NUM_PALETTE_RESERVED            20

const DEVINFO devInfoFramebuffer            = { (GCAPS_OPAQUERECT | GCAPS_LAYERED), SYSTM_LOGFONT, HELVE_LOGFONT, COURI_LOGFONT, 0, 0, 8, 8, 0 };

ULONG paletteColors[NUM_PALETTE_COLORS][4]  =
{
    { 0,      0,      0,      0 },  // 0
    { 0x80,   0,      0,      0 },  // 1
    { 0,      0x80,   0,      0 },  // 2
    { 0x80,   0x80,   0,      0 },  // 3
    { 0,      0,      0x80,   0 },  // 4
    { 0x80,   0,      0x80,   0 },  // 5
    { 0,      0x80,   0x80,   0 },  // 6
    { 0xC0,   0xC0,   0xC0,   0 },  // 7

    { 192,    220,    192,    0 },  // 8
    { 166,    202,    240,    0 },  // 9
    { 255,    251,    240,    0 },  // 10
    { 160,    160,    164,    0 },  // 11

    { 0x80,   0x80,   0x80,   0 },  // 12
    { 0xFF,   0,      0,      0 },  // 13
    { 0,      0xFF,   0,      0 },  // 14
    { 0xFF,   0xFF,   0,      0 },  // 15
    { 0,      0,      0xFF,   0 },  // 16
    { 0xFF,   0,      0xFF,   0 },  // 17
    { 0,      0xFF,   0xFF,   0 },  // 18
    { 0xFF,   0xFF,   0xFF,   0 }   // 19
};

BOOL InitialisePDEV(
    PPDEV       ppdev,
    DEVMODEW*   devMode,
    GDIINFO*    gdiInfo,
    DEVINFO*    devInfo)
{
    // Initialise default values.
    ppdev->ulMode                          = 0;
    ppdev->cxScreen                        = devMode->dmPelsWidth;
    ppdev->cyScreen                        = devMode->dmPelsHeight;
    ppdev->ulBitCount                      = devMode->dmBitsPerPel;
    ppdev->lDeltaScreen                    = 0;
    ppdev->flRed                           = 0x00FF0000;
    ppdev->flGreen                         = 0x000FF00;
    ppdev->flBlue                          = 0x00000FF;

    gdiInfo->ulVersion                     = GDI_DRIVER_VERSION;
    gdiInfo->ulTechnology                  = DT_RASDISPLAY;
    gdiInfo->ulHorzSize                    = 0;
    gdiInfo->ulVertSize                    = 0;
    gdiInfo->ulHorzRes                     = ppdev->cxScreen;
    gdiInfo->ulVertRes                     = ppdev->cyScreen;
    gdiInfo->ulPanningHorzRes              = 0;
    gdiInfo->ulPanningVertRes              = 0;
    gdiInfo->cBitsPixel                    = 32; // Fix for desktop icon shadows.
    gdiInfo->cPlanes                       = 1;
    gdiInfo->ulVRefresh                    = 1;
    gdiInfo->ulBltAlignment                = 1;
    gdiInfo->ulLogPixelsX                  = devMode->dmLogPixels;
    gdiInfo->ulLogPixelsY                  = devMode->dmLogPixels;
    gdiInfo->flTextCaps                    = TC_RA_ABLE;
    gdiInfo->flRaster                      = 0;
    gdiInfo->ulDACRed                      = 8;
    gdiInfo->ulDACGreen                    = 8;
    gdiInfo->ulDACBlue                     = 8;
    gdiInfo->ulAspectX                     = 0x24;
    gdiInfo->ulAspectY                     = 0x24;
    gdiInfo->ulAspectXY                    = 0x33;
    gdiInfo->xStyleStep                    = 1;
    gdiInfo->yStyleStep                    = 1;
    gdiInfo->denStyleStep                  = 3;
    gdiInfo->ptlPhysOffset.x               = 0;
    gdiInfo->ptlPhysOffset.y               = 0;
    gdiInfo->szlPhysSize.cx                = 0;
    gdiInfo->szlPhysSize.cy                = 0;
    gdiInfo->flShadeBlend                  = SB_CONST_ALPHA + SB_PIXEL_ALPHA; // Fix for desktop icon shadows.
    gdiInfo->ciDevice.Red.x                = 6700;
    gdiInfo->ciDevice.Red.y                = 3300;
    gdiInfo->ciDevice.Red.Y                = 0;
    gdiInfo->ciDevice.Green.x              = 2100;
    gdiInfo->ciDevice.Green.y              = 7100;
    gdiInfo->ciDevice.Green.Y              = 0;
    gdiInfo->ciDevice.Blue.x               = 1400;
    gdiInfo->ciDevice.Blue.y               = 800;
    gdiInfo->ciDevice.Blue.Y               = 0;
    gdiInfo->ciDevice.AlignmentWhite.x     = 3127;
    gdiInfo->ciDevice.AlignmentWhite.y     = 3290;
    gdiInfo->ciDevice.AlignmentWhite.Y     = 0;
    gdiInfo->ciDevice.RedGamma             = 20000;
    gdiInfo->ciDevice.GreenGamma           = 20000;
    gdiInfo->ciDevice.BlueGamma            = 20000;
    gdiInfo->ciDevice.Cyan.x               = 0;
    gdiInfo->ciDevice.Cyan.y               = 0;
    gdiInfo->ciDevice.Cyan.Y               = 0;
    gdiInfo->ciDevice.Magenta.x            = 0;
    gdiInfo->ciDevice.Magenta.y            = 0;
    gdiInfo->ciDevice.Magenta.Y            = 0;
    gdiInfo->ciDevice.Yellow.x             = 0;
    gdiInfo->ciDevice.Yellow.y             = 0;
    gdiInfo->ciDevice.Yellow.Y             = 0;
    gdiInfo->ciDevice.MagentaInCyanDye     = 0;
    gdiInfo->ciDevice.YellowInCyanDye      = 0;
    gdiInfo->ciDevice.CyanInMagentaDye     = 0;
    gdiInfo->ciDevice.YellowInMagentaDye   = 0;
    gdiInfo->ciDevice.CyanInYellowDye      = 0;
    gdiInfo->ciDevice.MagentaInYellowDye   = 0;
    gdiInfo->ulDevicePelsDPI               = 0;
    gdiInfo->ulPrimaryOrder                = PRIMARY_ORDER_CBA;

    gdiInfo->ulHTPatternSize               = HT_PATSIZE_4x4_M;
    gdiInfo->flHTFlags                     = HT_FLAG_ADDITIVE_PRIMS;

    *devInfo = devInfoFramebuffer;

#if (NTDDI_VERSION >= NTDDI_VISTA)
    devInfo->flGraphicsCaps2 |= GCAPS2_INCLUDEAPIBITMAPS | GCAPS2_EXCLUDELAYERED | GCAPS2_ALPHACURSOR | GCAPS2_MOUSETRAILS;
#endif

    if (ppdev->ulBitCount == 8)
    {
        gdiInfo->ulNumColors               = 20;
        gdiInfo->ulNumPalReg               = 1 << ppdev->ulBitCount;
        devInfo->flGraphicsCaps           |= (GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
        gdiInfo->ulHTOutputFormat          = HT_FORMAT_8BPP;
        devInfo->iDitherFormat             = BMF_8BPP;
        ppdev->cPaletteShift               = 8 - gdiInfo->ulDACRed;
    }
    else
    {
        gdiInfo->ulNumColors               = (ULONG)(-1);
        gdiInfo->ulNumPalReg               = 0;

        if (ppdev->ulBitCount == 16)
        {
            gdiInfo->ulHTOutputFormat      = HT_FORMAT_16BPP;
            devInfo->iDitherFormat         = BMF_16BPP;
        }
        else if (ppdev->ulBitCount == 24)
        {
            gdiInfo->ulHTOutputFormat      = HT_FORMAT_24BPP;
            devInfo->iDitherFormat         = BMF_24BPP;
        }
        else
        {
            gdiInfo->ulHTOutputFormat      = HT_FORMAT_32BPP;
            devInfo->iDitherFormat         = BMF_32BPP;
        }
    }

    devInfo->flGraphicsCaps |= (GCAPS_WINDINGFILL | GCAPS_GEOMETRICWIDE) | GCAPS2_REMOTEDRIVER;

    ULONG red       = 0;
    ULONG green     = 0;
    ULONG blue      = 0;

    for (INT i = NUM_PALETTE_RESERVED; i < NUM_PALETTE_COLORS; i++)
    {
        paletteColors[i][0] = red;
        paletteColors[i][1] = green;
        paletteColors[i][2] = blue;
        paletteColors[i][3] = 0;

        if (!(red += 32))
        {
            if (!(green += 32))
            {
                blue += 64;
            }
        }
    }

    if (ppdev->ulBitCount == 8)
    {
        devInfo->hpalDefault = ppdev->hpalDefault = EngCreatePalette(PAL_INDEXED, NUM_PALETTE_COLORS, (ULONG*)&paletteColors[0], 0, 0, 0);
    }
    else
    {
        devInfo->hpalDefault = ppdev->hpalDefault = EngCreatePalette(PAL_BITFIELDS, 0, NULL, ppdev->flRed, ppdev->flGreen, ppdev->flBlue);
    }

    return (TRUE);
}
