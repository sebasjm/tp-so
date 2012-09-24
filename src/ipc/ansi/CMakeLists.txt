add_library(WebConnector webConnector.cxx)
target_link_libraries (WebConnector Socket)
target_link_libraries (WebConnector Point)
target_link_libraries (WebConnector SocketBuilder)
target_link_libraries (WebConnector SocketGate)

