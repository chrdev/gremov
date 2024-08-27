gcc -Wall -s -O2 -o gremov-x86.exe gremov.c -lKernel32 -lshell32 -lsetupapi -mwin32 -mconsole -nostdlib -e _entry
upx -9 gremov-x86.exe
