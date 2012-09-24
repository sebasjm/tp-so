#include <winsock2.h>
#include <stddef.h>     /* _threadid variable */
#include <process.h>    /* _beginthread, _endthread */
#include <time.h>       /* time, _ctime */
#include "winsock.h"

/*
enum StateConstants {sckClosed, sckOpen, sckListening, sckConnectionPending, sckResolvingHost, sckHostResolved, sckConnecting, sckConnected, sckClosing, sckError}; //todos los estados posibles que puede tener el socket
enum SocketMode {sckNone, sckServer, sckClient};
enum ProtocolConstants {sckTCPProtocol, sckUDPProtocol};
*/

	void winsock::setRemoteHostIP(char*parNewValue){
		vRemoteHostIP = 0;
		return;
	}
	char* winsock::getRemoteHostIP(){
		return "127.0.0.1";
	}

	void winsock::setRemoteHost(char*parNewValue){
		//vRemoteIP = 0;
		return;
	}
	char* winsock::getRemoteHost(){
		return "localhost";
	}

	void winsock::setRemotePort(int*parNewValue){
		//vRemoteIP = 0;
		return;
	}
	int winsock::getRemotePort(){
		return vRemotePort;
	}

	void winsock::setProtocol(ProtocolConstants parNewValue){
		vProtocol = parNewValue;
		return;
	}
	ProtocolConstants winsock::getProtocol(){
		return vProtocol;
	}

	StateConstants winsock::state(){
		return vState;
	}


	//----------------------------------------------//
	winsock::winsock(void(*parDataArrival)(winsock*, int,char*), void(*parNewConnection)(winsock*, winsock*), void(*parConnectionLost)(winsock*, int)) {
		vMode = sckNone; //el modo del webserver, cliente/servidor
		vProtocol = sckTCPProtocol;
		vLocalPort = 0; //puerto del webserver
		vState = sckClosed; 
		vSocket = 0;
		vMaxConnections = 100;
		vRemoteHostIP = 0;
		vRemotePort = 0;
		//vector<winsock> vClients;

		threadConnection = 0;
		threadListen = 0;

		vClossing = 0;
		FD_ZERO(&fd_read);

		vAddrLen = sizeof(struct sockaddr_in);
		WSAStartup(MAKEWORD(2,2), &wsaData);
		//vSocket = socket(AF_INET, SOCK_STREAM, 0);
		vSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		vAddr.sin_family = AF_INET;
		pfDataArrival = parDataArrival;
		pfNewConnection = parNewConnection;
		pfConnectionLost = parConnectionLost;
		return;
	}

	int winsock::Listen(){
		switch(vMode){
			case sckNone:
				vMode = sckServer;
				vAddr.sin_addr.s_addr=INADDR_ANY;
				//Inicializar estructura para clientes
				break;
		}
		vAddr.sin_port = htons(vLocalPort);
		bind(vSocket, (struct sockaddr*)&vAddr,sizeof(struct sockaddr));
		listen(vSocket, vMaxConnections);
		FD_SET(vSocket, &fd_read);
		//Ejecutar un while con el select para detectar eventos (en un hilo/proceso aparte?)
		//Nuevo hilo que ejecute Listening
		Listening();
		return 0;
	}

	void winsock::Listening(){
		int iret = 0;
		vState = sckListening;
		while(vClossing != 1){
			//FD_ZERO(&fd_tmp);
			//FD_SET(vSocket, &fd_tmp);
			fd_tmp = fd_read;
			iret = select(vSocket + 1, &fd_tmp, NULL, NULL, NULL); /*ESPERAR EVENTOS*/
			if(vSocket > 0){
				if(FD_ISSET(vSocket, &fd_tmp)) fNewConnection();
			}
		}
		return;
	}

	void winsock::fNewConnection(){
		//Crear un objeto winsock y aceptar la conexion usando el metodo Accept, pasando por parametro el descriptor del socket que recibio la peticion de conexion(vSocket)
		winsock *newWinsock = new winsock(pfDataArrival, pfNewConnection, pfConnectionLost);
		(*newWinsock).Accept(vSocket);
		(*pfNewConnection)(this, newWinsock);
		return;
	}

	int winsock::Listen(int parPort){
		vLocalPort = parPort;
		Listen();
		return 0;
	}

	int winsock::Accept(int parRequestID){
		sockaddr remote;
		vSocket = accept(parRequestID, &remote, &vAddrLen);
		FD_SET(vSocket, &fd_read); /*AGREGAR PARA DETECTAR EVENTOS*/
		//Crear thread para select
		WaitEvents();
		return 0;
	}

	void winsock::WaitEvents(){
		int iret = 0;
		vState = sckListening;
		char*sBuff;
		while(vClossing != 1){
			fd_tmp = fd_read;
			iret = select(vSocket + 1, &fd_tmp, NULL, NULL, NULL); /*ESPERAR EVENTOS*/
			if(vSocket > 0){
				if(FD_ISSET(vSocket, &fd_tmp)){
					sBuff = (char*)malloc(101);
					iret = recv(vSocket, sBuff, 100, NULL);
					if(iret > 0){
						(*pfDataArrival)(this, iret, sBuff);
					}else{
						//Se perdio la conexion
						(*pfConnectionLost)(this, 1);
					}
				}
			}
		}
		return;
	}

	int winsock::Connect(){
		vAddr.sin_port = htons(vRemotePort);
		vAddr.sin_addr.s_addr = vRemoteHostIP;
		memset(&(vAddr.sin_zero), 0, 8);
		vState = sckConnecting;
		if(connect(vSocket, (struct sockaddr*)&(vAddr), sizeof(struct sockaddr)) == -1){
			//Error
			vState = sckClosed;
			return -1;
		}

		return 0;
	}

	int winsock::Close(){
		vClossing = 1;
		switch(vState){
			case sckNone:
				break;
			case sckListening:
				break;
		}
		vMode = sckNone;
		return 0;
	}

	int winsock::SendData(const char*parData, int parDataLen){
		send((SOCKET)vSocket, parData, parDataLen, NULL);
		return 0;
	}
