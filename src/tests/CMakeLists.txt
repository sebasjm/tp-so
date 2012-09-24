# ---------------------------------------------------
# Zona de pruebas automatizadas de C
# ---------------------------------------------------
crear_ejecutable (testIMC IMC)
add_test(IMCAssign testIMC.bin)
set_tests_properties (IMCAssign PROPERTIES PASS_REGULAR_EXPRESSION "ok")

crear_ejecutable (testDC-MigrationMemory IPCDaemon XmlConfig)
add_test(TestDCMigrationMemory testDC-MigrationMemory.bin)

crear_ejecutable(testTaskManager IMC TaskManager)
add_test(TestTM testTaskManager.bin)

crear_ejecutable (testNodeManager IMC NodeManager)
add_test(TestNM testNodeManager.bin)


crear_ejecutable(testWM-TM WebManager IMC TaskManager)
add_test(WM-TM testWM-TM.bin)
set_tests_properties (WM-TM PROPERTIES PASS_REGULAR_EXPRESSION "ok")

crear_ejecutable(testWM WebManager IMC)
add_test(WMTest testWM.bin)

crear_ejecutable(testTaskExecuter TaskExecuter)
add_test(MultiThreadTaskExecute testTaskExecuter.bin)
set_tests_properties (MultiThreadTaskExecute PROPERTIES PASS_REGULAR_EXPRESSION "Fail")
set_tests_properties (MultiThreadTaskExecute PROPERTIES PASS_REGULAR_EXPRESSION "Success")
set_tests_properties (MultiThreadTaskExecute PROPERTIES PASS_REGULAR_EXPRESSION "Complete")

crear_ejecutable (testMD5 MD5)
add_test(EncriptacionMD5 testMD5.bin)
set_tests_properties (EncriptacionMD5 PROPERTIES PASS_REGULAR_EXPRESSION "ok")

# ---------------------------------------------------
# Zona de ejecutables no test (experimentos)
# ---------------------------------------------------

crear_ejecutable (testTrace Trace)

crear_ejecutable(testRegex)

crear_ejecutable(testListener Point Socket SocketBuilder SocketGate Listener SocketEvent TecladoConsola)

crear_ejecutable(testXmlConfig XmlConfig)

# ---------------------------------------------------
# Zona de pruebas automatizadas de C++
# ---------------------------------------------------

crear_ejecutable(testModel Job Target Task Resource Dependency)
add_test(JugarUnPocoConElModelo testModel.bin)

#IF(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "SunOS" )
#crear_ejecutable(testDB BerkeleyDB)
#add_test(BaseDeDatos testDB.bin)
#ENDIF(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "SunOS" )

crear_ejecutable(testConfiguration Configuration Job Target Task Resource Dependency)
add_test(JobXml testConfiguration.bin "../../src/tests/prueba.xml" "Job(target=Target(hash=6e023d8b8758f8f826152de77a3cc1d9,length=6,method=MD5),tasks=[Task(taskId=task1,taskDescription=\"This is the first task\",dependencies=[Dependency(name=task3),Dependency(name=task6),Dependency(name=task2),],resources=[Resource(name=cpu,value=300),Resource(name=memory,value=250MB),Resource(name=disk,value=512MB),]])" )

crear_ejecutable(testAnsiConfiguration ConfigurationAnsi)
add_test(JobXML2 testAnsiConfiguration.bin "../../src/tests/prueba.xml")

crear_ejecutable(testAnsiConfigurationTaskResult ConfigurationAnsi)
add_test(TaskResultXML testAnsiConfigurationTaskResult.bin "../../src/tests/taskResult.xml")

crear_ejecutable(testIpcStream IPCStream Point)
add_test(ComunicarProcesos testIpcStream.bin)

crear_ejecutable(testSocket Point Socket SocketBuilder SocketGate Listener SocketEvent)
#add_test(PruebaDeSockets testSocket.bin "holaaaa como te va")
#set_tests_properties (PruebaDeSockets PROPERTIES PASS_REGULAR_EXPRESSION "callate")

crear_ejecutable(testDataPack Point Socket SocketBuilder SocketGate Listener SocketEvent)
