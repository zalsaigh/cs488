# Gather all cpp files in this directory
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(A0 ${SOURCES})

# ---- Dependencies (resolved via the vcpkg toolchain) ----
find_package(glfw3 CONFIG REQUIRED)      # glfw
find_package(imgui  CONFIG REQUIRED)     # imgui::imgui
find_package(OpenGL REQUIRED)            # OpenGL::GL
# (No need to find gl3w here unless A0 itself includes <GL/gl3w.h>;
# cs488-framework already links gl3w.)

# ---- Includes ----
target_include_directories(A0 PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/../shared"
  "${CMAKE_CURRENT_SOURCE_DIR}/../shared/include"
)

# If any A0 source uses ImGui OpenGL3 backend directly, ensure the loader define:
target_compile_definitions(A0 PRIVATE IMGUI_IMPL_OPENGL_LOADER_GL3W)

# ---- Link (platform-agnostic; GLFW brings in what it needs) ----
target_link_libraries(A0 PRIVATE
  cs488-framework
  imgui::imgui
  glfw
  OpenGL::GL
)

# ---- Output location ----
set_target_properties(A0 PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

# ---- Helpful note if the framework target is missing ----
if(NOT TARGET cs488-framework)
  message(STATUS "Note: A0 requires cs488-framework to be built first.")
endif()
