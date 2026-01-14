#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#define PORT "80"
const char serverAddr[] = "0.0.0.0";   // listen on all interfaces

class HttpRequest {
public:
    bool requestValidity = true;
    bool expected = true;
    std::string method;
    std::string path;
    std::string version;
    std::string headers;
    std::string body;

    HttpRequest(std::string msg) {
        size_t eorl = msg.find("\r\n");
        std::string requestWindow = msg.substr(0, eorl);

        if (requestWindow.find("GET") == std::string::npos) {
            expected = false;

            if (requestWindow.find("POST") != std::string::npos) method = "POST";
            else if (requestWindow.find("PUT") != std::string::npos) method = "PUT";
            else if (requestWindow.find("PATCH") != std::string::npos) method = "PATCH";
            else if (requestWindow.find("DELETE") != std::string::npos) method = "DELETE";
            else if (requestWindow.find("HEAD") != std::string::npos) method = "HEAD";
            else if (requestWindow.find("OPTIONS") != std::string::npos) method = "OPTIONS";
            else if (requestWindow.find("CONNECT") != std::string::npos) method = "CONNECT";
            else if (requestWindow.find("TRACE") != std::string::npos) method = "TRACE";
            else requestValidity = false;
        }
        else {
            method = "GET";
        }

        size_t firstspace = requestWindow.find(" ");
        size_t secondspace = requestWindow.find(" ", firstspace + 1);

        path = requestWindow.substr(firstspace + 1,
            secondspace - firstspace - 1);
        if (path == "/") path = "index.html";
        else path.erase(0, 1);

        version = msg.substr(msg.find("HTTP"), 8);

        size_t headerEnd = msg.find("\r\n\r\n");
        headers = msg.substr(msg.find("\r\n") + 2,
            headerEnd - msg.find("\r\n") - 2);

        body = msg.substr(headerEnd + 4);
    }
};

class HttpResponse {
    const std::string version = "HTTP/1.1";
public:
    std::string status;
    std::string reason;
    std::string headers;
    std::string body;

    std::string toString() const {
        return version + " " + status + " " + reason + "\r\n" +
            headers + "\r\n" + body;
    }
};

bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.is_open();
}

HttpResponse BuildResponse(const HttpRequest& request) {
    HttpResponse res;

    if (!request.requestValidity) {
        res.status = "400";
        res.reason = "Bad Request";
    }
    else if (!request.expected) {
        res.status = "405";
        res.reason = "Method Not Allowed";
    }
    else if (!fileExists(request.path)) {
        res.status = "404";
        res.reason = "Not Found";
    }
    else {
        std::ifstream file(request.path);
        std::stringstream buffer;
        buffer << file.rdbuf();

        res.body = buffer.str();
        res.status = "200";
        res.reason = "OK";

        res.headers =
            "Content-Type: text/html; charset=UTF-8\r\n"
            "Content-Length: " + std::to_string(res.body.size()) + "\r\n";
    }

    return res;
}

int main() {
    struct addrinfo hints {}, * results;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(serverAddr, PORT, &hints, &results) != 0) {
        perror("getaddrinfo");
        return EXIT_FAILURE;
    }

    int listenSocket = socket(results->ai_family,
        results->ai_socktype,
        results->ai_protocol);

    if (listenSocket < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    int opt = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(listenSocket, results->ai_addr, results->ai_addrlen) < 0) {
        perror("bind");
        return EXIT_FAILURE;
    }

    if (listen(listenSocket, SOMAXCONN) < 0) {
        perror("listen");
        return EXIT_FAILURE;
    }

    std::cout << "Listening on port 80...\n";

    while (true) {
        int clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            perror("accept");
            continue;
        }

        char buffer[4096];
        std::string msg;

        while (true) {
            ssize_t bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytes <= 0) break;
            msg.append(buffer, bytes);
            if (msg.find("\r\n\r\n") != std::string::npos) break;
        }

        if (msg.empty())
            continue;
        HttpRequest request(msg);
        HttpResponse response = BuildResponse(request);

        std::string out = response.toString();
        send(clientSocket, out.c_str(), out.size(), 0);

        close(clientSocket);
    }

    close(listenSocket);
    freeaddrinfo(results);
    return 0;
}



/*
	To-do
		more robust error handling with lookup table. String to int. int to String for debug. Debug config only?

*/
	
	