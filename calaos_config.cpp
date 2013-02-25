/******************************************************************************
**  Copyright (c) 2007-2008, Calaos. All Rights Reserved.
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

//
// Write/Read config options from local_config.xml
//

#include <Utils.h>
#include <Ecore_File.h>

void print_usage(void)
{
        cout << "Calaos Configuration Utility." << endl;
        cout << "(c)2008 Calaos" << endl << endl;
        cout << "Usage:\tcalaos_config <action> [params]" << endl << endl;
        cout << "Where action can be:" << endl;
        cout << "\tlist\t\tLists all keys:values" << endl;
        cout << "\tset [key]\tSet the value for the specified key" << endl;
        cout << "\tget [key]\tPrint the value for the specified key" << endl;
        cout << "\tdel [key]\tDelete entry for the specified key" << endl;
}

int main (int argc, char **argv)
{
        if (!ecore_file_exists(DEFAULT_CONFIG))
        {
                Utils::logger("root") << Priority::ERROR << "Error: Default config not found (" << DEFAULT_CONFIG << ")" << log4cpp::eol;
                exit(1);
        }

        if (argc < 2)
        {
                print_usage();
                return 1;
        }
        string action = argv[1];

        Utils::InitLoggingSystem(string(DEFAULT_CONFIG_PATH) + "calaos_console_log.conf");

        if (action == "get")
        {
		string key = argv[2];

		if (key == "hwid")
			cout << Utils::getHardwareID();
		else
	                cout << Utils::get_config_option(key);
        }
        else if (action == "set")
        {
                if (argc < 4)
                {
                        print_usage();
                        return 1;
                }
                string key = argv[2];
                string value = argv[3];

		if (key == "hwid")
			Utils::logger("root") << Priority::ERROR << "Can't change hwid" << log4cpp::eol;
		else
	                Utils::set_config_option(key, value);
        }
        else if (action == "list")
        {
                Params options;
                Utils::get_config_options(options);
                cout << "Local configuration:" << endl;
                for (int i = 0;i < options.size();i++)
                {
                        std::string key, value;
                        options.get_item(i, key, value);
                        cout << key << ": " << value << endl;
                }
        }
        else if (action == "del")
        {
                Utils::del_config_option(argv[2]);
        }
        else
        {
                print_usage();
                return 1;
        }

        return 0;
}
