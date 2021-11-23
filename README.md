# Asteroids
The clone of game Asteroids.<br>
I wrote this game to practise using Git and CMake.

### Compiling
I hope it is straightforward to compile this on both Windows and Linux (other operating systems are not supported). I tested this on Windows 10 and Ubuntu 20.04.3 LTS.

On both systems install CMake.

* Windows <br>
    Install Visual Studio 2019/2022.<br>
    Install libraries: SDL2 and SDL2_image.<br>
    Run CMake to create Visual Studio Solution.

* Linux <br>
    Install pkg-config and cmake-data.<br>
    Install libraries: SDL2 and SDL2_image.<br>
    Run CMake to create Makefile.<br>
    Run make to create the executable.

### Player movement

|  Movement  |   Binding   |
| ---------  | ----------- |
| Accelerate | Arrow Up    |
| Decelerate | Arrow Down  |
| Turn Left  | Arrow Left  |
| Turn Right | Arrow Right |
|   Shoot    |     X       |
|   Quit     |   Escape    |

### License
This code is distributed under GPL 3.0 license.