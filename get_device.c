#include <windows.h>
#include <setupapi.h>
#include <devguid.h> 
#include <regstr.h> 
#include <stdio.h>

#include "get_device.h"

#pragma comment(lib, "setupapi.lib")

HDEVINFO hDevInfo; // 设备的数据集
SP_DEVINFO_DATA DeviceInfoData; // 设备的数据集


//初始化变量，删除旧数据，格式化变量
void init_device_list() {
    if (hDevInfo != INVALID_HANDLE_VALUE)
    {
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, NULL, NULL, DIGCF_PRESENT);

    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
}


//同init_device_list()
void refresh_device_list(){
    init_device_list();
}


//通过枚举，获得设备数量
int get_devices_count(){
    int count = 0;

    while (SetupDiEnumDeviceInfo(hDevInfo, count, &DeviceInfoData))
    {
        count++;
    }

    return count;
}


//获取index的设备名称
char* get_device_name(int index){
    char portName[256] = {0};
    static char friendlyName[256] = "Joey";
    DWORD dataType, requiredSize;

    SetupDiEnumDeviceInfo(hDevInfo, index, &DeviceInfoData);

    if (SetupDiGetDeviceRegistryPropertyA(
            hDevInfo,
            &DeviceInfoData,
            SPDRP_FRIENDLYNAME,
            &dataType,
            (PBYTE)friendlyName,
            sizeof(friendlyName),
            &requiredSize))
    {

    }
    else
    {
        friendlyName[0] = '\0'; // 有些设备可能没有友好名
        return '\0';
    }


    return friendlyName;
}


int get_device_com_number(int index){
    int maxCount = get_devices_count();

    if (index< 0 || index > maxCount) return -1;

    char* deviceFriendlyName = get_device_name(index);
    char* result = strstr(deviceFriendlyName,"(COM");

    if (result == NULL) return -2;

    int str_position = (int)(result-deviceFriendlyName);
    int comNumber = deviceFriendlyName[str_position+4] - '0';

    return comNumber;
}