# CS488 Winter 2024 Project Code

---

## Dependencies
* OpenGL 3.2+
* GLFW
    * http://www.glfw.org/
* Lua
    * http://www.lua.org/
* Premake4
    * https://github.com/premake/premake-4.x/wiki
    * http://premake.github.io/download.html
* GLM
    * http://glm.g-truc.net/0.9.7/index.html
* ImGui
    * https://github.com/ocornut/imgui


---

## Building Projects
We use **premake4** as our cross-platform build system. First you will need to build all
the static libraries that the projects depend on. To build the libraries, open up a
terminal, and **cd** to the top level of the CS488 project directory and then run the
following:

    $ premake4 gmake
    $ make

This will build the following static libraries, and place them in the top level **lib**
folder of your cs488 project directory.
* libcs488-framework.a
* libglfw3.a
* libimgui.a

Next we can build a specific project.  To do this, **cd** into one of the project folders,
say **A0** for example, and run the following terminal commands in order to compile the A0 executable using all .cpp files in the A0 directory:

    $ cd A0/
    $ premake4 gmake
    $ make


----

## Windows
Sorry for all of the hardcore Microsoft fans out there.  We have not had time to test the build system on Windows yet. Currently our build steps work for OSX and Linux, but all the steps should be the same on Windows, except you will need different libraries to link against when building your project executables.  Some good news is that premake4 can output a Visual Studio .sln file by running:

    $ premake4 vs2013

 This should point you in the general direction.
