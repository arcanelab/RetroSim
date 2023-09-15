#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
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

    static void EventHandler(telnet_t *telnet, telnet_event_t *ev, void *user_data)
    {
        int *sock = (int *)user_data;

        switch (ev->type)
        {
        case TELNET_EV_DATA:
        {
            // allocate buffer for received data
            char *buffer = (char *)malloc(ev->data.size + 1);
            // copy received data into buffer
            memcpy(buffer, ev->data.buffer, ev->data.size);
            LogPrintf(RETRO_LOG_INFO, "Received data: %s\n", buffer);

            // process command
            std::string command = RemoveTrailingNewline(buffer);
            std::string response = RetroSim::RemoteMonitor::ProcessCommand(command);

            // send response
            telnet_send(telnet, response.c_str(), response.length());
            telnet_send(telnet, "\n", 1);
            
            free(buffer);
        }
        break;
        case TELNET_EV_SEND:
            send(*sock, ev->data.buffer, ev->data.size, 0);
            break;
        case TELNET_EV_ERROR:
            close(*sock);
            telnet_free(telnet);
            LogPrintf(RETRO_LOG_INFO, "Client error, connection closed.\n");
            break;
        default:
            break;
        }
    }

    int listen_sock;

    int Start()
    {
        listen_sock = socket(AF_INET, SOCK_STREAM, 0);
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

    void Stop()
    {
        LogPrintf(RETRO_LOG_INFO, "Stopping telnet server.\n");
        close(listen_sock);
    }
} // namespace RetroSim::TelnetServer
