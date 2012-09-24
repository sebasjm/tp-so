add_library (Log log.cxx)
add_library (TPL tpl.c)
add_library (StringTemplate template.cxx)
add_library (Weighted weighted.c)
add_library (Trace trace.cxx)

include_directories("/usr/include/libxml2")
add_library (XmlConfig config.cxx)
target_link_libraries (XmlConfig xml2)
