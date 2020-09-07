cmake_minimum_required (VERSION 3.15)
project (helium)

# Create the static library
add_library (helium STATIC vm.c)

# Make the include directory public
target_include_directories (helium PUBLIC ../include)

# enable C11
set_target_properties (helium PROPERTIES
    C_STANDARD 11
)