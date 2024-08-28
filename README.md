# GREMOV - Generate Removal Overrides

## Introduction

On Windows desktop, there could be internal devices shown as removables in the corner. It's annoying that an unnecessary eject icon always shows. And when we try to eject a USB disk, it may be tedious to click on the right item.

GREMOV helps to solve this. It generates a .reg file which set present removable devices to non-removable. This reg file must be merged on each system boot to take effect.

## Usage

On command prompt (run as Administrator to write to system32 dir and schedule)
```
gremov.exe %windir%\system32\removal_overrides.reg
schtasks.exe /create /tn removal_overrides /xml removal_overrides.xml
```
Reboot

## Tech Notes

Here 3 override methods are introduced.

#### Method 1. Works on Windows XP and Vista:

Use registry value RemovalPolicy. It sits besides Capabilities under HKLM\SYSTEM\CurrentControlSet\Enum\{Device Instance Id}. "Device Instance Id" can be fonud at Device Manager, device Properties -> Details -> Device Instance Id.

RemovalPolicy is of type DWORD, and can have one of these values:

1 - No removal. We want this to hide from eject menu.
2 - Orderly removal. This is the normal value to cause a device shown on eject menu.
3 - Surprise removal. For devices like keyboards and mice.

This method works for USB devices, but no so much PCI devices.

API funcitons SetupDiGetDeviceRegistryProperty() and SetupDiSetDeviceRegistryProperty() with Property of SPDRP_REMOVAL_POLICY_OVERRIDE can respectively get and set RemovalPolicy value. CM_Get_DevNode_Registry_Property() and CM_Set_DevNode_Registry_Property() with ulProperty of CM_DRP_REMOVAL_POLICY_OVERRIDE have the same usage.

On modern versions of Windows, these functions still read / write RemovalPolicy vaule correctly. But the value itself has no effect at all.

#### Method 2. Works on Windows 7 and onwards

Use DeviceOverrides registry key which is under HKLM\SYSTEM\CurrentControlSet\Control. This method is well [documented at this link](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/deviceoverrides-registry-key).

Like method 1, This method works for USB devices, but no so much PCI devices.

#### Method 3. Universally effective

Overwirte Capabilities registry value at boot. Reset its 3rd bit.

```
cfgmgr32.h

#define CM_DEVCAP_REMOVABLE (0x00000004)
```

This method is a hack, but universally effective for all devices, on all platforms. Gremov helps to generate the reg file needed to use this method.

## Link

[gremov on github](https://github.com/chrdev/gremov)
