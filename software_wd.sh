#!/bin/sh

##############################################################################
##  Copyright (c) 2007-2008, Calaos. All Rights Reserved.
##
##  This file is part of Calaos Home.
##
##  Calaos Home is free software; you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation; either version 3 of the License, or
##  (at your option) any later version.
##
##  Calaos Home is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with Foobar; if not, write to the Free Software
##  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
##
##############################################################################

#
# A simple software watchdog
#

if [ $# -lt 1 ] ; then

        echo "Software Watchdog - Copyright (c) Calaos"
        echo "Usage: $0 [app 1] ... [app n]"
        exit 1
fi

CMD_LIST=$@

#sleep 60s first a start, to let app to load correctly
sleep 60

while true ; do
        for cmd in $CMD_LIST ; do

                WD_FILE="/tmp/wd_$cmd"

                if [ -e $WD_FILE ] ; then

                        #wd file present, program is still running
                        rm -f $WD_FILE
                else

                        #wd file not present, program is somehow blocking
                        logger -t watchdog -- "Kill process: $cmd"
                        killall -9 $cmd > /dev/null 2> /dev/null
                fi

        done

        #wait some time before the next check
        sleep 10
done
