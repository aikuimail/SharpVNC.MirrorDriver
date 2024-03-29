/*

Windows 2000 Display Driver Model (XDDM) mirror driver for the Graphics Device Interface (GDI).

Copyright � Microsoft Corporation 1992-1998.
Copyright � SharpVNC Limited 2021. Use is permitted under license from SharpVNC Limited.

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
	{ INDEX_DrvBitBlt,					(PFN)DrvBitBlt				},
	{ INDEX_DrvCopyBits,				(PFN)DrvCopyBits			},
	{ INDEX_DrvEscape,					(PFN)DrvEscape				},
	{ INDEX_DrvMovePointer,				(PFN)DrvMovePointer			},
	{ INDEX_DrvSetPointerShape,			(PFN)DrvSetPointerShape		},
	{ INDEX_DrvAlphaBlend,				(PFN)DrvAlphaBlend			},
	{ INDEX_DrvGetModes,				(PFN)DrvGetModes			}
};

// Define routes to be called within the mirror driver.
#define hooks		  HOOK_BITBLT				/* DrvBitBlt function				*/	\
					| HOOK_COPYBITS				/* DrvCopyBits function				*/  \
					| HOOK_ALPHABLEND			/* DrvAlphaBlend function			*/			

/// <summary>
/// The main entry point for the driver.
/// </summary>
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

/// <summary>
/// The DrvEnablePDEV function returns a description of the physical device's characteristics to GDI.
/// </summary>
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

	if (!InitialisePDEV(ppdev, pDevmode, &gdiInfo, &devInfo))
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

/// <summary>
/// The DrvCompletePDEV function stores the GDI handle of the physical device being created.
/// </summary>
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

	ULONG allocSize = (sizeof(CHANGES_BUFFER) + (sizeof(CHANGE) * 2000)) + (ppdev->cxScreen * ppdev->cyScreen * bytesPerPixel);
	ULONG changesStart = (sizeof(CHANGES_BUFFER));
	ULONG framebufferStart = (changesStart + (sizeof(CHANGE) * 2000));

	// Allocate memory for the framebuffer. This can be read in the user space.
	ppdev->sharedMemory = EngMapFile(L"\\SystemRoot\\gdihook.dat", allocSize, &ppdev->hMem);

	// Empty the memory.
	for (ULONG i = 0; i < allocSize; i++)
	{
		((CHAR*)ppdev->sharedMemory)[i] = 0;
	}

	// Store the framebuffer, changes buffer and changes pointers.
	ppdev->cb = ppdev->sharedMemory;
	ppdev->changes = (PVOID)((CHAR*)ppdev->sharedMemory + changesStart);
	ppdev->framebuffer = (PVOID)((CHAR*)ppdev->sharedMemory + framebufferStart);

	// Modify the surface to accept the new allocated memory.
	EngModifySurface(hsurf, ppdev->hdevEng, flHooks, 0, (DHSURF)ppdev, ppdev->framebuffer, ppdev->cxScreen * bytesPerPixel, NULL);

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
The DrvDisableSurface function is used by GDI to notify a driver that the surface created by
DrvEnableSurface for the current device is no longer needed.
*/
VOID DrvDisableSurface(
	DHPDEV			dhpdev)
{
	DbOut((0, "Entered routine DrvDisableSurface\n"));

	PPDEV ppdev = (PPDEV)dhpdev;

	EngDeleteSurface(ppdev->hsurfEng);

	EngUnmapFile(ppdev->hMem);

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
	//DbOut((0, "Entered routine DrvCopyBits\n"));
	
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
	//DbOut((0, "Entered routine DrvBitBlt\n"));

	if (psoDst)
	{
		PPDEV ppdev = ((PPDEV)(((SURFOBJ *)psoDst)->dhpdev));

		if (ppdev && &ppdev->iCb)
		{
			if (pco)
			{
				AddClipRegion(&ppdev->iCb, pco, prclDst);
			}
			else
			{
				AddChange(&ppdev->iCb, prclDst);
			}

			/*if (pptlSrc)
			{
				RECTL change = { 0 };
				change.left = pptlSrc->x;
				change.top = pptlSrc->y;
				change.right = pptlSrc->x + (prclDst->right - prclDst->left);
				change.bottom = pptlSrc->y + (prclDst->bottom - prclDst->top);

				if (pco)
				{
					AddClipRegion(&ppdev->iCb, pco, &change);
				}
				else
				{
					AddChange(&ppdev->iCb, &change);
				}
			}*/
		}
	}

	return EngBitBlt(psoDst, psoSrc, psoMask, pco, pxlo, prclDst, pptlSrc, pptlMask, pbo, pptlBrush, rop4);
}

/*
The DrvAssertMode function sets the mode of the specified physical device to either the mode
specified when the PDEV was initialized or to the default mode of the hardware.
*/
BOOL DrvAssertMode(
	DHPDEV			dhpdev,
	BOOL			bEnable)
{
	//DbOut((0, "Entered routine DrvAssertMode\n"));

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
	//DbOut((0, "Entered routine DrvEscape\n"));

	UNREFERENCED_PARAMETER(cjIn);
	UNREFERENCED_PARAMETER(pvIn);
	UNREFERENCED_PARAMETER(cjOut);
	UNREFERENCED_PARAMETER(pvOut);
	
	// Check if this is a SharpVNC Mirror Driver SDK message.
	if (iEsc >= SVNC_ESC_ID && (iEsc & SVNC_ESC_ID) != 0)
	{
		if (!pso)
		{
			return TRUE;
		}

		// Get the PDEV for the current context.
		PPDEV ppdev = ((PPDEV)(((SURFOBJ*)pso)->dhpdev));

		if (!ppdev)
		{
			return TRUE;
		}

		switch (iEsc)
		{
			case SVNC_ESC_ENABLE_HW_POINTER: // Enable hardware pointer capabilities
			{
				ppdev->enableHwCursor = TRUE;

				break;
			}
			case SVNC_ESC_DISABLE_HW_POINTER: // Disable hardware pointer capabilities
			{
				ppdev->enableHwCursor = FALSE;

				break;
			}
			case SVNC_ESC_GET_LATEST_CHANGES: // Get latest changes
			{
				if (!ppdev->cb)
				{
					return TRUE;
				}
				if (!ppdev->cb)
				{
					return TRUE;
				}

				((CHANGES_BUFFER*)ppdev->cb)->count = ppdev->iCb.count % 2000;

				CHANGE* changes = (CHANGE*)ppdev->changes;
								
				for (ULONG i = 0; i < MAX_CHANGES; i++)
				{
					changes[i].type				=	ppdev->iCb.changes[i].type;
					changes[i].bounds.x			=	ppdev->iCb.changes[i].bounds.x;
					changes[i].bounds.y			=	ppdev->iCb.changes[i].bounds.y;
					changes[i].bounds.width		=	ppdev->iCb.changes[i].bounds.width;
					changes[i].bounds.height	=	ppdev->iCb.changes[i].bounds.height;
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
	//DbOut((0, "Entered routine DrvMovePointer\n"));

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
	//DbOut((0, "Entered routine DrvSetPointerShape\n"));

	// Get the PDEV for the current context.
	PPDEV ppdev = ((PPDEV)(((SURFOBJ*)pso)->dhpdev));

	if (ppdev->enableHwCursor)
	{
		return EngSetPointerShape(pso, psoMask, psoColor, pxlo, xHot, yHot, x, y, prcl, fl);
	}

	return SPS_ACCEPT_NOEXCLUDE;
}

BOOL DrvAlphaBlend(
	SURFOBJ*		psoDst,
	SURFOBJ*		psoSrc,
	CLIPOBJ*		pco,
	XLATEOBJ*		pxlo,
	RECTL*			prclDst,
	RECTL*			prclSrc,
	BLENDOBJ*		pBlendObj)
{
	UNREFERENCED_PARAMETER(psoSrc);
	UNREFERENCED_PARAMETER(pxlo);
	UNREFERENCED_PARAMETER(prclSrc);
	UNREFERENCED_PARAMETER(pBlendObj);

	DbOut((0, "Entered routine DrvAlphaBlend\n"));

	if (psoDst)
	{
		PPDEV ppdev = ((PPDEV)(((SURFOBJ*)psoDst)->dhpdev));

		if (ppdev && &ppdev->iCb)
		{
			if (pco)
			{
				AddClipRegion(&ppdev->iCb, pco, prclDst);
			}
			else
			{
				AddChange(&ppdev->iCb, prclDst);
			}
		}
	}

	return EngAlphaBlend(psoDst, psoSrc, pco, pxlo, prclDst, prclSrc, pBlendObj);
}

ULONG DrvGetModes(
	HANDLE			hDriver,
	ULONG			cjSize,
	DEVMODEW*		pdm)
{
	return 0;
}
