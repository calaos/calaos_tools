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
// Check firmware version and update it if needed.
//

#include <Utils.h>
#include <EcoreTimer.h>
#include <Firmwares.h>

static Firmwares firmwares;
static EcoreTimer *timer = NULL;

static void checkUpdate();
static void checkUpdate_cb(string msg);

static void downloadUpdate();
static void downloadUpdate_cb(string msg, FileProgress *progress);

static void installUpdate();
static void installUpdate_cb(string msg);

static void checkUpdate()
{
        if (timer)
        {
                delete timer;
                timer = NULL;
        }

        firmwares.checkUpdate(sigc::ptr_fun(checkUpdate_cb));
}

static void checkUpdate_cb(string msg)
{
        if (msg == "failed")
        {
                Utils::logger("root") << Priority::ERROR
                                << "Failed to check update"
                                << log4cpp::eol;

                ecore_main_loop_quit();
        }
        else if (msg == "no_update")
        {
                Utils::logger("root") << Priority::INFO
                                << "System is up to date"
                                << log4cpp::eol;

                ecore_main_loop_quit();
        }
        else
        {
                //update needed
                Utils::logger("root") << Priority::INFO
                                << "Update available: " << msg
                                << log4cpp::eol;

                downloadUpdate();
        }
}

static double counter;
static void downloadUpdate()
{
        counter = ecore_time_get();
        firmwares.downloadFirmware(sigc::ptr_fun(downloadUpdate_cb));
}

static int old_pourcent = -1;
static void downloadUpdate_cb(string msg, FileProgress *progress)
{
        if (msg == "failed")
        {
                Utils::logger("root") << Priority::ERROR
                                << "Failed to download update"
                                << log4cpp::eol;

                ecore_main_loop_quit();
        }
        else if (msg == "progress,update" && progress)
        {
                if (progress->dltotal == 0.0) progress->dltotal = 1.0;
                int pourcent = (int)((progress->dlnow / progress->dltotal) * 100.0);
                int elapsed = (int)(ecore_time_get() - counter);
                static int old_elapsed = 0;
                static string tps = "waiting...";

                if (elapsed != old_elapsed)
                {
                        double length = ((double)elapsed * 100) / (double)pourcent;
                        length -= elapsed;
                        if (progress->dlnow == 0)
                                tps = "Connexion en cours...";
                        else
                                tps = "Temps restant: " + Utils::time2string((long)length);

                        old_elapsed = elapsed;
                }

                stringstream size;
                progress->dlnow = progress->dlnow / 1000000;
                progress->dltotal = progress->dltotal / 1000000;
                if (progress->dltotal < 10) progress->dltotal = 0;
                size << setprecision(2) << progress->dlnow << " / " << progress->dltotal << " Mo";

                if (pourcent != old_pourcent)
                {
                        Utils::logger("root") << Priority::INFO
                                        << "Downloading: " << size.str() << " (" << pourcent << "%) - " << tps
                                        << log4cpp::eol;
                        old_pourcent = pourcent;
                }
        }
        else if (msg == "done")
        {
                Utils::logger("root") << Priority::INFO
                                << "Download done."
                                << log4cpp::eol;

                installUpdate();
        }
}

static void installUpdate()
{
        firmwares.installFirmware(sigc::ptr_fun(installUpdate_cb));
}

static void installUpdate_cb(string msg)
{
        if (msg == "done")
        {
                Utils::logger("root") << Priority::INFO
                                << "Install done."
                                << log4cpp::eol;
        }
        else
        {
                Utils::logger("root") << Priority::INFO
                                << "Install Failed !"
                                << log4cpp::eol;
        }

        ecore_main_loop_quit();
}

int main (int argc, char **argv)
{
        ecore_init();
        ecore_con_init();

        Utils::InitLoggingSystem(string(DEFAULT_CONFIG_PATH) + "calaos_console_log.conf");

        timer = new EcoreTimer(0.0, (sigc::slot<void>)sigc::ptr_fun(checkUpdate));

        ecore_main_loop_begin();

        ecore_con_shutdown();
        ecore_shutdown();

        return 0;
}
