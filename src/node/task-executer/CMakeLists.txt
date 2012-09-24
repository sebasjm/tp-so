add_library (ThreadLocal threadlocal.c)
target_link_libraries (ThreadLocal pthread)
target_link_libraries (ThreadLocal MD5)

add_library (TaskExecuter SHARED taskExecuter.c)
link_directories(${CMAKE_BINARY_DIR}/task-executer)
target_link_libraries (TaskExecuter ThreadLocal)
target_link_libraries (TaskExecuter Log)
