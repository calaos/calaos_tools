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

#Generate the /etc/network/interfaces file from local_config.xml at boot time

IFILE=/etc/network/interfaces
RESOLV=/etc/resolv.conf

#write the localhost config
echo -e "auto lo\niface lo inet loopback\n" > $IFILE

#eth0 config
DHCP=`calaos_config get eth0_dhcp`

if [ "$DHCP" == "true" ] ; then
        echo -e "auto eth0\niface eth0 inet dhcp\n" >> $IFILE
else
        ADDR=`calaos_config get eth0_address`
        MASK=`calaos_config get eth0_netmask`
        BCAST=`calaos_config get eth0_broadcast`
        GW=`calaos_config get eth0_gateway`

        echo -e "auto eth0\niface eth0 inet static" >> $IFILE
        echo -e "\taddress $ADDR\n\tnetmask $MASK" >> $IFILE
        echo -e "\tbroadcast $BCAST" >> $IFILE

        if [ "$GW" != "" ]; then
                echo -e "\tgateway $GW\n" >> $IFILE
        else
                echo "" >> $IFILE
        fi

        #DNS
        DNS=`calaos_config get dns_address`

        echo -e "search lan\nnameserver $DNS\n" > $RESOLV
fi

#eth1 config
ADDR=`calaos_config get eth1_address`
MASK=`calaos_config get eth1_netmask`
BCAST=`calaos_config get eth1_broadcast`
GW=`calaos_config get eth1_gateway`

echo -e "auto eth1\niface eth1 inet static" >> $IFILE
echo -e "\taddress $ADDR\n\tnetmask $MASK" >> $IFILE
echo -e "\tbroadcast $BCAST" >> $IFILE

if [ "$GW" != "" ]; then
        echo -e "\tgateway $GW\n" >> $IFILE
fi
