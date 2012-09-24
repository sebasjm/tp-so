add_subdirectory (ansi)

add_library (IPCStream ipcstream.cxx)
add_library (Point point.cxx)

add_library (Socket socket.cxx)
target_link_libraries (Socket Point)
target_link_libraries (Socket TPL)

add_library (SocketGate socketGate.cxx)
target_link_libraries (SocketGate Socket)

add_library (SocketBuilder socketBuilder.cxx)
target_link_libraries (SocketBuilder SocketGate)

add_library (SocketEvent socketEvent.cxx)
target_link_libraries (SocketEvent SocketGate)

add_library (Listener listener.cxx)
target_link_libraries (Listener SocketEvent)
#target_link_libraries (Listener Socket)

add_library (TecladoConsola tecladoConsola.cxx)
target_link_libraries (TecladoConsola Point)


add_library (IPCDaemon ipcDaemon.cxx)
target_link_libraries (IPCDaemon HandShake)
target_link_libraries (IPCDaemon SocketBuilder)
