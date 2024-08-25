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

## Link

[gremov on github](https://github.com/chrdev/gremov)
