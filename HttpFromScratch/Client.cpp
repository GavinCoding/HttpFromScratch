#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <Windows.h>

#define PORT 80   //communication over port 80 -> web Server port

const char szHost[] = "127.0.0.1"; //will hold IP address


SOCKET connectClient(char hostName[]) {
	//startup stuff
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);   //Version is represented as two bytes. one byte for both major and minor versions. (2.2  is newest version)
	if (WSAStartup(DllVersion, &wsaData) != 0)   //ensures version and ducks are in line 
	{
		std::cerr << "WSAStartup Fail with code: " << WSAStartup(DllVersion, &wsaData);
		ExitProcess(EXIT_FAILURE);
	}


	//Create Socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);   //Family : AF_INET -> lets use the internet      Stream Type : SOCK_STREAM -> let's use tcp       Protocol: 0 -> communicate over raw socket. Using TCP but sock stream handles that
	if (sock < 0)
	{
		std::cerr << "socket init Fail with code: " << sock;
		ExitProcess(EXIT_FAILURE);
	}

	//Get Server info. Using HOSTENT to store/gather host info
	HOSTENT* host = gethostbyname(hostName);
	if (host == nullptr)
	{
		std::cerr << "get host failed";
		ExitProcess(EXIT_FAILURE);
	}

	//define server info
	SOCKADDR_IN sin;
	ZeroMemory(&sin, sizeof(sin));

	sin.sin_port = htons(PORT);//convert into big endian(network byte order)
	sin.sin_family = AF_INET;
	memcpy(&sin.sin_addr.S_un.S_addr, host->h_addr_list[0], sizeof(sin.sin_addr.S_un.S_addr)); //1st para will hold the ip address


	//connect to server
	if (connect(sock, (const sockaddr*)(&sin), sizeof(sin)) != 0)
	{
		std::cerr << "connection Fail with code: " << connect(sock, (const sockaddr*)(&sin), sizeof(sin));
		ExitProcess(EXIT_FAILURE);
	}

	
	return sock;
}

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
