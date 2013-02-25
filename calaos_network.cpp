/******************************************************************************
**  Copyright (c) 2006-2011, Calaos. All Rights Reserved.
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
**  along with Calaos; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
**
******************************************************************************/
#include <Utils.h>
#include <EcoreTimer.h>
#include <CalaosNetwork.h>

static CalaosNetwork calaos_network;
static EcoreTimer *timer = NULL;
static int argc = 0;
static char **argv = NULL;

static void printUsage()
{
        cout << "Calaos Network command line tool." << endl;
        cout << "(c) 2011 Calaos" << endl << endl;
        cout << "Usage: " << argv[0] << " <command>" << endl << endl;
        cout << "Where command is one of:" << endl;
        cout << "\tregister [username] [password]\tRegister a machine with a calaos network account" << endl;
        cout << "\tupdate_ip <private_ip>\tUpdate the private IP" << endl;
        cout << "\tget_ip\t\tGet public and private IP" << endl;

        ecore_main_loop_quit();
}

static void parseCommandLine()
{
        delete timer;

        if (argc <= 1)
        {
                printUsage();
                return;
        }

        string command = argv[1];
        if (command == "register")
        {
                if (argc >= 4)
                        calaos_network.Register(argv[2], argv[3]);
                else
                        calaos_network.Register(Utils::get_config_option("cn_user"), Utils::get_config_option("cn_pass"));
        }
        else if (command == "update_ip")
        {
                if (argc < 3)
                        printUsage();
                else
                        calaos_network.updateIP(argv[2]);
        }
        else if (command == "get_ip")
        {
                calaos_network.getIP();
        }
        else
        {
                printUsage();
        }
}

static void register_cb(string result)
{
        if (result == "done")
                cout << "Register done." << endl;
        else
                cout << "Register failed !" << endl;

        ecore_main_loop_quit();
}

static void update_ip_cb(string result)
{
        if (result == "done")
                cout << "Update IP done." << endl;
        else
                cout << "Update IP failed !" << endl;

        ecore_main_loop_quit();
}

static void get_ip_cb(string result, string public_ip, string private_ip, bool at_home)
{
        if (result == "done")
                cout << "Public IP: " << public_ip << endl << "Private IP: " << private_ip << endl << "At Home: " << boolalpha << at_home << endl;
        else
                cout << "Get IP failed !" << endl;

        ecore_main_loop_quit();
}

int main (int _argc, char **_argv)
{
        argc = _argc;
        argv = _argv;
        ecore_init();
        ecore_con_init();

        calaos_network.registered.connect(sigc::ptr_fun(register_cb));
        calaos_network.ip_updated.connect(sigc::ptr_fun(update_ip_cb));
        calaos_network.ip_retrieved.connect(sigc::ptr_fun(get_ip_cb));

        Utils::InitLoggingSystem(string(DEFAULT_CONFIG_PATH) + "calaos_console_log.conf");

        timer = new EcoreTimer(0.0, (sigc::slot<void>)sigc::ptr_fun(parseCommandLine));

        ecore_main_loop_begin();

        ecore_con_shutdown();
        ecore_shutdown();

        return 0;
}
