#!/usr/bin/env perl
use v5.20;
use utf8;
use warnings;
use feature 'signatures';
use open qw(:std :utf8);

my $N = int(rand(4000)) + 100;
my $M = int(rand(4500 - $N)) + $N;
my $T = int(rand(9000)) + 100;
my $P = int(rand(50)) + 4;
my $D = int(rand(900)) + 50;
say "$N $M $T $P $D";

for my $i (1 .. ($N - 1)) {
    my ($s, $t) = ($i, int(rand($i)));
    my $d = int(rand($D - 10)) + 10;
    say "$s $t $d";
    $M--;
}

while ($M--) {
    my ($s, $t) = (0, 0);
    ($s, $t) = (int(rand($N)), int(rand($N))) while $s == $t;
    my $d = int(rand($D - 10)) + 10;
    say "$s $t $d";
}

while ($T--) {
    my ($s, $t) = (0, 0);
    ($s, $t) = (int(rand($N)), int(rand($N))) while $s == $t;
    say "$s $t";
}
