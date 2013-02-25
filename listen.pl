#!/usr/bin/perl -w
#######################################################################
# ecoute.pl
#######################################################################

use strict;
use Net::Telnet ();

# pour Flusher le STDOUT violement
$| = 1;

my $hostname    = "127.0.0.1";
my $port        = 4456;
my $user        = "user";
my $pass        = "pass";


### Uniquement pour faire un fallback vide
sub armure {
        }

my $listen = new Net::Telnet ( Telnetmode => 0,
        Errmode => \&armure );

$listen->open(Host => $hostname,Port => $port);
my $line="";

## Connexion....
$listen->print("login $user $pass");
$line=$listen->getline;
die $line unless $line =~ /ok/;
print "Procedure de login ok\n";

## Listen
$listen->print("listen");


while(1) {
        $line=$listen->getline;
        if($line) {
                chomp($line);
                print "Evenement recu :$line\n";
                }
        }

$listen->close;
exit;
