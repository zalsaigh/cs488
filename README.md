# CS488 Winter 2024 Project Code

---

## Dependencies
* VCPKG
    * If you're using Visual Studio: https://learn.microsoft.com/en-ca/vcpkg/get_started/get-started-vs?pivots=shell-powershell
    * Otherwise: https://learn.microsoft.com/en-ca/vcpkg/get_started/overview#get-started-with-vcpkg
* OpenGL 3.2+
* GLFW
    * http://www.glfw.org/
* Lua
    * http://www.lua.org/
* GLM
    * http://glm.g-truc.net/0.9.7/index.html
* ImGui
    * https://github.com/ocornut/imgui
---

## Building Projects
We use CMake as our cross-platform build system. First you will need to fetch all
dependencies from `vcpkg` by doing `vcpkg install`. This will use the existing `vcpkg.json`
to install all dependencies.

If you're using Visual Studio, you can now click on the green arrow to build and run your
project. Otherwise, the equivalent CMake commands are ChatGPT-able, but they're roughly:
```
    $ mkdir build
    $ cd build
    $ cmake .. -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
    $ cmake --build .
```
This will generate the build files in a `build` directory, and then build the project.
You can then run your project executable from the `build` directory.

```
    $ ./build/A0.exe
```

---
## Windows
Sorry for all of the hardcore Microsoft fans out there.  We have not had time to test the build system on Windows yet. Currently our build steps work for OSX and Linux, but all the steps should be the same on Windows, except you will need different libraries to link against when building your project executables.  Some good news is that premake4 can output a Visual Studio .sln file by running:

    $ premake4 vs2013

 This should point you in the general direction.
