add_library (NodeManager nodeManager.c)
target_link_libraries (NodeManager Weighted )
target_link_libraries (NodeManager XmlConfig )

add_library (WebManager webManager.c)
target_link_libraries (WebManager WebConnector)
target_link_libraries (WebManager StringTemplate)
target_link_libraries (WebManager XmlConfig)
target_link_libraries (WebManager ConfigurationAnsi)

add_library (TaskManager taskManager.c)

add_library (IMC imc.c)
target_link_libraries (IMC pthread)

add_library (DC dc.c)
target_link_libraries (DC NodeManager)
target_link_libraries (DC WebManager)
target_link_libraries (DC TaskManager)
target_link_libraries (DC IPCDaemon)
target_link_libraries (DC DaemonCommons)
target_link_libraries (DC IMC)

crear_ejecutable(dc_main DC NetworkManager HandShake TaskManager)
