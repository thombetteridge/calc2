@echo off
make
link  /OUT:build2\\calc2.exe build2\\source\\calc2.obj build2\\source\\main.obj  build2\\libimgui.lib build2\\librlimgui.lib lib\\raylib.lib kernel32.lib user32.lib shell32.lib winmm.lib gdi32.lib opengl32.lib
"build2\calc2.exe"