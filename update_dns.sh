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

## Update the public ip address to the Calaos Database.

CALAOS_URL="https://www.calaos.fr/calaos_network/api.php"

HWID=`/sbin/calaos_config get hwid`
CNUSER=`/sbin/calaos_config get cn_user`
CNPASS=`/sbin/calaos_config get cn_pass`
FWVERSION=`/sbin/calaos_config get fw_version`
ETH0_IP=`ifconfig eth0 | grep 'inet ' | cut -d: -f2 | awk '{ print $1 }'`

if [ "$HWID" = "" ] ; then
        exit 1;
fi

if [ "$CNUSER" = "" ] ; then
	exit 1;
fi

if [ "$CNPASS" = "" ] ; then
	exit 1;
fi

calaos_network update_ip $ETH0_IP

