#!/usr/local/bin/perl -w
my($use_direct) = 0;
if ($#ARGV < 0) {
  &Usage("Not enough arguments");
}

sub Usage {
  my($message) = @_;
  print STDERR "Usage: ".$ARGV[0]." -direct inputfile\n";
  print STDERR "ERROR: $message\n";
  exit 1;
}
while ($#ARGV != -1) {
  if ($ARGV[0] eq "-direct") {
    $use_direct = 1; shift; next;
  }
  if ($ARGV[0] eq "-deep") {
    $do_deep = 1; shift; next;
  }
  if ($ARGV[0] eq "-fast") {
    $do_fast = 1; shift; next;
  }
  last;
}
if ($#ARGV < 0) {
  &Usage("Not enough arguments");
}
my($file) = $ARGV[0];
open(IN, "<$file") || die ("Could not open $file");

$graph = {};
while ($line = <IN>) {
  chomp $line;
  if ($line =~ /^(I?)CALL: (.*)->(.*)$/) {
    my($ind, $src,$dst) = ($1, $2, $3);
    if ($use_direct && ($ind eq "I")) { next; }
    if (!defined($graph->{$src})) {
      $graph->{$src} = {};
    }
    if (!defined($graph->{$dst})) {
      $graph->{$dst} = {};
    }
#    print "Adding : $src->$dst\n";
    $graph->{$src}->{$dst} = 1;
  }
}
close(IN);

# print it out
#print_davinci_graph($graph);
&build_scc($graph);
#&print_GTL($graph);

#$defined = {};

sub print_davinci_edges {
  my($graph, $defined, $src, $level) = @_;
  my($dst);
  print "[";
  my($need_comma) = 0;
  foreach $dst (keys(%{$graph->{$src}})) {
    if($need_comma) { print ","."\n"." "x$level; } 
    $need_comma = 1;
    print "e(\"$src->$dst\",[],";
    &print_davinci_node($graph, $defined, $dst, $level+2);
    print ")";
  }
  print "]";
}
sub print_davinci_node {
  my($graph, $defined, $src, $level) = @_;
  if (defined($defined->{$src})) {
    print "r(\"$src\")";
  } else {
    print "\n"." "x$level;
    $defined->{$src} = 1;
    print "l(\"$src\",n(\"$src\",[],";
    &print_davinci_edges($graph, $defined, $src, $level+2);
    print "))";
    print "\n". " "x$level;
  }
}

sub print_davinci_graph {
  my($graph) = @_; 
  my($defined) = {};
  my($src);
  print "[\n";
  my($need_a_comma) = 0;
  foreach $src ("main", keys(%$graph)) {
    if (!defined($graph->{$src})) { next; }
    if (!defined($defined->{$src})) {
      if ($need_a_comma) { print ",\n"; } 
      $need_a_comma = 1;
      &print_davinci_node($graph, $defined, $src, 0);
    } 
  }
  print "]\n";
}

sub print_GTL_edges {
  my($graph, $src, $name_id) = @_;
  my($srcid) = $name_id->{$src};
  my($dst);
  foreach $dst (keys(%{$graph->{$src}})) {
    my($dstid) = $name_id->{$dst};
    print "edge [\n";
    print "source $srcid\n";
    print "target $dstid\n";
    print " ]\n";
  }
}
sub print_GTL_node {
  my($graph, $src, $name_id) = @_;
  my($id) = $name_id->{$src};
  print "node [\n".
	     "id $id\n".
	     "label \"$src\" \n".
	     "]\n";
}

sub print_GTL {
  my($graph) = @_; 
  my($name_id) = {};
  my($src);
  my($defined) = {};
  my($id) = 0;
  print "graph [\n";
  foreach $src ("main", keys(%$graph)) {
    if (!defined($graph->{$src})) { next; }
    if (!defined($name_id->{$src})) {
      $name_id->{$src} = $id;
      $id++;
      &print_GTL_node($graph, $src, $name_id);
    } 
  }

  foreach $src ("main", keys(%$graph)) {
    if (!defined($graph->{$src})) { next; }
    if (!defined($defined->{$src})) {
      $defined->{$src} = 1;
      &print_GTL_edges($graph, $src, $name_id);
    } 
  }
  print "]\n";
}



sub dfs {
  my($src, $node_id, $dfsnum,
     $starttime_p, $SC, $touched,
     $low, $dfslist) = @_;

  push(@$dfslist, $src);
  my($srcid) = $node_id->{$src};  # get an id number
  ${$starttime_p}++;
  my($dfsid) = ${$starttime_p};
  $dfsnum->{$src} = $dfsid;
  $low->{$src} = $dfsid;
  
#  print "Dfs->$src $dfsid\n";
  $touched->{$src} = 0;

  my($dst);
  foreach $dst (keys(%{$graph->{$src}})) {
    my($dstid) = $node_id->{$dst};
    if ($dfsnum->{$dst} == 0) {
      &dfs($dst, $node_id, $dfsnum, 
	   $starttime_p, $SC, $touched, $low);

      if ($low->{$dst} < $low->{$src}) {
	$low->{$src} = $low->{$dst};
      }
    } else {
      if (! $touched->{$dst}) {
	if ($dfsnum->{$dst} < $low->{$src}) {
	  $low->{$src} = $dfsnum->{$dst};
	}
      }
    }
  }
  if ($low->{$src} == $dfsnum->{$src}) {
    $touched->{$src} = 1;
###    print "$src\n";
#    print "SC_SIZE : ".$#{$SC}."\n";
    while (($#{$SC} != -1) &&
	   ($dfsnum->{$SC->[0]} > $dfsnum->{$src})) {
###      print $SC->[0]."\n";
      $touched->{$SC->[0]} = 1;
      shift(@$SC);
    }
  } else {
    @{$SC} = ($src, @$SC);
  }
  
}
      
sub build_scc {
  my($graph) = @_;

  my($starttime) = 0; #global
  my($SC) = [];
  my($touched) = {};
  
  my($node_id) = {};
  my($dfsnum) = {};
  my($src);
  my($low) = {};
  my($id) = 0;
  my($dfslist) = [];
  foreach $src (keys(%$graph)) {
    $dfsnum->{$src} = 0;
  }
  foreach $src ("main", keys(%$graph)) {
    if (!defined($graph->{$src})) { next; }
    if (!defined($node_id->{$src})) {
      $node_id->{$src} = $id; 
      $id++;
    }
    my($srcid) = $node_id->{$src};
    if ($dfsnum->{$src} == 0) {
      &dfs($src, $node_id, $dfsnum,
	   \$starttime, $SC, $touched, $low,
	   $dfslist);
    }
  }


  # Now what we want is the total per number.
  my($hist) = {};
  foreach $name (keys(%$low)) {
    my($val) = $low->{$name};
    if (!defined($hist->{$val})) {
      $hist->{$val} = [];
    }
    push(@{$hist->{$val}}, $name);
  }
  my($scc_name) = {}; # map to a unique printable name
  foreach $val (sort {$a <=> $b} keys(%$hist)) {
    my($arr) = $hist->{$val};
    my($name) = join(" ", "$val:",$#$arr+1, @$arr);
#    print "$name\n";
    if ($do_fast) {
      $scc_name->{$val} = $val;
    } else {
      $scc_name->{$val} = $name;
    }
  }

  # let's build the graph (a tree) of the scc.
  # have each parent point to it's children.
  my($scc_graph);

  foreach $srclow (sort {$a <=> $b} keys(%$hist)) {
    my($src_name) = $scc_name->{$srclow};
    if (!defined($scc_graph->{$src_name})) {
      $scc_graph->{$src_name} = {};
    }
    my($arr) = $hist->{$srclow};
#    print join("\t", "VISITING SRCS:", @$arr, "\n");
    foreach $src (@$arr) {
      foreach $dst (keys(%{$graph->{$src}})) {
	my($dstlow) = $low->{$dst};
	my($dst_name) = $scc_name->{$dstlow};
#	print "Adding $srclow->$dstlow, $src->$dst: ($src_name)->($dst_name)\n";
	$scc_graph->{$src_name}->{$dst_name} = 1;
      }
    }
  }
  # Now we have an scc_graph.  we'll print it.
  my($main_scc_name) = $scc_name->{$low->{"main"}};
  &print_dfs($scc_graph, $main_scc_name, {}, {}, 5);


#  foreach $name (keys(%$low)) {
#    print "$name: ". $low->{$name}. "\n";
#  }
}

sub print_dfs {
  my($graph, $src, $visited, $visiting, $level) = @_;
  if (!defined($visited->{$src})) { $visited->{$src} = 0; }
  my($vnum) = $visited->{$src};
  my($levelstring) = "";
  if (!$do_fast) {
    $levelstring = " "x$level;
  }
  if (!$vnum) {
    print $levelstring."|$src\n";
  } else {
    print $levelstring."|(+$vnum)$src\n";
  }
  $level += 2;
  $visited->{$src}++;
  $visiting->{$src} = 1;
  foreach $dst (keys(%{$graph->{$src}})) {
#    print "$src->$dst\n";
    if (!$visited->{$dst}) {
      print_dfs($graph, $dst, $visited, $visiting, $level);
    } else {
      if ((!$visiting->{$dst})) { 
	if ($do_deep) {
	  print_dfs($graph, $dst, $visited, $visiting, $level);
	} else {
	  my($vnum) = $visited->{$dst};
	  print $levelstring."|->(+$vnum)$dst\n";
	  $visited->{$dst}++;
	}
      } else {
	if ($src eq $dst) {
	  print $levelstring."|SLOOP->$dst\n"; # was -5
	} else {
	  print $levelstring."|LOOP->$dst\n"; # was -4
	}
      }
    }
  }
  $visiting->{$src} = 0;
}
