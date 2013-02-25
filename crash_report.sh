#!/bin/sh

##############################################################################
##  Copyright (c) 2006-2011, Calaos. All Rights Reserved.
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

## crash reporter. compress backtrace with system info and send it to calaos.fr
BTPATH="/mnt/ext3/backtraces"
mkdir -p $BTPATH

if [ $# -gt 1 ] ; then
        process="$1"
        dump_id="$2"

        if [ ! -f $BTPATH/$dump_id.dmp ] ; then
                echo "Can't find file $BTPATH/$dump_id.dmp"
                exit 1;
        fi

        #create an archive of system files
        (cd $BTPATH ;
        mkdir -p $dump_id ;
        cd $dump_id ;
        cp ../$dump_id.dmp /etc/calaos/io.xml /etc/calaos/rules.xml /etc/calaos/local_config.xml /etc/calaos/simple_scenarios.xml /var/log/message* /etc/BUILD /etc/VERSION . ;
        tar -cjf ../bt_${process}_${dump_id}.tar.bz2 * )

        #clean everything
        rm -fr $BTPATH/$dump_id $BTPATH/$dump_id.dmp

        exit 0;
fi

#Lock file, avoid multiple instance of the script
test -f /tmp/.send_backtrace.lock && logger -t crash_report -- "script already running? exiting..." && exit 1
touch /tmp/.send_backtrace.lock

CALAOS_URL="https://www.calaos.fr/calaos_network/api.php"
HWID=`/sbin/calaos_config get hwid`
CNUSER=`/sbin/calaos_config get cn_user`
CNPASS=`/sbin/calaos_config get cn_pass`

if [ "$HWID" = "" ] ; then
        exit 1;
fi

if [ "$CNUSER" = "" ] ; then
	exit 1;
fi

if [ "$CNPASS" = "" ] ; then
	exit 1;
fi

for bt in `ls ${BTPATH}/bt_*.tar.bz2` ; do

        btname=`basename $bt`;
        encoded=`uuencode -m $bt $btname`;

        JSON="{\"cn_user\":\"${CNUSER}\",\"cn_pass\":\"${CNPASS}\",\"hwid\":\"${HWID}\",\"action\":\"upload_backtrace\"}"
        curl --form "json=$JSON;type=application/json" --form file=@$bt --insecure --silent --output /dev/null $CALAOS_URL
        if [ $? -eq 1 ]
        then
                logger -t crash_report -- "Error sending crash backtrace"
                exit 1;
        fi

        logger -t crash_report -- "Crash report $bt successfully sent."

        #delete backtrace
        rm -f $bt

done

#remove the lock
rm -f /tmp/.send_backtrace.lock

