add_library (Shell shell.cxx)
target_link_libraries (Shell ConectorCliente Listener TecladoConsola ShellCommands)

add_library (ShellCommands commands.cxx)
target_link_libraries (ShellCommands XmlConfig SocketBuilder HandShake)

# Point Socket SocketBuilder SocketGate Listener SocketEvent  Resolvers
crear_ejecutable(interface Shell XmlConfig)
