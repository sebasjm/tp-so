add_subdirectory (ansi)

include_directories("/usr/include/libxml2")
add_library (Configuration configuration.cxx)
target_link_libraries (Configuration xml2)
target_link_libraries (Configuration Task)
target_link_libraries (Configuration Target)
target_link_libraries (Configuration Job)
target_link_libraries (Configuration Dependency)
target_link_libraries (Configuration Resource)
target_link_libraries (Configuration Result)
target_link_libraries (Configuration TaskResult)

