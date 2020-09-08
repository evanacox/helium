cmake_minimum_required (VERSION 3.15)
project (helium-as)

# Create the executable
add_executable (helium-as main.cc helium_as.cc)

# Enable C++17
set_target_properties (helium-as PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
)
