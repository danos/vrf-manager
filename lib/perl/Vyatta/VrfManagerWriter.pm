#!/usr/bin/perl

# Copyright (c) 2017-2019, AT&T Intellectual Property.  All rights reserved.
# Copyright (c) 2016 by Brocade Communications Systems, Inc.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

package Vyatta::VrfManagerWriter;

use strict;
use warnings;

use Exporter qw(import);
use File::Temp qw(tempfile);

my $FILE = "/run/route-domain.conf";

our @EXPORT_OK = qw(set_vrf_map unset_vrf_map);

sub get_vrf_lines {
    my @list;
    open my $read_fh, "<", $FILE or return @list;
    @list = <$read_fh>;
    close $read_fh;
    return @list;
}

# Exported routines
sub set_vrf_map {
    my $vrf_name      = shift;
    my $pbr_tbl_id    = shift;
    my $kernel_tbl_id = shift;

    if ( !defined $pbr_tbl_id || !defined $vrf_name ) {
        return;
    }

    my @file_lines = get_vrf_lines();
    ( my $temp_fh, my $temp_fname ) = tempfile( DIR => "/run" );
    foreach my $line (@file_lines) {
        print {$temp_fh} $line;
    }
    print {$temp_fh} "$vrf_name $pbr_tbl_id $kernel_tbl_id\n";
    close $temp_fh;
    chmod 0644, $temp_fname;
    rename $temp_fname, $FILE;
    return;
}

sub unset_vrf_map {
    my $vrf_name   = shift;
    my $pbr_tbl_id = shift;

    if ( !defined $pbr_tbl_id || !defined $vrf_name ) {
        return;
    }

    my @file_lines = get_vrf_lines();
    ( my $temp_fh, my $temp_fname ) = tempfile( DIR => "/run" );
    foreach my $line (@file_lines) {
        print {$temp_fh} $line unless ( $line =~ /^$vrf_name $pbr_tbl_id / );
    }
    close $temp_fh;
    chmod 0644, $temp_fname;
    rename $temp_fname, $FILE;
    return;
}

1;
