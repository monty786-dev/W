cmake_minimum_required(VERSION 3.20)
project(WinShell C)
set(CMAKE_C_STANDARD 11)

add_executable(winshell
    src/main.c src/config.c src/theme.c src/taskbar.c
    src/startmenu.c src/skin.c src/ipc.c)
target_link_libraries(winshell PRIVATE
    user32 gdi32 comctl32 shell32 shlwapi advapi32 msimg32 ole32 gdiplus)

add_executable(winshell-setup src/installer.c res/winshell.rc)
target_link_libraries(winshell-setup PRIVATE user32 advapi32 shell32 ole32)

add_executable(winshell-start src/launcher.c)
target_link_libraries(winshell-start PRIVATE user32 shell32)

# MinGW GUI subsystem: no console window popping in Wine
set_target_properties(winshell winshell-setup winshell-start PROPERTIES
    LINK_FLAGS "-mwindows -s -static -static-libgcc")
