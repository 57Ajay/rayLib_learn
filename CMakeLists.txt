cmake_minimum_required(VERSION 3.20)

# Project name
project(raylibLearn)

# Set the C standard
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Enable compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()


# Add the executable
add_executable(${PROJECT_NAME} src/main.c)

# Set output directory
set_target_properties(${PROJECT_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Adding dependency: raylib
find_package(raylib CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} ${raylib_LIBRARIES})

# --- Handle Resource Files ---
# Create the output directory for resources if it doesn't exist
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/resources")

# Copy the entire resources directory to the output directory
file(COPY "${CMAKE_SOURCE_DIR}/resources" DESTINATION "${CMAKE_BINARY_DIR}/bin")
