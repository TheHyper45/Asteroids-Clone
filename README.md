# Asteroids
The clone of game Asteroids.<br>
I wrote this game to practise using Git and CMake.

### Compiling
I hope it is straightforward to compile this on both Windows and Linux (other operating systems are not supported).<br>
I tested this on Windows 10 21H1 and Ubuntu 20.04.3 LTS.

* Windows
    * Install cmake and Visual Studio 2019/2022.
    * Download development libraries: SDL2 and SDL2_image and put them where you want.
    * Put paths to directories of these libraries in your PATH environment variable.
    * Run cmake to create Visual Studio Solution.

* Linux
    * Install make, cmake, cmake-data and pkg-config.
    * Install development libraries: SDL2 (libsdl2-dev) and SDL2_image (libsdl2-image-dev).
    * Run CMake to create Makefile.
    * Run make to create the executable.

### Game information

|   Action   |   Binding   |
| ---------- | ----------- |
| Accelerate | Arrow Up    |
| Decelerate | Arrow Down  |
| Turn Left  | Arrow Left  |
| Turn Right | Arrow Right |
|   Shoot    |     X       |
|   Quit     |   Escape    |

Blue objects can be touched safely (they are used as "particle effect" that appear after destroying a rock, UFO or player).

### License
This code is distributed under GPL 3.0 license.