add_library (ConectorCliente conectorCliente.cxx)
target_link_libraries (ConectorCliente Socket)
target_link_libraries (ConectorCliente SocketEvent)
target_link_libraries (ConectorCliente Point)

#IF(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "SunOS" )
#add_library (BerkeleyDB db.cxx)
#target_link_libraries (BerkeleyDB db)

#add_library (Resolvers resolvers.cxx)
#target_link_libraries (Resolvers Configuration)
#target_link_libraries (Resolvers HandShake)
#target_link_libraries (Resolvers StringTemplate)
#target_link_libraries (Resolvers Job)
#target_link_libraries (Resolvers WebConnector)
#target_link_libraries (Resolvers BerkeleyDB)

#crear_ejecutable(webserver Point Socket SocketBuilder SocketGate Listener XmlConfig SocketEvent ConectorCliente Resolvers WebConnector)
#ENDIF(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "SunOS" )
