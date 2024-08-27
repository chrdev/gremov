gcc -Wall -s -O2 -o gremov.exe gremov.c -lKernel32 -lshell32 -lsetupapi -mwin32 -mconsole -nostdlib -e entry
upx -9 gremov.exe
