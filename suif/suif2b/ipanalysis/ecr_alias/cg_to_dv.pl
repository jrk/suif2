#!/usr/local/bin/perl -w
$node = {};
while ($line = <>) {
  chomp $line;
  if ($line =~ /^CALL: (.*)->(.*)$/) {
    my($src,$tgt) = ($1, $2);
    if (!defined($node->{$src})) {
      $node->{$src} = {};
    }
    if (!defined($node->{$tgt})) {
      $node->{$tgt} = {};
    }
    $node->{$src}->{$tgt} = 1;
  }
}

# print it out

$defined = {};

sub print_edges {
  my($src, $level) = @_;
  my($dst);
  print "[";
  my($need_comma) = 0;
  foreach $dst (keys(%{$node->{$src}})) {
    if($need_comma) { print ","."\n"." "x$level; } 
    $need_comma = 1;
    print "e(\"$src->$dst\",[],";
    &print_node($dst, $level+2);
    print ")";
  }
  print "]";
}
sub print_node {
  my($src, $level) = @_;
  if (defined($defined->{$src})) {
    print "r(\"$src\")";
  } else {
    print "\n"." "x$level;
    $defined->{$src} = 1;
    print "l(\"$src\",n(\"$src\",[],";
    &print_edges($src, $level+2);
    print "))";
    print "\n". " "x$level;
  }
}

my($src);
print "[\n";
my($need_a_comma) = 0;
foreach $src ("main", keys(%$node)) {
  if (!defined($node->{$src})) { next; }
  if (!defined($defined->{$src})) {
    if ($need_a_comma) { print ",\n"; } 
    $need_a_comma = 1;
    &print_node($src, 0);
  } 
}
print "]\n";
