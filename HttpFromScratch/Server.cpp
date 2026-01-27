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

    HttpRequest(const std::string& msg) {
        if (msg.empty()) {
            requestValidity = false;
            return;
        }

        // ---- Request line ----
        size_t eorl = msg.find("\r\n");
        if (eorl == std::string::npos) {
            // Incomplete request line
            expected = false;
            return;
        }

        std::string requestLine = msg.substr(0, eorl);

        // ---- Method detection ----
        if (requestLine.rfind("GET ", 0) == 0) {
            method = "GET";
        }
        else {
            expected = false;

            if (requestLine.rfind("POST ", 0) == 0)    method = "POST";
            else if (requestLine.rfind("PUT ", 0) == 0)     method = "PUT";
            else if (requestLine.rfind("PATCH ", 0) == 0)   method = "PATCH";
            else if (requestLine.rfind("DELETE ", 0) == 0)  method = "DELETE";
            else if (requestLine.rfind("HEAD ", 0) == 0)    method = "HEAD";
            else if (requestLine.rfind("OPTIONS ", 0) == 0) method = "OPTIONS";
            else if (requestLine.rfind("CONNECT ", 0) == 0) method = "CONNECT";
            else if (requestLine.rfind("TRACE ", 0) == 0)   method = "TRACE";
            else {
                requestValidity = false;
                return;
            }
        }

        // ---- Parse path ----
        size_t firstSpace = requestLine.find(' ');
        if (firstSpace == std::string::npos) {
            requestValidity = false;
            return;
        }

        size_t secondSpace = requestLine.find(' ', firstSpace + 1);
        if (secondSpace == std::string::npos || secondSpace <= firstSpace) {
            requestValidity = false;
            return;
        }

        path = requestLine.substr(
            firstSpace + 1,
            secondSpace - firstSpace - 1
        );

        if (path == "/") path = "index.html";
        else if (!path.empty() && path[0] == '/') path.erase(0, 1);

        // ---- HTTP version ----
        size_t httpPos = requestLine.find("HTTP/");
        if (httpPos == std::string::npos || httpPos + 8 > requestLine.size()) {
            requestValidity = false;
            return;
        }

        version = requestLine.substr(httpPos, 8);

        // ---- Headers ----
        size_t headerEnd = msg.find("\r\n\r\n");
        if (headerEnd == std::string::npos) {
            // Headers not complete yet (not invalid)
            expected = false;
            return;
        }

        size_t headerStart = eorl + 2;
        if (headerStart > headerEnd) {
            requestValidity = false;
            return;
        }

        headers = msg.substr(
            headerStart,
            headerEnd - headerStart
        );

        // ---- Body ----
        if (headerEnd + 4 <= msg.size()) {
            body = msg.substr(headerEnd + 4);
        }
        else {
            body.clear();
        }
    }

    bool isEmpty(const std::string& msg) const {
        return msg.empty();
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
	
	