/******************************************************************************
 **  Copyright (c) 2007-2011, Calaos. All Rights Reserved.
 **
 **  This file is part of Calaos Home.
 **
 **  Calaos Home is free software; you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation; either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Calaos Home is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Foobar; if not, write to the Free Software
 **  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 **
 ******************************************************************************/

#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Con.h>
#include "modbus.h"
#include <errno.h>
#include <signal.h>
#include <unistd.h>

using namespace std;

#define NB_CONNECTION           5
#define WAGO_UDP_PORT           4646

static void calaos_udp_server_start();
static void libmodbus_start();
static void _sigint(int dummy);
static Eina_Bool _modbus_connection_handler(void *data, Ecore_Fd_Handler *handler);
static Eina_Bool _modbus_client_handler(void *data, Ecore_Fd_Handler *handler);

static Eina_Bool _calaos_udp_data_get(void *data, int type, Ecore_Con_Event_Client_Data *ev);

static bool verbose_mode = false;
static modbus_t *ctx = NULL;
static int server_socket = 0;
static modbus_mapping_t *mb_mapping = NULL;

static Ecore_Fd_Handler *fd_handler = NULL;

static Ecore_Con_Server *udp_server = NULL;

//Utility string function
bool startsWith(std::string& src, std::string token)
{
        return src.substr(0,token.length()) == token;
}

int main(int argc, char **argv)
{
        cout << "*** [ Calaos Wago Simulator ] ***" << endl << endl;

        ecore_init();
        ecore_con_init();

        for (int i = 0;i < argc;i++)
        {
                if (string(argv[i]) == "-v" ||
                    string(argv[i]) == "--verbose")
                {
                        verbose_mode = true;
                }
        }

        cout << "Sarting modbus server." << endl;
        libmodbus_start();

        cout << "Sarting UDP server." << endl;
        calaos_udp_server_start();

        signal(SIGINT, _sigint);

        //Start main loop
        cout << "Waiting for incomming connection..." << endl;
        ecore_main_loop_begin();

        ::close(server_socket);
        ecore_con_server_del(udp_server);

        modbus_free(ctx);
        modbus_mapping_free(mb_mapping);
        ecore_con_shutdown();
        ecore_shutdown();

        return 0;
}

void _sigint(int dummy)
{
        cout << "Exit called." << endl;
        ecore_main_loop_quit();
}


void libmodbus_start()
{
        cout << "Initializing modbus." << endl;

        ctx = modbus_new_tcp("0.0.0.0", MODBUS_TCP_DEFAULT_PORT);

        mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, MODBUS_MAX_READ_BITS,
                                        MODBUS_MAX_WRITE_REGISTERS, MODBUS_MAX_WRITE_REGISTERS);
        if (mb_mapping == NULL)
        {
                fprintf(stderr, "Failed to allocate the mapping: %s\n",
                        modbus_strerror(errno));

                modbus_free(ctx);
        }

        memset(mb_mapping->tab_bits, 0, sizeof(uint8_t) * MODBUS_MAX_READ_BITS);
        memset(mb_mapping->tab_input_bits, 0, sizeof(uint8_t) * MODBUS_MAX_READ_BITS);
        memset(mb_mapping->tab_registers, 0, sizeof(uint16_t) * MODBUS_MAX_WRITE_REGISTERS);
        memset(mb_mapping->tab_input_registers, 0, sizeof(uint16_t) * MODBUS_MAX_WRITE_REGISTERS);

        if (verbose_mode)
                modbus_set_debug(ctx, TRUE);

        modbus_set_slave(ctx, 0x01);

        server_socket = modbus_tcp_listen(ctx, NB_CONNECTION);

        ecore_main_fd_handler_add(server_socket, ECORE_FD_READ, _modbus_connection_handler, mb_mapping, NULL, NULL);
}

Eina_Bool _modbus_connection_handler(void *data, Ecore_Fd_Handler *handler)
{
        if (mb_mapping != data)
        {
                printf("_modbus_connection_handler: Not for me !\n");
                return EINA_TRUE;
        }

        socklen_t addrlen;
        struct sockaddr_in clientaddr;
        int newfd;

        /* Handle new connections */
        addrlen = sizeof(clientaddr);
        memset(&clientaddr, 0, sizeof(clientaddr));
        newfd = accept(server_socket, (struct sockaddr *)&clientaddr, &addrlen);

        if (newfd == -1)
        {
                perror("Server accept() error");
        }
        else
        {
                ecore_main_fd_handler_add(newfd, ECORE_FD_READ, _modbus_client_handler, mb_mapping, NULL, NULL);

                printf("New connection from %s:%d on socket %d\n",
                       inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, newfd);
        }

        return EINA_TRUE;
}

Eina_Bool _modbus_client_handler(void *data, Ecore_Fd_Handler *handler)
{
        if (mb_mapping != data)
        {
                printf("_modbus_client_handler: Not for me !\n");
                return EINA_TRUE;
        }

        /* An already connected master has sent a new query */
        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];

        int fd = ecore_main_fd_handler_fd_get(handler);

        int rc = modbus_receive(ctx, fd, query);

        if (rc != -1)
        {
                printf("Replying command...\n");
                modbus_reply(ctx, query, rc, mb_mapping);
        }
        else
        {
                /* Connection closed by the client, end of server */
                printf("Connection closed on socket %d\n", fd);
                ::close(fd);

                ecore_main_fd_handler_del(handler);
        }

        for (int i = 0;i < 8;i++)
        {
                if (mb_mapping->tab_input_registers[i] == 200)
                        mb_mapping->tab_input_registers[i] = 198;
                else
                        mb_mapping->tab_input_registers[i] = 200;

                mb_mapping->tab_registers[i] = mb_mapping->tab_input_registers[i];
        }

        return EINA_TRUE;
}

void calaos_udp_server_start()
{
        udp_server = ecore_con_server_add(ECORE_CON_REMOTE_UDP, "0.0.0.0", WAGO_UDP_PORT, NULL);
        ecore_con_server_data_set(udp_server, udp_server);

        ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, (Ecore_Event_Handler_Cb)_calaos_udp_data_get, NULL);
}

Eina_Bool _calaos_udp_data_get(void *data, int type, Ecore_Con_Event_Client_Data *ev)
{
        if (ev && (udp_server != ecore_con_server_data_get(ecore_con_client_server_get(ev->client))))
        {
                printf("_calaos_udp_data_get: Not for me !\n");
                return ECORE_CALLBACK_PASS_ON;
        }

        string d((char *)ev->data, ev->size);
        string res;

        if (verbose_mode)
                cout << "CALAOS received: " << d << endl;

        if (startsWith(d, "WAGO_GET_OUTTYPE" ))
                res = d + " 0";
        else if (startsWith(d, "WAGO_GET_OUTTADDR "))
                res = d + " 0 0 -1";
        else if (startsWith(d, "WAGO_GET_INFO_MODULE "))
                res = d + " 0 0 0 0";
        else if (startsWith(d, "WAGO_GET_INFO"))
                res = d + " 0 0 0 0 0 0 0";
        else if (startsWith(d, "WAGO_DALI_GET "))
                res = "WAGO_DALI_GET 0 0 0";
        else if (startsWith(d, "WAGO_DALI_GET_ADDR"))
        {
                res = d + " ";
                for (int i = 0;i < 64;i++)
                        res += "0";
        }
        else if (startsWith(d, "WAGO_DALI_GET_DEVICE_INFO "))
                res = d + " 0 0 0 0 0 0";
        else if (startsWith(d, "WAGO_DALI_GET_DEVICE_GROUP "))
        {
                res = d + " ";
                for (int i = 0;i < 16;i++)
                        res += "0";
        }
        else if (startsWith(d, "WAGO_DALI_DEVICE_ADD_GROUP "))
                res = "WAGO_DALI_DEVICE_ADD_GROUP 1";
        else if (startsWith(d, "WAGO_DALI_DEVICE_DEL_GROUP "))
                res = "WAGO_DALI_DEVICE_DEL_GROUP 1";
        else if (startsWith(d, "WAGO_DALI_ADDRESSING_STATUS "))
                res = "WAGO_DALI_ADDRESSING_STATUS 1";
        else if (startsWith(d, "WAGO_GET_VERSION"))
                res = d + " 2.0 750-849";
        else if (startsWith(d, "WAGO_INFO_VOLET_GET "))
                res = d + " 0";

        if (!res.empty())
        {
                if (verbose_mode)
                        cout << "CALAOS sending: " << res << endl;
                ecore_con_client_send(ev->client, res.c_str(), res.length() + 1);
                ecore_con_client_flush(ev->client);
        }

        return ECORE_CALLBACK_RENEW;
}
