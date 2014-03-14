#!/usr/bin/perl

open F,"libraw_build.h";
$l1 = <F>;
$l2 = <F>;
$l3 = <F>;

@a = split(/\s+/,$l2);
$a[2]++;
$l2 = "$a[0] $a[1] $a[2]\n";
open F,">libraw_build.h";
print F "$l1$l2$l3";