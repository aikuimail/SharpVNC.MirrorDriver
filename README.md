![Project Logo](/resources/main_image.png)

SharpVNC Mirror Driver is a Windows 2000 Display Driver Model (XDDM) [mirror driver](https://docs.microsoft.com/en-us/windows-hardware/drivers/display/mirror-drivers) for the Graphics Device Interface (GDI). Mirror drivers enable **fast and efficient** access to the Windows framebuffer, although were made obsolete from Windows 8 onwards. SharpVNC Core has **built-in** support for the mirror driver, and it can also be licensed in binary or source form depending on the use cases required.

## Getting Started

It is recommended that all development of the mirror driver is performed on a virtual machine.

### Compiling

You should use Microsoft Visual Studio 2019 for development and compiling. Windows Driver Development Kit (WDK) 7.0, the Windows Software Development Kit (SDK) 8.1 and Microsoft Visual Studio Build Tools 2013 are required in order to successfully compile.
