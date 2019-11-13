#!/usr/bin/perl

# Copyright (c) 2017-2019, AT&T Intellectual Property.  All rights reserved.
# Copyright (c) 2016 by Brocade Communications Systems, Inc.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

package Vyatta::VrfManager;

use strict;
use warnings;
use Readonly;
use File::Slurp qw(read_file);
use File::Basename;

Readonly::Scalar our $VRFID_INVALID    => 0;
Readonly::Scalar our $VRFID_DEFAULT    => 1;
Readonly::Scalar our $VRFNAME_NONE     => "";
Readonly::Scalar our $VRFNAME_DEFAULT  => "default";
Readonly::Scalar our $VRFNAME_MAX_LEN  => 12;
Readonly::Scalar our $VRFMASTER_PREFIX => "vrf";

my $FILE = "/run/route-domain.conf";
use Exporter qw(import);

our @EXPORT_OK =
  qw(get_vrf_id get_vrf_name get_vrf_list get_vrf_name_map get_tbl_id_map
  get_interface_vrf_id get_interface_vrf update_interface_vrf
  $VRFID_INVALID $VRFID_DEFAULT $VRFNAME_NONE $VRFNAME_DEFAULT $VRFMASTER_PREFIX
  $VRFNAME_MAX_LEN);

# Exported routines

# Mangle the list into the form that users expect, i.e. "<name> <id>\n"
sub get_vrf_list {
    # Using the name hash ensures we get uniqueness of the names
    my $vrf_hash = get_vrf_name_map();

    return map {
        $_ . " " . get_vrf_id($_) . "\n"
    } keys %$vrf_hash;
}

sub get_vrf_id {
    my $vrf_name = shift;

    return $VRFID_DEFAULT if $vrf_name eq $VRFNAME_DEFAULT;

    my $intf = $VRFMASTER_PREFIX . $vrf_name;

    open( my $sysf, '<', "/sys/class/net/$intf/ifindex" )
      or return $VRFID_INVALID;

    my $value = <$sysf>;
    chomp $value if defined $value;
    close $sysf;
    return $value;
}

sub get_vrf_name {
    my $vrf_id = shift;

    if ( $vrf_id == $VRFID_DEFAULT ) {
        return $VRFNAME_DEFAULT;
    }

    if ( $vrf_id <= $VRFID_INVALID ) {
        return $VRFNAME_NONE;
    }

    opendir(my $dh, "/sys/class/net") || return $VRFNAME_NONE;
    foreach my $intf (readdir $dh) {
        if (substr($intf, 0, length($VRFMASTER_PREFIX)) eq $VRFMASTER_PREFIX &&
            get_vrf_id(substr($intf, length($VRFMASTER_PREFIX))) == $vrf_id) {
            closedir $dh;
            return substr($intf, length($VRFMASTER_PREFIX));
        }
    }
    closedir $dh;

    return $VRFNAME_NONE;
}

my $vrf_pat = qr/^(\S+)\s+(\d+)\s+(\d+).*$/;
# returns a reference to hash
# vrfname => pbr-table => kernel-table
sub get_vrf_name_map {
    my %name_hash;
    foreach my $line (read_file($FILE, 'err_mode' => 'quiet')) {
        my ($name, $pbr_table, $kernel_table) = ($line =~ /$vrf_pat/);
	$name_hash{$name}{$pbr_table} = $kernel_table;
    }
    return \%name_hash;
}

# return a reference to hash
# kernel-table => (vrf-name, pbr-table)
sub get_tbl_id_map {
    my %table_hash;
    foreach my $line (read_file($FILE, 'err_mode' => 'quiet')) {
	my ($name, $pbr_table, $kernel_table) = ($line =~ /$vrf_pat/);
	$table_hash{$kernel_table} = [$name, $pbr_table];
    }
    return \%table_hash;
}

# Get the VRF ID for an interface
sub get_interface_vrf_id {
    my $intf = shift;
    my $vrf = get_interface_vrf($intf);

    return get_vrf_id($vrf);
}

# Get the VRF name for an interface
# Returns
sub get_interface_vrf {
    my $intf = shift;
    my $master_path = readlink "/sys/class/net/$intf/master";
    my $master = $VRFNAME_DEFAULT;
    # Strip off $VRFMASTER_PREFIX from master device name to get VRF
    $master = substr(basename($master_path), length($VRFMASTER_PREFIX)) if
        defined($master_path) &&
        substr(basename($master_path), 0, length($VRFMASTER_PREFIX)) eq $VRFMASTER_PREFIX;
    return $master;
}

# Is the interface a bridge (or switch) port?
sub is_bridge_port {
    my $ifname = shift;

    if (-d "/sys/class/net/$ifname/brport") {
        return 1;
    } else {
        return 0;
    }
}

# Is the interface a slave in a bonding group?
sub is_bonding_slave {
    my $ifname = shift;

    # check IFF_ flags
    my $IFF_SLAVE = 0x800;
    my @lines = read_file("/sys/class/net/$ifname/flags", chomp => 1);
    my $flags = hex($lines[0]);

    if ($flags & $IFF_SLAVE) {
        return 1;
    } else {
        return 0;
    }
}

# Move interface into a VRF.
# assumes interface exists.
# flaps interface based on $intf_up.
sub update_interface_vrf {
    my ( $ifname, $vrf, $intf_up ) = @_;
    my $rc;

    # Deferred config
    return 0 if !-d "/sys/class/net/$ifname";

    if (is_bridge_port($ifname)) {
        if ($vrf ne '' && $vrf ne 'default') {
            warn "Cannot change bridge interface $ifname to $vrf\n";
        }
        return 1;
    } elsif (is_bonding_slave($ifname)) {
        if ($vrf ne '' && $vrf ne 'default') {
            warn "Cannot change bonding slave interface $ifname to $vrf\n";
         }
         return 1;
    }

    if ($intf_up) {
        system("ip", "link", "set", "dev", $ifname, "down") == 0
          or warn "Cannot change interface $ifname state down\n";
    }
    if ($vrf eq '' || $vrf eq 'default') {
        $rc = system "ip", "link", "set", "dev", $ifname, "nomaster";
    }
    else {
        $rc = system "ip", "link", "set", "dev", $ifname, "master", $VRFMASTER_PREFIX . $vrf;
    }
    warn "Cannot change interface $ifname to $vrf\n" unless $rc == 0;
    if ($intf_up) {
        system ("ip", "link", "set", "dev", $ifname, "up") == 0
          or warn "Cannot change interface $ifname state up\n";
    }
    return $rc == 0;
}

1;
