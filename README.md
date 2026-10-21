# GREMOV - Generate Removal Overrides

Hide items from the taskbar *Safely Remove Hardware And Eject Media* menu. For Windows XP and 2000.

This repo has been superseded by [NoEject](https://github.com/chrdev/noeject), which works on Windows Vista and later.

From this

![Original Eject Menu](https://github.com/chrdev/noeject/pic/original.png)

To this

![Hidden by GREMOV](https://github.com/chrdev/noeject/pic/noejected.png)

## Usage

On command prompt:

### Step 1. View reg file.
```
gremov.exe
```

### Step 2. Redirect stdout to save the .reg file.
```
gremov.exe > %WINDIR%\removal_overrides.reg
```

### Step 3. Schedule the task.

Example:

    Command: "regedit.exe /s %WINDIR%\removal_overrides.reg"
    Run At:  system startup
    User:    SYSTEM

Vista and later can use
```
schtasks.exe /create /tn removal_overrides /xml removal_overrides.xml
```

### Step 4. Reboot to take effect.

## Tech Notes

Three removal overriding methods:

#### Method 1. Works on Windows XP and Vista:

Use RemovalPolicy registry key. The path is HKLM\SYSTEM\CurrentControlSet\Enum\{Device Instance Id}. The device instance id can be fonud at Device Manager, device Properties -> Details -> Device Instance Id.

RemovalPolicy is of type DWORD, and can have one of these values:

1 - No removal. We want this to hide the eject menu item.  
2 - Orderly removal. This is the normal value to cause a device shown on eject menu.  
3 - Surprise removal. For devices like keyboards and mice.

This method only works for USB devices, NOT for PCI devices.

API funcitons SetupDiGetDeviceRegistryProperty() and SetupDiSetDeviceRegistryProperty() with Property of SPDRP_REMOVAL_POLICY_OVERRIDE can respectively get and set RemovalPolicy value.

On modern versions of Windows, these functions still read / write RemovalPolicy vaule correctly. But the value itself has no effect.

#### Method 2. Works on Windows 7 and later

Use DeviceOverrides registry key. The path is HKLM\SYSTEM\CurrentControlSet\Control. This method is documented at [Microsoft site](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/deviceoverrides-registry-key).

Like method 1, This method only works for USB devices, NOT for PCI devices.

#### Method 3. Works on all Windows versions

Schedule a boot task to overwirte Capabilities registry key.
This method is a hack. But it's universally effective. Gremov helps to generate a .reg file for this method.

```
cfgmgr32.h
----------
#define CM_DEVCAP_REMOVABLE (0x00000004)
```

## License

This is free and unencumbered software released into the public domain. For more information, please refer to <https://unlicense.org>

## Link to this repo

[gremov on github](https://github.com/chrdev/gremov)
