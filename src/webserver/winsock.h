#include <iostream>
#include <vector>
#include <winsock2.h>
#ifndef winsockH
#define winsockH

enum StateConstants {sckClosed, sckOpen, sckListening, sckConnectionPending, sckResolvingHost, sckHostResolved, sckConnecting, sckConnected, sckClosing, sckError}; //todos los estados posibles que puede tener el socket
enum SocketMode {sckNone, sckServer, sckClient};
enum ProtocolConstants {sckTCPProtocol, sckUDPProtocol};

class winsock {
private:
	SocketMode vMode; //el modo del webserver, cliente/servidor
	ProtocolConstants vProtocol;
	int vLocalPort; //puerto del webserver
	StateConstants vState; 
	int vSocket;
	WSADATA wsaData;
	int vMaxConnections;
	ULONG vRemoteHostIP;
	int vRemotePort;
	//vector<winsock> vClients;

	int vAddrLen;
	struct sockaddr_in vAddr;
	fd_set fd_read, fd_tmp;

	int threadConnection;
	int threadListen;

	int vClossing;

	/*EVENTOS(punteros a funciones)*/
	void(*pfDataArrival)(winsock*, int, char*);
	void(*pfNewConnection)(winsock*, winsock*);
	void(*pfConnectionLost)(winsock*, int);

public:
	void winsock::setRemoteHostIP(char*parNewValue);
	char* winsock::getRemoteHostIP();
	void winsock::setRemoteHost(char*parNewValue);
	char* winsock::getRemoteHost();
	void winsock::setRemotePort(int*parNewValue);
	int winsock::getRemotePort();
	void winsock::setProtocol(ProtocolConstants parNewValue);
	ProtocolConstants winsock::getProtocol();
	StateConstants winsock::state();

	//----------------------------------------------//
	winsock::winsock(void(*parDataArrival)(winsock*, int, char*), void(*parNewConnection)(winsock*, winsock*), void(*parConnectionLost)(winsock*, int));
	int winsock::Listen();
	void winsock::Listening();
	void winsock::fNewConnection();
	int winsock::Listen(int parPort);
	int winsock::Accept(int parRequestID);
	void winsock::WaitEvents();
	int winsock::Connect();
	int winsock::Close();
	int winsock::SendData(const char*parData, int parDataLen);
};

#endif
