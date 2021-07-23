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

#include "Mirror.h"

// Adds change bounds to the changes buffer.
VOID AddChange(
	CHANGES_BUFFER*		cb,
	RECTL*				bounds)
{
	if (!cb)
	{
		return;
	}
	if (!bounds)
	{
		return;
	}

	if (cb->count >= 20000)
	{
		cb->count = 0;
	}

	ULONG count = cb->count;
	cb->count++;

	cb->changes[count].type = 0;
	cb->changes[count].bounds.x = bounds->left;
	cb->changes[count].bounds.y = bounds->top;
	cb->changes[count].bounds.width = bounds->right - bounds->left;
	cb->changes[count].bounds.height = bounds->bottom - bounds->top;
}

// Adds the computed intersection of a clip region and destination bounds to the changes buffer.
VOID AddClipRegion(
	CHANGES_BUFFER*		cb,
	CLIPOBJ*			pco,
	RECTL*				dest)
{
	if (!pco)
	{
		return;
	}

	switch (pco->iDComplexity)
	{
		case DC_TRIVIAL:
		{
			// The clip region is not used, add the destination bounds as is.
			AddChange(cb, dest);

			break;
		}
		case DC_RECT:
		{
			AddChange(cb, &pco->rclBounds);

			break;
		}
		case DC_COMPLEX:
		{
			BOOL overflow;
			ENUMRECTS rects;

			CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

			do
			{
				overflow = CLIPOBJ_bEnum(pco, sizeof(rects), (ULONG*)&rects);

				for (RECTL* rect = &rects.arcl[0]; rects.c != 0; rects.c--, rect++)
				{
					AddChange(cb, rect);
				}
			} while (overflow);

			break;
		}
	}
}
