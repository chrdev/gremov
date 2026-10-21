# Build noeject.exe x64 or x86. VS2026 nmake.
# chrdev, 0BSD.


.SUFFIXES : .c


# Paths
# =============================================================================

N = gremov
B = build
O = $B\$(PLATFORM)
T = $O\$N.exe
R = $N-$(PLATFORM)
C = $B\$R


# Flags
# =============================================================================

CC = cl

CFLAGS = $(CFLAGS) /nologo \
	/TC /W4 /WX /GS- \
	/O1 /Oi /Gy /GF /Zc:inline \
	/DNDEBUG /D_CONSOLE /D_UNICODE /DUNICODE \

LDFLAGS = $(LDFLAGS) /NOLOGO \
	/WX \
	/FIXED \
	/NODEFAULTLIB \
	/ENTRY:entry \

LDLIBS = kernel32.lib shell32.lib Shlwapi.lib \
	setupapi.lib \

!IF "$(PLATFORM)" == "x86"
# 5.00 / Windows 2000 is not supported by Visual Stiduo 2026 link.exe.
# But we can later use editbin.exe to set 5.00, though it gives a warning.
# Subsystem 5.01 is Windows XP 32-bit
LDFLAGS_SUBSYS = /SUBSYSTEM:CONSOLE,5.01
!ELSE
# Subsystem 5.02 is Windows XP 64-bit
LDFLAGS_SUBSYS = /SUBSYSTEM:CONSOLE,5.02
!ENDIF


# Build
# =============================================================================

all : $O $T
!IF "$(PLATFORM)" == "x86"
# Support Windows 2000
	editbin /NOLOGO /SUBSYSTEM:CONSOLE,5.00 $T
!ENDIF

$O :
	if not exist $O mkdir $O


# Pack
# =============================================================================

collect : all
	if exist $C rmdir /s /q $C
	mkdir $C
	copy /y $T $C
	copy /y removal_overrides.xml $C
	copy /y LICENSE* $C
	copy /y README.* $C

release : collect
	cd $B
	7z a -mx -myx -mtr- -stl -sdel $R.7z $R
	cd ..


# Inferences
# =============================================================================

{}.c{$O}.exe:
	$(CC) $(CFLAGS) /Fo$O\ /Fe$@ $< /link $(LDFLAGS) $(LDFLAGS_SUBSYS) $(LDLIBS)


# Maintenance
# =============================================================================

clean:
	if exist $O rmdir /s /q $O
	if exist $B\$R.7z del /q $B\$R.7z

cleanall:
	if exist $B rmdir /s /q $B
