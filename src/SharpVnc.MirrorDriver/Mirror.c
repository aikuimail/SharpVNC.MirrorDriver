/*

Windows 2000 Display Driver Model (XDDM) mirror driver for the Graphics Device Interface (GDI).

Copyright © Microsoft Corporation 1992-1998.
Copyright © SharpVNC Limited 2021. Use is permitted under license from SharpVNC Limited.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

// Include headers.
#include "Mirror.h"
#include "Debug.h"

#define HOOKS_BMF8BPP 0

#define HOOKS_BMF16BPP 0

#define HOOKS_BMF24BPP 0

#define HOOKS_BMF32BPP 0

// Define the driver function table.
static DRVFN routineTable[] =
{
	{ INDEX_DrvEnablePDEV,				(PFN)DrvEnablePDEV			},
	{ INDEX_DrvCompletePDEV,			(PFN)DrvCompletePDEV		},
	{ INDEX_DrvDisablePDEV,				(PFN)DrvDisablePDEV			},
	{ INDEX_DrvEnableSurface,			(PFN)DrvEnableSurface		},
	{ INDEX_DrvDisableSurface,			(PFN)DrvDisableSurface		},
	{ INDEX_DrvAssertMode,				(PFN)DrvAssertMode			},
	{ INDEX_DrvNotify,					(PFN)DrvNotify				},

	{ INDEX_DrvTextOut,					(PFN)DrvTextOut				},
	{ INDEX_DrvBitBlt,					(PFN)DrvBitBlt				},
	{ INDEX_DrvCopyBits,				(PFN)DrvCopyBits			},
	{ INDEX_DrvStrokePath,				(PFN)DrvStrokePath			},
	{ INDEX_DrvLineTo,					(PFN)DrvLineTo				},
	{ INDEX_DrvStrokeAndFillPath,		(PFN)DrvStrokeAndFillPath	},
	{ INDEX_DrvStretchBlt,				(PFN)DrvStretchBlt			},
	{ INDEX_DrvAlphaBlend,				(PFN)DrvAlphaBlend			},
	{ INDEX_DrvTransparentBlt,			(PFN)DrvTransparentBlt		},
	{ INDEX_DrvGradientFill,			(PFN)DrvGradientFill		},
	{ INDEX_DrvPlgBlt,					(PFN)DrvPlgBlt				},
	{ INDEX_DrvStretchBltROP,			(PFN)DrvStretchBltROP		},

#if (NTDDI_VERSION >= NTDDI_VISTA)
	{ INDEX_DrvRenderHint,				(PFN)DrvRenderHint			},
#endif

	{ INDEX_DrvEscape,					(PFN)DrvEscape				},
	{ INDEX_DrvMovePointer,				(PFN)DrvMovePointer			},
	{ INDEX_DrvSetPointerShape,			(PFN)DrvSetPointerShape		}
};

// Define routes to be called within the mirror driver.
#define hooks		  HOOK_BITBLT				/* DrvBitBlt function				*/	\
					| HOOK_TEXTOUT				/* DrvTextOut function				*/	\
					| HOOK_COPYBITS				/* DrvCopyBits function				*/	\
					| HOOK_STROKEPATH			/* DrvStrokePath function			*/	\
					| HOOK_LINETO				/* DrvLineTo function				*/	\
					| HOOK_FILLPATH				/* DrvFillPath function				*/	\
					| HOOK_STROKEANDFILLPATH	/* DrvStrokeAndFillPath function	*/	\
					| HOOK_STRETCHBLT			/* DrvStretchBlt function			*/	\
					| HOOK_ALPHABLEND			/* DrvAlphaBlend function			*/	\
					| HOOK_TRANSPARENTBLT		/* DrvTransparentBlt function		*/	\
					| HOOK_GRADIENTFILL			/* DrvGradientFill function			*/	\
					| HOOK_PLGBLT				/* DrvPlgBlt function				*/	\
					| HOOK_STRETCHBLTROP		/* DrvStretchBltROP function		*/ 

/*
The main entry point for the driver.
*/
BOOL DrvEnableDriver(
	ULONG				iEngineVersion,
	ULONG				cj,
	PDRVENABLEDATA		pded)
{
	DbOut((0, "Entered routine DrvEnableDriver\n"));

	if (cj >= sizeof(DRVENABLEDATA))
	{
		// Update the function table.
		pded->pdrvfn = routineTable;
	}

	if (cj >= (sizeof(ULONG) * 2))
	{
		pded->c = sizeof(routineTable) / sizeof(DRVFN);
	}

	if (cj >= sizeof(ULONG))
	{
		pded->iDriverVersion = DDI_DRIVER_VERSION_NT4;
	}
	
	return (TRUE);
}

/*
The DrvEnablePDEV function returns a description of the physical device's characteristics to GDI.
*/
DHPDEV DrvEnablePDEV(
	__in					DEVMODEW*	pDevmode,
	__in_opt				PWSTR		pwszLogAddress,
	__in					ULONG		cPatterns,
	__in_opt				HSURF*		ahsurfPatterns,
	__in					ULONG		cjGdiInfo,
	__out_bcount(cjGdiInfo) ULONG*		pGdiInfo,
	__in					ULONG		cjDevInfo,
	__out_bcount(cjDevInfo)	DEVINFO*	pDevInfo,
	__in_opt				HDEV		hdev,
	__in_opt				PWSTR		pwszDeviceName,
	__in					HANDLE		hDriver)
{
	DbOut((0, "Entered routine DrvEnablePDEV\n"));

	GDIINFO gdiInfo;
	DEVINFO devInfo;
	PPDEV ppdev = (PPDEV)NULL;

	// Allocate a physical device structure.
	ppdev = (PPDEV)EngAllocMem(FL_ZERO_MEMORY, sizeof(PDEV), ALLOC_TAG);

	// Check if null. If so, the routine has failed.
	if (ppdev == (PPDEV)NULL)
	{
		RIP("Routine DrvEnablePDEV failed at EngAllocMem\n");

		return ((DHPDEV)0);
	}

	// Save the current output handle.
	ppdev->hDriver = hDriver;

	if (!bInitPDEV(ppdev, pDevmode, &gdiInfo, &devInfo))
	{
		DbOut((0, "Routine DrvEnablePDEV failed at bInitPDEV\n"));

		goto error_free;
	}

	if (sizeof(DEVINFO) > cjDevInfo)
	{
		DbOut((0, "Routine DrvEnablePDEV failed at pDevInfo (insufficient memory)\n"));

		goto error_free;
	}

	// RtlCopyMemory is a wrapper around MemCpy in the event that MemCpy changes.
	RtlCopyMemory(pDevInfo, &devInfo, sizeof(DEVINFO));

	if (sizeof(GDIINFO) > cjGdiInfo)
	{
		DbOut((0, "DISP DrvEnablePDEV failed: insufficient pDevInfo memory\n"));
		goto error_free;
	}

	// RtlCopyMemory is a wrapper around MemCpy in the event that MemCpy changes.
	RtlCopyMemory(pGdiInfo, &gdiInfo, sizeof(GDIINFO));

	return ((DHPDEV)ppdev);

	// Error case for failure.
error_free:
	EngFreeMem(ppdev);
	return((DHPDEV)0);
}

/*
The DrvCompletePDEV function stores the GDI handle of the physical device being created.
*/
VOID DrvCompletePDEV(
	DHPDEV		dhpdev,
	HDEV		hdev)
{
	DbOut((1, "Entered routine DrvCompletePDEV\n"));

	((PPDEV)dhpdev)->hdevEng = hdev;
}

/*
The DrvDisablePDEV function is used by GDI to notify a driver that the specified PDEV is no longer needed.
*/
VOID DrvDisablePDEV(
	DHPDEV		dhpdev)
{
	DbOut((0, "Entered routine DrvDisablePDEV\n"));

	EngDeletePalette(((PPDEV)dhpdev)->hpalDefault);

	if (((PPDEV)dhpdev)->hMem)
	{
		EngUnmapFile(((PPDEV)dhpdev)->hMem);
	}
}

/*
The DrvEnableSurface function sets up a surface to be drawn on and associates it with a given physical device.
*/
HSURF DrvEnableSurface(
	DHPDEV		dhpdev)
{
	DbOut((0, "Entered routine DrvEnableSurface\n"));

	PPDEV ppdev = (PPDEV)dhpdev;
	HSURF hsurf;
	SIZEL sizl;
	ULONG ulBitmapType;
	FLONG flHooks;
	ULONG bytesPerPixel;
	MIRRSURF* mirrsurf;

	// Set the origin coordinates.
	((PPDEV)dhpdev)->ptlOrg.x = 0;
	((PPDEV)dhpdev)->ptlOrg.y = 0;

	sizl.cx = ppdev->cxScreen;
	sizl.cy = ppdev->cyScreen;

	if (ppdev->ulBitCount == 16)
	{
		ulBitmapType = BMF_16BPP;
		flHooks = HOOKS_BMF16BPP;
		bytesPerPixel = 2;
	}
	else if (ppdev->ulBitCount == 24)
	{
		ulBitmapType = BMF_24BPP;
		flHooks = HOOKS_BMF24BPP;
		bytesPerPixel = 3;
	}
	else
	{
		ulBitmapType = BMF_32BPP;
		flHooks = HOOKS_BMF32BPP;
		bytesPerPixel = 4;
	}

	flHooks |= hooks;

	hsurf = EngCreateDeviceSurface((DHSURF)ppdev, sizl, ulBitmapType);

	if (hsurf == (HSURF)0)
	{
		RIP("Routine DrvEnableSurface failed at EngCreateDeviceSurface\n");

		return (FALSE);
	}

	// Allocate memory for the framebuffer. This can be read in the user space.
	ppdev->pVideoMemory = EngMapFile(L"\\SystemRoot\\gdihook.dat", ppdev->cxScreen * ppdev->cyScreen * bytesPerPixel, &ppdev->hMem);

	EngModifySurface(hsurf, ppdev->hdevEng, flHooks, 0, (DHSURF)ppdev, ppdev->pVideoMemory, ppdev->cxScreen * bytesPerPixel, NULL);

	if (!EngAssociateSurface(hsurf, ppdev->hdevEng, flHooks))
	{
		RIP("Routine DrvEnableSurface failed at EngAssociateSurface\n");

		EngDeleteSurface(hsurf);

		return (FALSE);
	}

	ppdev->hsurfEng = (HSURF)hsurf;

	return (hsurf);
}

/*
The DrvNotify function allows a display driver to be notified about certain information by GDI.
*/
VOID DrvNotify(
	SURFOBJ*	pso,
	ULONG		iType,
	PVOID		pvData)
{
	DbOut((0, "Entered routine DrvNotify\n"));

	UNREFERENCED_PARAMETER(pso);
	UNREFERENCED_PARAMETER(pvData);

	switch (iType)
	{
		case DN_DEVICE_ORIGIN:
		{
			POINTL* coordinates = (POINTL*)pvData;

			DbOut((0, "Device origin: X=%x,Y=%y\n", coordinates->x, coordinates->y));

			break;
		}
		case DN_DRAWING_BEGIN:
		{
			DbOut((0, "Drawing started"));

			break;
		}
	}
}

/*
The DrvDisableSurface function is used by GDI to notify a driver that the surface created by
DrvEnableSurface for the current device is no longer needed.
*/
VOID DrvDisableSurface(
	DHPDEV			dhpdev)
{
	DbOut((0, "Entered routine DrvDisableSurface\n"));

	PPDEV ppdev = (PPDEV)dhpdev;

	EngDeleteSurface(ppdev->hsurfEng);
	EngFreeMem(ppdev->pvTmpBuffer);
}

/*
The DrvCopyBits function translates between device-managed raster surfaces and GDI standard-format bitmaps.
*/
BOOL DrvCopyBits(
	OUT			SURFOBJ*		psoDst,
	IN			SURFOBJ*		psoSrc,
	IN			CLIPOBJ*		pco,
	IN			XLATEOBJ*		pxlo,
	IN			RECTL*			prclDst,
	IN			POINTL*			pptlSrc)
{
	DbOut((0, "Entered routine DrvCopyBits\n"));
	
	return DrvBitBlt(psoDst, psoSrc, NULL, pco, pxlo, prclDst, pptlSrc, NULL, NULL, NULL, 0xCCCC);
}

/*
The DrvBitBlt function provides general bit-block transfer capabilities between device-managed surfaces, between GDI-managed
standard-format bitmaps, or between a device-managed surface and a GDI-managed standard-format bitmap.
*/
BOOL DrvBitBlt(
	IN			SURFOBJ*		psoDst,
	IN			SURFOBJ*		psoSrc,
	IN			SURFOBJ*		psoMask,
	IN			CLIPOBJ*		pco,
	IN			XLATEOBJ*		pxlo,
	IN			RECTL*			prclDst,
	IN			POINTL*			pptlSrc,
	IN			POINTL*			pptlMask,
	IN			BRUSHOBJ*		pbo,
	IN			POINTL*			pptlBrush,
	IN			ROP4			rop4)
{
	DbOut((0, "Entered routine DrvBitBlt\n"));

	PPDEV ppdev = ((PPDEV)(((SURFOBJ *)psoDst)->dhpdev));

	if (pco)
	{
		AddClipRegion(&ppdev->cb, pco, prclDst);
	}
	else
	{
		AddChange(&ppdev->cb, prclDst);
	}

	return EngBitBlt(psoDst, psoSrc, psoMask, pco, pxlo, prclDst, pptlSrc, pptlMask, pbo, pptlBrush, rop4);
}

/*
The DrvTextOut function is the entry point from GDI that calls for the driver to render a set of glyphs at specified positions.
*/
BOOL DrvTextOut(
	IN			SURFOBJ*		psoDst,
	IN			STROBJ*			pstro,
	IN			FONTOBJ*		pfo,
	IN			CLIPOBJ*		pco,
	IN			RECTL*			prclExtra,
	IN			RECTL*			prclOpaque,
	IN			BRUSHOBJ*		pboFore,
	IN			BRUSHOBJ*		pboOpaque,
	IN			POINTL*			pptlOrg,
	IN			MIX				mix)
{
	DbOut((0, "Entered routine DrvTextOut\n"));

	return EngTextOut(psoDst, pstro, pfo, pco, prclExtra, prclOpaque, pboFore, pboOpaque, pptlOrg, mix);
}

/*
The DrvStrokePath function strokes (outlines) a path.
*/
BOOL DrvStrokePath(
	SURFOBJ*		pso,
	PATHOBJ*		ppo,
	CLIPOBJ*		pco,
	XFORMOBJ*		pxo,
	BRUSHOBJ*		pbo,
	POINTL*			pptlBrush,
	LINEATTRS*		pLineAttrs,
	MIX				mix)
{
	DbOut((0, "Entered routine DrvStrokePath\n"));
		
	return EngStrokePath(pso, ppo, pco, pxo, pbo, pptlBrush, pLineAttrs, mix);
}

/*
The DrvLineTo function draws a single, solid, integer-only cosmetic line.
*/
BOOL DrvLineTo(
	SURFOBJ*		pso,
	CLIPOBJ*		pco,
	BRUSHOBJ*		pbo,
	LONG			x1,
	LONG			y1,
	LONG			x2,
	LONG			y2,
	RECTL*			prclBounds,
	MIX				mix)
{
	DbOut((0, "Entered routine DrvLineTo\n"));

	return EngLineTo(pso, pco, pbo, x1, y1, x2, y2, prclBounds, mix);
}

/*
The DrvFillPath function is an optional entry point to handle the filling of closed paths.
*/
BOOL DrvFillPath(
	SURFOBJ*		pso,
	PATHOBJ*		ppo,
	CLIPOBJ*		pco,
	BRUSHOBJ*		pbo,
	PPOINTL			pptlBrushOrg,
	MIX				mix,
	FLONG			flOptions)
{
	DbOut((0, "Entered routine DrvFillPath\n"));
	
	return EngFillPath(pso, ppo, pco, pbo, pptlBrushOrg, mix, flOptions);
}

/*
The DrvStrokeAndFillPath function strokes (outlines) and fills a path concurrently.
*/
BOOL DrvStrokeAndFillPath(
	SURFOBJ*		pso,
	PATHOBJ*		ppo,
	CLIPOBJ*		pco,
	XFORMOBJ*		pxo,
	BRUSHOBJ*		pboStroke,
	LINEATTRS*		plineattrs,
	BRUSHOBJ*		pboFill,
	POINTL*			pptlBrushOrg,
	MIX				mixFill,
	FLONG			flOptions)
{
	DbOut((0, "Entered routine DrvStrokeAndFillPath\n"));
	
	return EngStrokeAndFillPath(pso, ppo, pco, pxo, pboStroke, plineattrs, pboFill, pptlBrushOrg, mixFill, flOptions);
}

/*
The DrvTransparentBlt function provides bit-block transfer capabilities with transparency.
*/
BOOL DrvTransparentBlt(
	SURFOBJ*		psoDst,
	SURFOBJ*		psoSrc,
	CLIPOBJ*		pco,
	XLATEOBJ*		pxlo,
	RECTL*			prclDst,
	RECTL*			prclSrc,
	ULONG			iTransColor,
	ULONG			ulReserved)
{
	DbOut((0, "Entered routine DrvTransparentBlt\n"));

	return EngTransparentBlt(psoDst, psoSrc, pco, pxlo, prclDst, prclSrc, iTransColor, ulReserved);
}

/*
The DrvAlphaBlend function provides bit-block transfer capabilities with alpha blending.
*/
BOOL DrvAlphaBlend(
	SURFOBJ*		psoDst,
	SURFOBJ*		psoSrc,
	CLIPOBJ*		pco,
	XLATEOBJ*		pxlo,
	RECTL*			prclDst,
	RECTL*			prclSrc,
	BLENDOBJ*		pBlendObj)
{
	DbOut((0, "Entered routine DrvAlphaBlend\n"));

	return EngAlphaBlend(psoDst, psoSrc, pco, pxlo, prclDst, prclSrc, pBlendObj);
}

/*
The DrvGradientFill function shades the specified primitives.
*/
BOOL DrvGradientFill(
	SURFOBJ*		pso,
	CLIPOBJ*		pco,
	XLATEOBJ*		pxlo,
	TRIVERTEX*		pVertex,
	ULONG			nVertex,
	PVOID			pMesh,
	ULONG			nMesh,
	RECTL*			prclExtents,
	POINTL*			pptlDitherOrg,
	ULONG			ulMode)
{
	DbOut((0, "Entered routine DrvGradientFill\n"));

	return EngGradientFill(pso, pco, pxlo, pVertex, nVertex, pMesh, nMesh, prclExtents, pptlDitherOrg, ulMode);

}

/*
The DrvStretchBlt function provides stretching bit-block transfer capabilities between any combination of
device-managed and GDI-managed surfaces.
*/
BOOL DrvStretchBlt(
	SURFOBJ*			psoDst,
	SURFOBJ*			psoSrc,
	SURFOBJ*			psoMsk,
	CLIPOBJ*			pco,
	XLATEOBJ*			pxlo,
	COLORADJUSTMENT*	pca,
	POINTL*				pptlHTOrg,
	RECTL*				prclDst,
	RECTL*				prclSrc,
	POINTL*				pptlMsk,
	ULONG				iMode)
{
	DbOut((0, "Entered routine DrvStretchBlt\n"));

	return EngStretchBlt(psoDst, psoSrc, psoMsk, pco, pxlo, pca, pptlHTOrg, prclDst, prclSrc, pptlMsk, iMode);
}

/*
The DrvStretchBltROP function performs a stretching bit-block transfer using a ROP.
*/
BOOL DrvStretchBltROP(
	SURFOBJ*			psoTrg,
	SURFOBJ*			psoSrc,
	SURFOBJ*			psoMask,
	CLIPOBJ*			pco,
	XLATEOBJ*			pxlo,
	COLORADJUSTMENT*	pca,
	POINTL*				pptlBrushOrg,
	RECTL*				prclTrg,
	RECTL*				prclSrc,
	POINTL*				pptlMask,
	ULONG				iMode,
	BRUSHOBJ*			pbo,
	ROP4				rop4)
{
	DbOut((0, "Entered routine DrvStretchBltROP\n"));

	return EngStretchBltROP(psoTrg, psoSrc, psoMask, pco, pxlo, pca, pptlBrushOrg, prclTrg, prclSrc, pptlMask, iMode, pbo, rop4);
}

/*
The DrvPlgBlt function provides rotate bit-block transfer capabilities between combinations of
device-managed and GDI-managed surfaces.
*/
BOOL DrvPlgBlt(
	SURFOBJ*			psoTrg,
	SURFOBJ*			psoSrc,
	SURFOBJ*			psoMsk,
	CLIPOBJ*			pco,
	XLATEOBJ*			pxlo,
	COLORADJUSTMENT*	pca,
	POINTL*				pptlBrushOrg,
	POINTFIX*			pptfx,
	RECTL*				prcl,
	POINTL*				pptl,
	ULONG				iMode)
{
	DbOut((0, "Entered routine DrvPlgBlt\n"));

	return EngPlgBlt(psoTrg, psoSrc, psoMsk, pco, pxlo, pca, pptlBrushOrg, pptfx, prcl, pptl, iMode);
}

#if (NTDDI_VERSION >= NTDDI_VISTA)

LONG DrvRenderHint(
	DHPDEV				dhpdev,
	ULONG				NotifyCode,
	SIZE_T				Length,
	PVOID				Data)
{
	return TRUE;
}

#endif

/*
The DrvAssertMode function sets the mode of the specified physical device to either the mode
specified when the PDEV was initialized or to the default mode of the hardware.
*/
BOOL DrvAssertMode(
	DHPDEV			dhpdev,
	BOOL			bEnable)
{
	DbOut((0, "Entered routine DrvAssertMode\n"));

	UNREFERENCED_PARAMETER(bEnable);
	UNREFERENCED_PARAMETER(dhpdev);

	return TRUE;

}

/*
The DrvEscape function is used for retrieving information from a device that is not available
in a device-independent device driver interface; the particular query depends on the value of
the iEsc parameter.
*/
ULONG DrvEscape(
	SURFOBJ*		pso,
	ULONG			iEsc,
	ULONG			cjIn,
	PVOID			pvIn,
	ULONG			cjOut,
	PVOID			pvOut)
{
	DbOut((0, "Entered routine DrvEscape\n"));

	UNREFERENCED_PARAMETER(cjIn);
	UNREFERENCED_PARAMETER(pvIn);
	UNREFERENCED_PARAMETER(cjOut);
	UNREFERENCED_PARAMETER(pvOut);
	
	// Check if this is a SharpVNC Mirror Driver SDK message.
	if (iEsc >= 0x100000 && (iEsc & 0x100000) != 0)
	{
		// Get the PDEV for the current context.
		PPDEV ppdev = ((PPDEV)(((SURFOBJ*)pso)->dhpdev));

		switch (iEsc)
		{
			case 0x100001: // Enable hardware pointer capabilities
			{
				ppdev->enableHwCursor = TRUE;

				break;
			}
			case 0x100010: // Disable hardware pointer capabilities
			{
				ppdev->enableHwCursor = FALSE;

				break;
			}
			case 0x100011: // Get latest changes
			{
				CHANGES_BUFFER *cb = (CHANGES_BUFFER*)pvOut;

				ULONG count = ppdev->cb.count;

				cb->count = count;

				for (ULONG i = 0; i < count; ++i)
				{
					cb->changes[i].type				=	ppdev->cb.changes[i].type;
					cb->changes[i].bounds.x			=	ppdev->cb.changes[i].bounds.x;
					cb->changes[i].bounds.y			=	ppdev->cb.changes[i].bounds.y;
					cb->changes[i].bounds.width		=	ppdev->cb.changes[i].bounds.width;
					cb->changes[i].bounds.height	=	ppdev->cb.changes[i].bounds.height;
				}

				break;
			}
		}
	}

	return TRUE;
}

/*
The DrvMovePointer function moves the pointer to a new position and ensures that GDI does not
interfere with the display of the pointer.
*/
VOID DrvMovePointer(
	SURFOBJ			*pso,
	LONG			x,
	LONG			y,
	RECTL			*prcl)
{
	// Get the PDEV for the current context.
	PPDEV ppdev = ((PPDEV)(((SURFOBJ*)pso)->dhpdev));

	if (ppdev->enableHwCursor)
	{
		EngMovePointer(pso, x, y, prcl);
	}
}

/*
The DrvSetPointerShape function is used to request the driver to take the pointer off the
display, if the driver has drawn it there; to attempt to set a new pointer shape; and to put
the new pointer on the display at a specified position.
*/
ULONG DrvSetPointerShape(
	SURFOBJ			*pso,
	SURFOBJ			*psoMask,
	SURFOBJ			*psoColor,
	XLATEOBJ		*pxlo,
	LONG			xHot,
	LONG			yHot,
	LONG			x,
	LONG			y,
	RECTL			*prcl,
	FLONG			fl)
{
	// Get the PDEV for the current context.
	PPDEV ppdev = ((PPDEV)(((SURFOBJ*)pso)->dhpdev));

	if (ppdev->enableHwCursor)
	{
		return EngSetPointerShape(pso, psoMask, psoColor, pxlo, xHot, yHot, x, y, prcl, fl);
	}

	return SPS_ACCEPT_NOEXCLUDE;
}
