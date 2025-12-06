#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <ws2tcpip.h>
#include <Windows.h>

#define PORT "80"   //communication over port 80 -> web Server port
const char severAddr[] = "127.0.0.1";


int main() {
	SOCKET listenSocket = INVALID_SOCKET; 


	//startup stuff
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);   //Version is represented as two bytes. one byte for both major and minor versions. (2.2  is newest version)
	if (WSAStartup(DllVersion, &wsaData) != 0)   //ensures version and ducks are in line 
	{
		std::cerr << "WSAStartup Fail with code: " << WSAStartup(DllVersion, &wsaData);
		ExitProcess(EXIT_FAILURE);
	}

	struct addrinfo* results = NULL, hints;

	ZeroMemory(&hints, sizeof(hints) );

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;   //For bind 

	int code = getaddrinfo(severAddr, PORT, &hints, &results);
	if (code != 0)  //failed to gen addr info???
	{
		std::cerr << "failed to gen addr info w/ code: " << code;
		WSACleanup();
		ExitProcess(EXIT_FAILURE);
	}

	//Create Socket
	listenSocket = socket(results->ai_family, results->ai_socktype, results->ai_protocol);  //Family : AF_INET -> lets use the internet      Stream Type : SOCK_STREAM -> let's use tcp       Protocol: IPPROTO_TCP -> . Using TCP, microsoft says so in docs?
	if (listenSocket < 0)
	{
		std::cerr << "socket init Fail with code: " << listenSocket;
		WSACleanup();
		ExitProcess(EXIT_FAILURE);
	}
	
	code = bind(listenSocket, results->ai_addr, results->ai_addrlen);


	//Bind the server socket to an IP and port
	if (code != 0)
	{
		//hcf
		std::cerr << "Bind failed w/ code: " << code;
		freeaddrinfo(results);
		WSACleanup();
		ExitProcess(EXIT_FAILURE);
	}
	
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "Listen failed";
		closesocket(listenSocket);
		WSACleanup();
		ExitProcess(EXIT_FAILURE);
	}

	SOCKET clientSocket = INVALID_SOCKET;


	char buffer[4096];
	ZeroMemory(&buffer, sizeof(buffer));
	// Accept a client socket
	while (true)
	{
		clientSocket = accept(listenSocket, NULL, NULL);
		if (clientSocket == INVALID_SOCKET) {
			std::cerr << "accept failed: " << WSAGetLastError();
			closesocket(listenSocket);
			WSACleanup();
			ExitProcess(EXIT_FAILURE);
		}

		int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);

		std::cout << buffer;

	}
	

	freeaddrinfo(results);




	return 0;
	//return listenSocket;	
}