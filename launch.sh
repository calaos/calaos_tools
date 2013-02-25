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

while true; do

        ulimit -c 0

        #send all backtraces to calaos_network
        crash_report.sh &

        if [ "$2" = "-log" ] ; then
                echo "Log output to /tmp/$1.log"
                $1 >> /tmp/$1.log 2>> /tmp/$1.log &
        else
                $1 > /dev/null 2> /dev/null &
        fi
        pid="$!"
        wait $pid
done
