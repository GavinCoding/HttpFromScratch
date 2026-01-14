#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <fstream>
#include <ctime>

#include <iostream>
#include <string>
#include <sstream>

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
			size_t eorl = msg.find("\r\n");//end of request line
			//check first part of MSG. Other parts may contain request type keywords

			std::string requestWindow = msg.substr(0, eorl);
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


			size_t firstspace = requestWindow.find(" ");
			size_t secondspace = requestWindow.find(" ", firstspace + 1);
			//next Get Path(Request-URI)
			//std::cout << "REquest path is ->" << msg.substr(method.length() + 1, msg.find(" HTTP") - method.length()-1) << "<-";
			setPath( msg.substr(firstspace + 1, secondspace - firstspace -1 ) );

			//Normalize root
			if (path.empty())
				path = "/";

			setVersion(msg.substr(msg.find("HTTP"), 8));

			//Use \n (end of request line) as start delimiter and \n\r (end of Request Header) to extract Request Header
			setHeader( msg.substr(msg.find("\n"), (msg.find("\r\n") - msg.find("\n")) ));

			//std::cout << msg.substr( msg.find("\r\n\r\n") + 4, msg.length() - msg.find("\r\n\r\n"));
			setBody( msg.substr(msg.find("\r\n\r\n") + 4, msg.length() - msg.find("\r\n\r\n")) );
		}

		void setMethod(std::string method)
		{
			this->method= method;
			//std::cout << this->method;7
		}
		void setPath(std::string path)
		{
			path.erase(0, 1);
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
		bool isEmpty(std::string msg)
		{
			return msg.empty();
		}
		
};
class HttpResponse
{
	const std::string version = "HTTP/1.1";
	public:
		std::string status = "";
		std::string reason = "";
		std::string body = "";
		std::string headers = "";

		

		// Status depends on Request info
		
		
		void setStatus(std::string status)
		{
			this->status = status;
			//std::cout << this->method;
		}
		void setReason(std::string reason)
		{
			this->reason = reason;
		}
		void setBody(std::string body)
		{
			this->body = body;
		}
		void setHeaders(std::string headers)
		{
			this->headers = headers;
		}
		std::string toString() const
		{
			return version +" " + status + " "+  reason + "\r\n" + headers + "\r\n" + body + "\r\n\r\n";
		}

};
bool fileExists(const std::string& filename) {
	std::ifstream file(filename);
	return file.is_open();
}

HttpResponse BuildResponse(HttpRequest request)
{
	HttpResponse temp;
	//std::cout << "looking for file " << request.path << std::endl;
	//Unsuccessful requests
	if (request.requestValidity == false)
	{
		std::cout << "Bad Request";
		temp.setStatus("400");   //code 400 -> Bad Request
		temp.setReason("Bad Request");
	}
	else if (request.expected == false)
	{
		std::cout << "Method Not Allowed";
		temp.setStatus("405");	//code 405 -> Method Not allowed. Potentially send them to another page
		temp.setReason("Method Not Allowed");
		//if not expecterd return METHOD NOT ALLOWED
	}
	else if ( (!fileExists(request.path)) &&  request.path !=  "/") 
	{
		//quick test
		
		std::cout << "Can't find that file";
		temp.setStatus("404");
		temp.setReason("Not Found");
		//return 404

	}
	else
	{
		//std::cout << "found file\n";
		//File exists
		// things can go smoothly
		temp.setStatus("200");
		temp.setReason("OK");

		std::ifstream file;
		if (request.path == "/")
		{
			file.open("index.html");
		}
		else
		{
			file.open(request.path);
		}
		
		std::stringstream buffer;

		buffer << file.rdbuf();

		std::string resonseBody;
		resonseBody = buffer.str();

		file.close();


		//std::cout << "\n\n\nResponse Body: \n" << resonseBody << "\n\n\n";
		temp.setBody(resonseBody);

		size_t contentSize = resonseBody.size();
		temp.setHeaders(
			"Content-Type: text/html; charset=UTF-8\r\n"
			"Content-Length: " + std::to_string(contentSize) + "\r\n"
		);
	}
	return temp;



	//Successful requests

	/*	200 OK:
			This is the most prevalent and ideal response for a successful GET request.
			It signifies that the request was processed without issues, and the requested resource (e.g., a webpage, JSON data) is included in the response body.
		204 No Content:
			This code indicates that the request was successful, but there is no content to return in the response body.
			This is useful when a GET request is made to check for the existence of a resource or to trigger a server-side action that doesn't require a data response.
		304 Not Modified:
			This code is a redirection message, but it's often considered a successful outcome for a GET request when caching is involved.
			It tells the client that the requested resource has not been modified since the last request, allowing the client to use its cached version, saving bandwidth and improving performance.*/
			/*

			*
				Handle immediate errors then get into nitty gritty URI availability checking/returning system
			 bool requestValidity = TRUE;
				bool expected = TRUE;
			*/


	return temp;
}

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
			break;
		}

		char buffer[4096];
		std::string msg;

		// Receive full HTTP request
		while (true) {
			int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
			if (bytes <= 0) break;

			msg.append(buffer, bytes);
			if (msg.find("\r\n\r\n") != std::string::npos) break;
		}
		if (msg.empty())
			continue;
		HttpRequest request(msg);
		std::cout << "HTTP REQUEST: \n" << msg;

		HttpResponse response = BuildResponse(request);

		std::string responseStr = response.toString();
		std::cout << "\nHTTP Response: \n" << responseStr;

		send(clientSocket, responseStr.c_str(), responseStr.size(), 0);

	}
	

	freeaddrinfo(results);
	closesocket(listenSocket);
	closesocket(clientSocket);



	return 0;
	//return listenSocket;	
}


/*
	To-do
		more robust error handling with lookup table. String to int. int to String for debug. Debug config only?

*/
	
	