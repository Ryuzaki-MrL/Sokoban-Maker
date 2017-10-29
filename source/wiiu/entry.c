#include <string.h>

#include "wiiu.h"

#include "main.h"

int __entry_menu(int argc, char* argv[]) {
    InitOSFunctionPointers();
    InitSocketFunctionPointers();
    InitACPFunctionPointers();
    InitAocFunctionPointers();
    InitAXFunctionPointers();
    InitCurlFunctionPointers();
    InitFSFunctionPointers();
    InitGX2FunctionPointers();
    InitPadScoreFunctionPointers();
    InitSysFunctionPointers();
    InitSysHIDFunctionPointers();
    InitVPadFunctionPointers();

    memoryInitialize();
    mount_sd_fat("sd");
    VPADInit();

    int ret = mainfunc();

    unmount_sd_fat("sd");
    memoryRelease();

    return ret;
}
