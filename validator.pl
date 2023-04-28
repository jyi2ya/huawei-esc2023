#!/usr/bin/env perl
use v5.20;
use utf8;
use warnings;
use feature 'signatures';
use open qw(:std :utf8);

if (@ARGV < 2) {
    say "usage: perl $0 <input_filename> <result_filename>";
    exit 1;
}

my ($input_filename, $result_filename) = @ARGV;
open my $input_fd, '<', $input_filename or die "cannot open input file";

my $first_line = <$input_fd>;
my ($N, $M, $T, $P, $D) = ($first_line =~ /^(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)/);

my @graph_edges;
my %graph;
for my $id (0 .. ($M - 1)) {
    my $line = <$input_fd>;
    my ($s, $t, $d) = ($line =~ /^(\d+)\s+(\d+)\s+(\d+)/);
    $graph_edges[$id] = {
        src => $s,
        dst => $t,
        dist => $d,
        channels => [],
    };
    push @{$graph{$s}{$t}}, {
        dist => $d,
        id => $id,
    };
    push @{$graph{$t}{$s}}, {
        dist => $d,
        id => $id,
    };
}

my @tasks;
for my $id (0 .. ($T - 1)) {
    my $line = <$input_fd>;
    my ($s, $t) = ($line =~ /^(\d+)\s+(\d+)/);
    push @tasks, {
        src => $s,
        dst => $t,
        id => $id,
    };
}

my %cost = (
    add_edge => 0,
    edge => 0,
    relay => 0,
);

open my $result_fd, '<', $result_filename or die "cannot open result file";
my $lineno = 1;
my $Y = int(<$result_fd>);
for my $id ($M .. ($M + $Y - 1)) {
    $lineno += 1;
    my $line = <$result_fd>;
    my ($s, $t) = ($line =~ /^(\d+)\s+(\d+)/);
    die "cannot add edge: result file line $lineno, from $s to $t. no existing edge between nodes." unless defined $graph{$s}{$t};
    my $dist = 1e16;
    for my $edge (@{$graph{$s}{$t}}) {
        $dist = $edge->{dist} if $edge->{dist} < $dist;
    }
    $graph_edges[$id] = {
        src => $s,
        dst => $t,
        dist => $dist,
        channels => [],
    };
    push @{$graph{$s}{$t}}, {
        dist => $dist,
        id => $id,
    };
    push @{$graph{$t}{$s}}, {
        dist => $dist,
        id => $id,
    };

    $cost{add_edge} += 1_000_000;
}

for my $task (0 .. ($T - 1)) {
    $lineno += 1;
    my $line = <$result_fd>;
    my ($p, $m, $n, @res) = split /\s+/, $line;
    my @edges = @res[0 .. ($m - 1)];
    my @relays = @res[$m .. ($m + $n - 1)];
    my ($edges_num, $relays_num) = (scalar(@edges), scalar(@relays));
    die "number of relays and edges doesnt match. result file line $lineno: edges = $edges_num, expect $m; relays = $relays_num, expect $n;" unless $edges_num == $m and $relays_num == $n;

    my ($src_ok, $dst_ok);
    for my $edge (@edges) {
        if ($graph_edges[$edge]->{src} == $tasks[$task]->{src} or $graph_edges[$edge]->{dst} == $tasks[$task]->{src}) {
            $src_ok = 1;
        }
        if ($graph_edges[$edge]->{src} == $tasks[$task]->{dst} or $graph_edges[$edge]->{dst} == $tasks[$task]->{dst}) {
            $dst_ok = 1;
        }
        if (defined $graph_edges[$edge]->{channels}->[$p]) {
            my $channel = $graph_edges[$edge]->{channels}->[$p];
            die "channel conflict: channel $p on edge $edge, from $graph_edges[$edge]->{src} to $graph_edges[$edge]->{dst}, required by task $task, occupied by task $channel->{task}";
        }
        $graph_edges[$edge]->{channels}->[$p]->{task} = $task;
        $cost{edge} += 1;
    }
    unless ($src_ok and $dst_ok) {
        warn "doesnt resolve task #$task: $tasks[$task]->{src} -> $tasks[$task]->{dst}";
        warn "path:";
        for my $edge (@edges) {
            warn "edge #$edge: $graph_edges[$edge]->{src} <=> $graph_edges[$edge]->{dst}";
        }
        die;
    }

    my %relay_graph = map { $_ => 1 } @relays;
    my $dist = 0;
    my $last_dst = $tasks[$task]->{src};
    for my $edge_id (@edges) {
        my $edge = $graph_edges[$edge_id];
        my ($s, $t) = ($edge->{src}, $edge->{dst});
        ($s, $t) = ($t, $s) if ($edge->{dst} == $last_dst);
        $last_dst = $t;
        if ($relay_graph{$s}) {
            $dist = 0;
            $cost{relay} += 100;
        }
        $dist += $edge->{dist};
        if ($dist > $D) {
            warn "doesnt resolve task #$task: $tasks[$task]->{src} -> $tasks[$task]->{dst}: no enough relays";
            warn "path:";
            for my $edge (@edges) {
                if ($edge_id == $edge) {
                    warn "X broken here: edge #$edge: $graph_edges[$edge]->{src} <=> $graph_edges[$edge]->{dst}";
                } else {
                    warn "edge #$edge: $graph_edges[$edge]->{src} <=> $graph_edges[$edge]->{dst}";
                }
            }
            die;
        }
    }
}

my $cost = 0;
while (my ($k, $v) = each %cost) {
    $cost += $v;
    say "$k: $v";
}
say "total: $cost";
