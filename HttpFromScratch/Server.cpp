#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>
#include <Windows.h>

#include <iostream>
#include <string>

#define PORT "80"   //communication over port 80 -> web Server port
const char severAddr[] = "127.0.0.1";


class HttpRequest
{
	public:
		bool requestValidity = TRUE;
		bool expected = TRUE;
		std::string method;
		std::string path;
		std::string version;
		std::string headers;
		std::string body;


		//Constructor
		HttpRequest(std::string msg)
		{
			//std::cout << msg << "\nExtractedBody:";
			//check first part of MSG. Other parts may contain request type keywords
			std::string requestWindow = msg.substr(0, 8);
			if (!requestWindow.contains("GET"))
			{
				this->expected = FALSE;   //We are onyl eexpecting GET results
				if (requestWindow.contains("POST"))
					setMethod("POST");
				else if (requestWindow.contains("PUT"))
					setMethod("PUT");
				else if (requestWindow.contains("PATCH"))
					setMethod("PATCH");
				else if (requestWindow.contains("DELETE"))
					setMethod("DELETE");
				else if (requestWindow.contains("HEAD"))
					setMethod("HEAD");
				else if (requestWindow.contains("OPTIONS"))
					setMethod("OPTIONS");
				else if (requestWindow.contains("CONNECT"))
					setMethod("CONNECT");
				else if (requestWindow.contains("TRACE"))
					setMethod("TRACE");
				else									
					this->requestValidity = FALSE;					//NOT A VALID HTTP MSG

			}
			else
			{
				setMethod("GET");
			}
			//next Get Path(Request-URI)
			setPath( msg.substr( method.length(), msg.find(" HTTP") - method.length() ) );

			setVersion(msg.substr(msg.find("HTTP"), 8));

			//Use \n (end of request line) as start delimiter and \n\r (end of Request Header) to extract Request Header
			setHeader( msg.substr(msg.find("\n"), (msg.find("\r\n") - msg.find("\n")) ));

			//std::cout << msg.substr( msg.find("\r\n\r\n") + 4, msg.length() - msg.find("\r\n\r\n"));
			setBody( msg.substr(msg.find("\r\n\r\n") + 4, msg.length() - msg.find("\r\n\r\n")) );
		}

		void setMethod(std::string method)
		{
			this->method= method;
			//std::cout << this->method;
		}
		void setPath(std::string path)
		{
			this->path = path;
		}
		void setVersion(std::string version)
		{
			this->version = version;
		}
		void setHeader(std::string headers)
		{
			this->headers = headers;
		}
		void setBody(std::string body)
		{
			this->body = body;
		}
		
		
};

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

	char response[4096];
	ZeroMemory(&response, sizeof(response));

	std::string msg;
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
		
		msg.append(buffer, bytes);

		HttpRequest request(msg);

		ZeroMemory(&msg, sizeof(msg));
		//getchar();

	}
	

	freeaddrinfo(results);




	return 0;
	//return listenSocket;	
}

	//std::string header;
	//	std::string status;
	//		std::string requestType;
	//size_t pos;

	////get header

	//pos = request.find("\r\n\r\n");
	//header = request.substr(0, pos);
	////std::cout << "HEADER-> " << header << " <-HEADER";

	////get status
	//pos = header.find("\r\n");
	//status = header.substr(0, pos);

	////if Header is not GET Request then we HCF'
	//if (status.contains("GET"))
	//{
	//	pos = status.find("GET");
	//	requestType = status.substr(0, pos + 3);
	//
	//}
	//else
	//{
	//	printf("GET Not found.\n");
	//	//Dont throw error return 404. Support for other request types later

	//}
	////Get Request type
	