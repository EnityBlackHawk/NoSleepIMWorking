cmake_minimum_required(VERSION 3.25)
project(NoSleepIMWorking LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} WIN32
    src/main.cpp
    src/Application.cpp
    src/Application.h
    src/MonitorDetector.cpp
    src/MonitorDetector.h
    src/PowerManager.cpp
    src/PowerManager.h
    src/TrayIcon.cpp
    src/TrayIcon.h
    
)

target_compile_definitions(${PROJECT_NAME}  PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_compile_options(${PROJECT_NAME}  PRIVATE /W4 /permissive-)

target_link_libraries(${PROJECT_NAME}  PRIVATE
    user32
    shell32
    advapi32
    powrprof
)
