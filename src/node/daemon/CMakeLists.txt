add_library (HandShake handshake.cxx)
target_link_libraries (HandShake Socket)

add_library (DaemonCommons common.cxx)

add_library (DaemonCommands commands.cxx)

add_library (NetDisconvery netDiscovery.cxx)
target_link_libraries (NetDisconvery DaemonCommons)

add_library (ADMTask admTask.c)
target_link_libraries (ADMTask TaskExecuter)

add_library (Migration migration.c)
target_link_libraries (Migration XmlConfig)
target_link_libraries (Migration Weighted)

add_library (NetworkManager networkManager.cxx)
target_link_libraries (NetworkManager ADMTask)
target_link_libraries (NetworkManager NetDisconvery)
target_link_libraries (NetworkManager Migration)
target_link_libraries (NetworkManager Listener)
target_link_libraries (NetworkManager Listener)
target_link_libraries (NetworkManager DaemonCommons)
target_link_libraries (NetworkManager DaemonCommands)
target_link_libraries (NetworkManager IPCDaemon)

crear_ejecutable(daemon Listener NetDisconvery NetworkManager SocketBuilder XmlConfig HandShake)


