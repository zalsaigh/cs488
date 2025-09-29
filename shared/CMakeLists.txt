# Finds (via vcpkg toolchain)
find_package(glfw3 CONFIG REQUIRED)      # target: glfw
find_package(imgui  CONFIG REQUIRED)     # target: imgui::imgui
find_package(glm    CONFIG REQUIRED)     # target: glm
find_package(OpenGL REQUIRED)            # target: OpenGL::GL
find_package(gl3w   CONFIG REQUIRED)     # target: gl3w
find_package(lua    CONFIG REQUIRED)     # target: lua

set(INCLUDE_DIRS
  "${CMAKE_SOURCE_DIR}/shared"
  "${CMAKE_SOURCE_DIR}/shared/include"
)

file(GLOB CS488_SOURCES "${CMAKE_SOURCE_DIR}/shared/cs488-framework/*.cpp")
add_library(cs488-framework STATIC ${CS488_SOURCES})

# Tell ImGui which GL loader you're using
target_compile_definitions(cs488-framework PRIVATE IMGUI_IMPL_OPENGL_LOADER_GL3W)

target_include_directories(cs488-framework PUBLIC ${INCLUDE_DIRS})

target_link_libraries(cs488-framework PUBLIC
  imgui::imgui
  glfw
  glm::glm
  OpenGL::GL
  unofficial::gl3w::gl3w
  lua
)

# If the port exports backend interface targets, link them (no-op if absent)
foreach(_t IN ITEMS imgui::glfw-binding imgui::opengl3-binding)
  if(TARGET ${_t})
    target_link_libraries(cs488-framework PRIVATE ${_t})
  endif()
endforeach()

set_target_properties(cs488-framework PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
)
