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

#include <dderror.h>
#include <devioctl.h>
#include <miniport.h>
#include <ntddvdeo.h>
#include <video.h>
#include "Mirror.h"

VOID DbgBreakPoint() {};

VOID
MirrorNotImplemented(
    __in  char* s
)
{
    VideoDebugPrint((0, "Mirror Sample: Not used '%s'.\n", s));
}

/// <summary>
/// 
/// </summary>
VP_STATUS __checkReturn HwFindAdapter(
    __in                                            PVOID                   HwDeviceExtension,
    __in                                            PVOID                   HwContext,
    __in                                            PWSTR                   ArgumentString,
    __inout_bcount(sizeof(VIDEO_PORT_CONFIG_INFO))  PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    __out                                           PUCHAR                  Again)
{
    UNREFERENCED_PARAMETER(HwDeviceExtension);
    UNREFERENCED_PARAMETER(HwContext);
    UNREFERENCED_PARAMETER(ArgumentString);
    UNREFERENCED_PARAMETER(ConfigInfo);
    UNREFERENCED_PARAMETER(Again);

    return NO_ERROR;
}

/// <summary>
/// 
/// </summary>
BOOLEAN HwInitialize(
    PVOID                   HwDeviceExtension)
{
    UNREFERENCED_PARAMETER(HwDeviceExtension);

    return TRUE;
}

/// <summary>
/// 
/// </summary>
BOOLEAN HwStartIO(
    PVOID                   HwDeviceExtension,
    PVIDEO_REQUEST_PACKET   RequestPacket)
{
    UNREFERENCED_PARAMETER(HwDeviceExtension);
    UNREFERENCED_PARAMETER(RequestPacket);

    return TRUE;
}

/// <summary>
/// 
/// </summary>
BOOLEAN HwResetHw(
    PVOID                   HwDeviceExtension,
    ULONG                   Columns,
    ULONG                   Rows)
{
    UNREFERENCED_PARAMETER(HwDeviceExtension);
    UNREFERENCED_PARAMETER(Columns);
    UNREFERENCED_PARAMETER(Rows);

    return TRUE;
}

/// <summary>
/// 
/// </summary>
BOOLEAN HwInterrupt(
    PVOID                   HwDeviceExtension)
{
    UNREFERENCED_PARAMETER(HwDeviceExtension);

    return TRUE;
}

/// <summary>
/// 
/// </summary>
VP_STATUS HwGetPowerState(
    PVOID                   HwDeviceExtension,
    ULONG                   HwId,
    PVIDEO_POWER_MANAGEMENT VideoPowerControl)
{
    UNREFERENCED_PARAMETER(HwDeviceExtension);
    UNREFERENCED_PARAMETER(HwId);
    UNREFERENCED_PARAMETER(VideoPowerControl);

    return NO_ERROR;
}

/// <summary>
/// 
/// </summary>
VP_STATUS HwSetPowerState(
    PVOID                   HwDeviceExtension,
    ULONG                   HwId,
    PVIDEO_POWER_MANAGEMENT VideoPowerControl)
{
    UNREFERENCED_PARAMETER(HwDeviceExtension);
    UNREFERENCED_PARAMETER(HwId);
    UNREFERENCED_PARAMETER(VideoPowerControl);

    return NO_ERROR;
}

/// <summary>
/// 
/// </summary>
VP_STATUS HwGetVideoChildDescriptor(
    IN      PVOID                   HwDeviceExtension,
    IN      PVIDEO_CHILD_ENUM_INFO  ChildEnumInfo,
    OUT     PVIDEO_CHILD_TYPE       pChildType,
    OUT     PVOID                   pChildDescriptor,
    OUT     PULONG                  pUId,
    OUT     PULONG                  pUnused)
{
    UNREFERENCED_PARAMETER(HwDeviceExtension);
    UNREFERENCED_PARAMETER(ChildEnumInfo);
    UNREFERENCED_PARAMETER(pChildType);
    UNREFERENCED_PARAMETER(pChildDescriptor);
    UNREFERENCED_PARAMETER(pUId);
    UNREFERENCED_PARAMETER(pUnused);

    return ERROR_NO_MORE_DEVICES;
}

/// <summary>
/// The main entry point for the video miniport.
/// </summary>
ULONG DriverEntry(
    PVOID                   Context1,
    PVOID                   Context2)
{    
    VIDEO_HW_INITIALIZATION_DATA initialisationData;
    VideoPortZeroMemory(&initialisationData, sizeof(VIDEO_HW_INITIALIZATION_DATA));

    initialisationData.HwInitDataSize               =   sizeof(VIDEO_HW_INITIALIZATION_DATA);
    initialisationData.HwFindAdapter                =   &HwFindAdapter;
    initialisationData.HwInitialize                 =   &HwInitialize;
    initialisationData.HwStartIO                    =   &HwStartIO;
    initialisationData.HwResetHw                    =   &HwResetHw;
    initialisationData.HwInterrupt                  =   &HwInterrupt;
    initialisationData.HwGetPowerState              =   &HwGetPowerState;
    initialisationData.HwSetPowerState              =   &HwSetPowerState;
    initialisationData.HwGetVideoChildDescriptor    =   &HwGetVideoChildDescriptor;
    initialisationData.HwLegacyResourceList         =   NULL;
    initialisationData.HwLegacyResourceCount        =   0;
    initialisationData.HwDeviceExtensionSize        =   0;
    initialisationData.AdapterInterfaceType         =   0;

    return VideoPortInitialize(Context1, Context2, &initialisationData, NULL);
}
