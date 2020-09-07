cmake_minimum_required (VERSION 3.15)
project (helium)

# Compile the library
add_subdirectory (lib)

# Compile the executable using the library
add_subdirectory (src)

# link the executable with the library
target_link_libraries (helium-as PRIVATE helium)