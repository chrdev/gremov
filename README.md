# GREMOV - Generate Removal Overrides

### This repo has been superseded by [NoEject](https://github.com/chrdev/noeject), which also supports from Windows 2000 to Windows 11.

Gremov hides items from the taskbar *Safely Remove Hardware And Eject Media* menu.

## Usage

On command prompt:

### Step 1. View the .reg file.
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

If on Vista and later:
```
schtasks.exe /create /tn removal_overrides /xml removal_overrides.xml
```

### Step 4. Reboot to take effect.

## Tech Notes (may be inaccurate, but left for references)

Three removal overriding methods:

#### Method 1. Works on Windows XP and Vista:

Use **RemovalPolicy** registry key. The path is HKLM\SYSTEM\CurrentControlSet\Enum\{Device Instance Id}. The device instance id can be fonud at Device Manager, device Properties -> Details -> Device Instance Id.

**RemovalPolicy** is a DWORD key. It's value can be:

1 - No removal. The item hides from the eject menu.  
2 - Orderly removal. The item shows in the eject menu.  
3 - Surprise removal. The item will not show in the eject menu. Like keyboards and mice.

This method only works for USB devices, NOT for PCI devices.

SetupDiGetDeviceRegistryProperty(), SetupDiSetDeviceRegistryProperty(), SPDRP_REMOVAL_POLICY_OVERRIDE can be used to get and set **RemovalPolicy**.

On Windows 7 and later, these APIs still read and write **RemovalPolicy**. But the key has no effect.

#### Method 2. Works on Windows 7 and later

Use **DeviceOverrides** registry key. It's documented at [Microsoft site](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/deviceoverrides-registry-key).

In practice, I find it can be hard to set up right. When the obvious devices are hidden, another vague device may start to appear in the eject menu.

#### Method 3. Works on all Windows versions

Schedule a boot task to overwirte **Capabilities** registry key.
This method is a hack. But it's universally effective. Gremov helps to generate a .reg file for this method.

## License

This is free and unencumbered software released into the public domain. For more information, please refer to <https://unlicense.org>

## Link to this repo

[gremov on github](https://github.com/chrdev/gremov)
