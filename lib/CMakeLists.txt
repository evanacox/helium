cmake_minimum_required (VERSION 3.15)
project (helium)

# Create the static library
add_library (helium STATIC 
    helium/memory.c
    helium/module.c
    helium/value.c
    helium/vector.c 
    helium/vm.c
)

# Make the include directory public
target_include_directories (helium PUBLIC ../include)

# enable C11 and disable GNU extensions
set_target_properties (helium PROPERTIES
    C_STANDARD 11
    C_EXTENSIONS ON
)
