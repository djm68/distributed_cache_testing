#!/usr/bin/perl

my $npids = shift @ARGV or die;
for(my $n = 0; $n != $npids; ++$n)
{
	my $pid = fork;
	die "Could not fork" if $pid < 0;
	if($pid == 0)
	{
		system("./get_dave -o1000 -p4");
		exit;
	}
}
 
 
for(my $n = 0; $n != $npids; ++$n)
{
	print STDERR "WAIT $n\n";
	wait;
}
