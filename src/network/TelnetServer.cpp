#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif
#ifdef __linux__
#include <algorithm>
#endif
#include <libtelnet.h>
#include <assert.h>
#include "Logger.h"
#include "RemoteMonitor.h"

using namespace RetroSim::Logger;
using namespace RetroSim::RemoteMonitor;

namespace RetroSim::TelnetServer
{
    static const telnet_telopt_t telopts[] = {
        {TELNET_TELOPT_ECHO, TELNET_WILL, TELNET_DO},
        {TELNET_TELOPT_TTYPE, TELNET_WONT, TELNET_DO},
        {-1, 0, 0}};

    std::string RemoveTrailingNewline(char *str)
    {
        std::string s(str);
        s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
        s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
        return s;
    }

    void TelnetSend(telnet_t *telnet, const std::string &message)
    {
#ifdef WIN32
        std::string processedMessage;
        processedMessage.reserve(message.length() * 2);

        for(char c : message)
        {
            if (c == '\n')
                processedMessage += "\r\n";
            else
                processedMessage += c;
        }
        
        telnet_send(telnet, processedMessage.c_str(), processedMessage.length());
#else
        telnet_send(telnet, message.c_str(), message.length());
#endif
    }

    static void EventHandler(telnet_t *telnet, telnet_event_t *ev, void *user_data)
    {
#ifdef WIN32
        SOCKET *sock = (SOCKET *)user_data;
#else
        int *sock = (int *)user_data;
#endif

        switch (ev->type)
        {
        case TELNET_EV_DATA:
        {
            // allocate buffer for received data
            char *buffer = (char *)malloc(ev->data.size + 1);
            // copy received data into buffer
            memcpy(buffer, ev->data.buffer, ev->data.size);
            LogPrintf(RETRO_LOG_INFO, "Telnet input: %s", buffer);

            // process command
            std::string command = RemoveTrailingNewline(buffer);
            std::string response = RetroSim::RemoteMonitor::ProcessCommand(command);

            // send response
            response += "\n>";
            TelnetSend(telnet, response);

            free(buffer);
        }
        break;
        case TELNET_EV_SEND:
            send(*sock, ev->data.buffer, (int)ev->data.size, 0);
            break;
        case TELNET_EV_ERROR:
#ifdef WIN32
            closesocket(*sock);
#else
            close(*sock);
#endif
            telnet_free(telnet);
            LogPrintf(RETRO_LOG_INFO, "Client error, connection closed.\n");
            break;
        default:
            break;
        }
    }

    int listen_sock;

#ifdef WIN32
    int StartSocketWindows()
    {
        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0)
        {
            printf("WSAStartup failed: %d\n", iResult);
            return -1; // return an error code
        }

        std::string address = "127.0.0.1";
        int port = 8080;
        
        SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return -1;
        }

        sockaddr_in service;
        service.sin_family = AF_INET;
        // using inet_pton instead of inet_addr
        if (inet_pton(AF_INET, address.c_str(), &service.sin_addr) <= 0)
        {
            printf("inet_pton failed with error\n");
            closesocket(listenSocket);
            WSACleanup();
            return -1;
        }
        service.sin_port = htons(port);

        if (bind(listenSocket, (SOCKADDR *)&service, sizeof(service)) == SOCKET_ERROR)
        {
            printf("bind failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return -1;
        }

        if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
        {
            printf("listen failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return -1;
        }

        while (true)
        {
            SOCKET clientSocket = accept(listenSocket, NULL, NULL);
            if (clientSocket == INVALID_SOCKET)
            {
                printf("accept failed with error: %d\n", WSAGetLastError());
                closesocket(listenSocket);
                WSACleanup();
                return -1;
            }

            telnet_t *telnet = telnet_init(telopts, EventHandler, 0, &clientSocket);
            if (telnet == NULL)
            {
                printf("telnet_init failed\n");
                closesocket(clientSocket);
                continue;
            }

            std::string welcomeMessage = "Welcome to RetroSim!\n";
            TelnetSend(telnet, welcomeMessage);
            std::string response = RemoteMonitor::DisplayHelp() + "\n>";
            TelnetSend(telnet, response);

            std::string input_buffer;
            char buffer[512];
            int bytes;

            while ((bytes = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
            {
                for (int i = 0; i < bytes; ++i)
                {
                    char c = buffer[i];
                    if (c == '\r' || c == '\n')
                    {
                        // Process the complete line
                        telnet_recv(telnet, input_buffer.c_str(), input_buffer.length());
                        input_buffer.clear();
                    }
                    else
                    {
                        input_buffer += c;
                    }
                }
            }

            telnet_free(telnet);
            closesocket(clientSocket);
        }

        return 0; // return value to signify success
    }
#else
    int StartSocketUnix()
    {
        listen_sock = socket(AF_INET, SOCK_STREAM, 0);

        int yes = 1;
        if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        {
            LogPrintf(RETRO_LOG_ERROR, "Could not set socker reuse option.");
            return 1;
        }

        struct sockaddr_in addr;
        int rs;

        if (listen_sock == -1)
        {
            LogPrintf(RETRO_LOG_ERROR, "Failed to create listening socket.");
            return 1;
        }

        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(23);

        rs = bind(listen_sock, (struct sockaddr *)&addr, sizeof(addr));
        if (rs == -1)
        {
            LogPrintf(RETRO_LOG_ERROR, "bind() failed.");
            return 1;
        }

        rs = listen(listen_sock, 5);
        if (rs == -1)
        {
            LogPrintf(RETRO_LOG_ERROR, "listen() failed.");
            return 1;
        }

        LogPrintf(RETRO_LOG_INFO, "Telnet server started on port 23.\n");

        while (1)
        {
            int client_sock = accept(listen_sock, 0, 0);
            if (client_sock == -1)
            {
                LogPrintf(RETRO_LOG_ERROR, "accept() failed.");
                return 1;
            }

            telnet_t *telnet = telnet_init(telopts, EventHandler, 0, &client_sock);

            LogPrintf(RETRO_LOG_INFO, "Client connected.\n");
            telnet_send(telnet, "Welcome to RetroSim!\n\n", 23);
            std::string response = RemoteMonitor::DisplayHelp() + "\n>";
            telnet_send(telnet, response.c_str(), response.length());

            char buffer[512];
            int bytes;

            while ((bytes = recv(client_sock, buffer, sizeof(buffer), 0)) > 0)
            {
                telnet_recv(telnet, buffer, bytes);
            }

            telnet_free(telnet);
            close(client_sock);
        }

        close(listen_sock);

        return 0;
    }
#endif

    int Start()
    {
#ifdef WIN32
        return StartSocketWindows();
#else
        return StartSocketUnix();
#endif
    }

    void Stop()
    {
        LogPrintf(RETRO_LOG_INFO, "Stopping telnet server.\n");
#ifdef WIN32
        closesocket(listen_sock);
#else
        close(listen_sock);
#endif
    }
} // namespace RetroSim::TelnetServer
