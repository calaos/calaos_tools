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
# Extract/Check/Update the firmware to another revision
#

# Do the work in a detached process, so it can't be killed by 
# the parent (calaosd/calaos_gui)

drive=`cat /etc/drive`

if [ $PPID -ne 1 ] ; then

	( ( fw_update.sh & ) & )
	
	exit 0
fi

test -f /tmp/update_in_progress && exit 0
echo "update" > /tmp/update_in_progress

mkdir -p /mnt/ext3/log
LOG="/mnt/ext3/log/fwupdate_log"

rm -f $LOG

log ()
{
        echo $1 >> $LOG
}

exit_with_error ()
{
        log $1
        exit 1
}

log "Starting update system"
log "`date`"

#Mount the vfat partition
log "Unmounting vfat partition in case it's already mounted"
umount -n /mnt/vfat/ >> $LOG 2>> $LOG
log "Done."

log "Mounting vfat partition"
mount -n ${drive}1 /mnt/vfat/ >> $LOG 2>> $LOG || exit_with_error "Command failed: mount -n ${drive}1 /mnt/vfat/"
test -d /mnt/vfat/tmp && (rm -fr /mnt/vfat/tmp >> $LOG 2>> $LOG)
mkdir /mnt/vfat/tmp >> $LOG 2>> $LOG
log "Done."

#extract and check
log "Extract files..."
( cd /tmp/; tar xjvf /tmp/image.tar.bz2 -C /mnt/vfat/tmp >> $LOG 2>> $LOG );

log "Test files"
test -f /mnt/vfat/tmp/rootfs >> $LOG 2>> $LOG || exit_with_error "Command failed: test -f /mnt/vfat/tmp/rootfs"
test -f /mnt/vfat/tmp/vmlinuz >> $LOG 2>> $LOG || exit_with_error "Command failed: test -f /mnt/vfat/tmp/vmlinuz"
test -f /mnt/vfat/tmp/syslinux.cfg >> $LOG 2>> $LOG || exit_with_error "Command failed: test -f /mnt/vfat/tmp/syslinux.cfg"
test -f /mnt/vfat/tmp/VERSION >> $LOG 2>> $LOG || exit_with_error "Command failed: test -f /mnt/vfat/tmp/VERSION"
log "Files OK."

log "Clean /tmp"
rm -fr /tmp/image.tar.bz2  >> $LOG 2>> $LOG
log "Done."

log "Getting version:"
VERSION=`cat /mnt/vfat/tmp/VERSION`
log "fw revision $VERSION"

#clean old files if any
log "Cleaning old rootfs files..."
rm -f /mnt/vfat/rootfs.old /mnt/vfat/vmlinuz.old /mnt/vfat/syslinux.old >> $LOG 2>> $LOG
log "Done."

#copy
log "Begin copy of rootfs..."
mv /mnt/vfat/tmp/rootfs /mnt/vfat/rootfs.new >> $LOG 2>> $LOG || exit_with_error "Command failed: mv /mnt/vfat/tmp/rootfs /mnt/vfat/rootfs.new"
log "Done."

log "Begin copy of kernel..."
mv /mnt/vfat/tmp/vmlinuz /mnt/vfat/vmlinuz.new >> $LOG 2>> $LOG || exit_with_error "Command failed: mv /mnt/vfat/tmp/vmlinuz /mnt/vfat/vmlinuz.new"
log "Done."

log "Begin copy of syslinux.cfg..."
mv /mnt/vfat/tmp/syslinux.cfg /mnt/vfat/syslinux.new >> $LOG 2>> $LOG || exit_with_error "Command failed: mv /mnt/vfat/tmp/syslinux.cfg /mnt/vfat/syslinux.new"
log "Done."

log "## Start Critical Operations ##"
#now we do some critical operations, if the power fails now
#the system could not be able to boot anymore !
log "Moving old rootfs"
mv /mnt/vfat/rootfs /mnt/vfat/rootfs.old >> $LOG 2>> $LOG || exit_with_error "Command failed: mv /mnt/vfat/rootfs /mnt/vfat/rootfs.old"
log "Done."

log "Moving old kernel"
mv /mnt/vfat/vmlinuz /mnt/vfat/vmlinuz.old >> $LOG 2>> $LOG || exit_with_error "Command failed: mv /mnt/vfat/vmlinuz /mnt/vfat/vmlinuz.old"
log "Done."

log "Moving old syslinux.cfg"
mv /mnt/vfat/syslinux.cfg /mnt/vfat/syslinux.old >> $LOG 2>> $LOG || exit_with_error "Command failed: mv /mnt/vfat/syslinux.cfg /mnt/vfat/syslinux.old"
log "Done."

log "Moving new rootfs"
mv /mnt/vfat/rootfs.new /mnt/vfat/rootfs >> $LOG 2>> $LOG || ( mv /mnt/vfat/rootfs.old /mnt/vfat/rootfs ; exit_with_error "Command failed: mv /mnt/vfat/rootfs.new /mnt/vfat/rootfs" )
log "Done."

log "Moving new kernel"
mv /mnt/vfat/vmlinuz.new /mnt/vfat/vmlinuz >> $LOG 2>> $LOG || ( mv /mnt/vfat/vmlinuz.old /mnt/vfat/vmlinuz ; exit_with_error "Command failed: mv /mnt/vfat/vmlinuz.new /mnt/vfat/vmlinuz" )
log "Done."

log "Moving new syslinux.cfg"
mv /mnt/vfat/syslinux.new /mnt/vfat/syslinux.cfg >> $LOG 2>> $LOG || ( mv /mnt/vfat/syslinux.old /mnt/vfat/syslinux.cfg ; exit_with_error "Command failed: mv /mnt/vfat/syslinux.new /mnt/vfat/syslinux.cfg" )
log "Done."
#done
log "## Done ##"

#Ok, the update is now installed. The last thing is to update the firmware info
#in the local_config.xml
log "Update firmware version in local_config.xml"
/sbin/calaos_config set "fw_version" "$VERSION" >> $LOG 2>> $LOG
log "version: `/sbin/calaos_config set "fw_version"`"

log "Clean vfat partition."
rm -fr /mnt/vfat/tmp >> $LOG 2>> $LOG
log "Done."

#Do a backup of all config files to the VFAT partition, in case something failed we can restore from here
log "Backup config files"
rm /mnt/vfat/backup_conf.tar.gz -f >> $LOG 2>> $LOG
(cd /etc/calaos/ ;
 tar czvf /mnt/vfat/backup_conf.tar.gz * >> $LOG 2>> $LOG
);
log "Done."

#clean old files if any
log "Cleaning old rootfs files..."
rm -f /mnt/vfat/rootfs.old /mnt/vfat/vmlinuz.old /mnt/vfat/syslinux.old >> $LOG 2>> $LOG
log "Done."

#we don't care about all files in /tmp, they'll be erased the next time we boot
log "Unmount vfat filesystem"
umount -n /mnt/vfat/ >> $LOG 2>> $LOG
log "Done."

#tell calaos.fr that we have updated
USER_ID=`/sbin/calaos_config get "calaos_id"`
CALAOS_URL="https://home.calaos.fr:8445/"

VERSION=`/sbin/calaos_config get "fw_version"`

if [ "$USER_ID" != "" ] ; then
        wget $CALAOS_URL --post-data "id=$USER_ID&fw_version=$VERSION&set=fw_version" --no-check-certificate --quiet --output-document=/dev/null
fi

log "Sync FS before reboot"
sync  >> $LOG 2>> $LOG

reboot
