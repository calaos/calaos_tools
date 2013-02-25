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

#This script update network info from the old interfaces file on the CF
#If the update is allready done, just exit

if [ "`calaos_config get eth1_address`" != "" ]; then
        exit 0
fi
if [ ! -f /mnt/ext3/interfaces ]; then
        exit 0
fi

IFILE=/mnt/ext3/interfaces
RESOLV=/mnt/ext3/resolv.conf

echo "Updating to new network config..."

IFS='
'

read_eth0=false
read_eth1=false

for el in `cat $IFILE` ; do

        if [ "$el" == "iface eth0 inet dhcp" ] ; then
                ETH0_DHCP="true"
        fi
        if [ "$el" == "iface eth0 inet static" ] ; then
                ETH0_DHCP="false"
                read_eth0=true
        fi

        #read eth0
        if $read_eth0 ; then
                if [ `echo $el | awk '{ print $1 }'` == "address" ] ; then
                        ETH0_ADDR=`echo $el | awk '{ print $2 }'`
                fi
                if [ `echo $el | awk '{ print $1 }'` == "netmask" ] ; then
                        ETH0_MASK=`echo $el | awk '{ print $2 }'`
                fi
                if [ `echo $el | awk '{ print $1 }'` == "broadcast" ] ; then
                        ETH0_BCAST=`echo $el | awk '{ print $2 }'`
                fi
                if [ `echo $el | awk '{ print $1 }'` == "gateway" ] ; then
                        ETH0_GW=`echo $el | awk '{ print $2 }'`
                fi
        fi

        if [ "$el" == "iface eth1 inet static" ] ; then
                read_eth0=false
                read_eth1=true
        fi

        #read eth1
        if $read_eth1 ; then
                if [ `echo $el | awk '{ print $1 }'` == "address" ] ; then
                        ETH1_ADDR=`echo $el | awk '{ print $2 }'`
                fi
                if [ `echo $el | awk '{ print $1 }'` == "netmask" ] ; then
                        ETH1_MASK=`echo $el | awk '{ print $2 }'`
                fi
                if [ `echo $el | awk '{ print $1 }'` == "broadcast" ] ; then
                        ETH1_BCAST=`echo $el | awk '{ print $2 }'`
                fi
                if [ `echo $el | awk '{ print $1 }'` == "gateway" ] ; then
                        ETH1_GW=`echo $el | awk '{ print $2 }'`
                fi
        fi

done

if [ -f $RESOLV ]; then
        #read dns
        for el in `cat $RESOLV` ; do

                if [ `echo $el | awk '{ print $1 }'` == "nameserver" ]; then
                        DNS=`echo $el | awk '{ print $2 }'`
                fi
        done
fi

IFS=' '

#update config in local_config.xml

calaos_config set eth0_dhcp $ETH0_DHCP
if [ "$ETH0_DHCP" == "false" ]; then
        calaos_config set eth0_address $ETH0_ADDR
        calaos_config set eth0_netmask $ETH0_MASK
        calaos_config set eth0_broadcast $ETH0_BCAST
        calaos_config set eth0_gateway $ETH0_GW
fi

#eth1
calaos_config set eth1_address $ETH1_ADDR
calaos_config set eth1_netmask $ETH1_MASK
calaos_config set eth1_broadcast $ETH1_BCAST
calaos_config set eth1_gateway $ETH1_GW

#DNS
calaos_config set dns_address $DNS

echo "Update done."
