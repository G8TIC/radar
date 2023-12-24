#!/usr/bin/perl
#
# issue.pl - create a version and build id for the project using Git ids
# Copyright (c) 2007 Dirk Koopman, G1TLH
#
# Modified by Mike Tubby 12-Feb-2022
#

use strict;
use vars qw($root);

my $fn = "./issue.h";
my ($version, $count, $id) = ("", "", "");

if (-e "/usr/bin/git") {
	my $desc = `git describe --tags`;
	chomp $desc;
	($version, $count, $id) = $desc =~ /([-\w.]+?)(?:-(\d+))?-g(\w+)$/;
	$count ||= '0';		# replace null count with zero
}

my $user = `whoami`;
chomp $user;

my $host = `hostname`;
chomp $host;

my $arch = `uname -m`;
chomp $arch;

open FOUT, ">$fn" or die "issue.pl: can't open $fn $!\n";

print FOUT qq(/*
 * issue.h -- Software build and git version information
 *
 * DO NOT ALTER THIS FILE. It is generated automatically and will be overwritten.
 * DO NOT PLACE UNDER VERSION CONTROL. It will break the system.
 *
 */

#ifndef GIT_VERSION
#define GIT_VERSION "$version"
#define GIT_COUNT "$count"
#define GIT_ID "$id"
#endif

#ifndef BUILD_HOST
#define BUILD_HOST "$host"
#define BUILD_USER "$user"
#define BUILD_ARCH "$arch"
#endif
);
