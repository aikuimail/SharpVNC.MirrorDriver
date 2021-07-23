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

#define MAX_CHANGES 20000;			// The max number of changes.

typedef struct _CHANGE_BOUNDS
{
	LONG x;							// The X coordinate.
	LONG y;							// The Y coordinate.
	LONG width;						// The width.
	LONG height;					// The height.
} CHANGE_BOUNDS, *PCHANGE_BOUNDS;

typedef struct _CHANGE
{
	ULONG type;						// The type of change.
	CHANGE_BOUNDS bounds;			// The bounds of the change.
} CHANGE, *PCHANGE;

typedef struct _CHANGES_BUFFER
{
	ULONG count;					// The current count of changes.
	CHANGE changes[20000];			// Array of changes.
} CHANGES_BUFFER, *PCHANGES_BUFFER;

// Adds change bounds to the changes buffer.
VOID AddChange(
	CHANGES_BUFFER*		cb,
	RECTL*				bounds);

// Adds the computed intersection of a clip region and destination bounds to the changes buffer.
VOID AddClipRegion(
	CHANGES_BUFFER*		cb,
	CLIPOBJ*			pco,
	RECTL*				dest);
