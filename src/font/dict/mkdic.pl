#!/usr/bin/perl
#
# Make Kanji Dictionary for MB-S1
#
# Programmed by Sasaji 2025
#

my $infile = 's1dic.txt';
my $outfile = 'S1DIC.ROM';

my $fhi,$fho;

my @aindex = ();
my %dhash = ();
my @threesk = (); # Three strokes

sub sjis_to_jis
{
	my $h = unpack("C", substr($_[0],0,1));
	my $l = unpack("C", substr($_[0],1,1));

#	printf("%x %x\n",$h,$l);

	if (!((0x81 <= $h && $h <= 0x9f) || (0xe0 <= $h && $h <= 0xff))) {
		# 1byte
		return ($h, $l);
	}
	# convert the first code
	if ($h <= 0x9f) {
		if ($l < 0x9f) {
			$h = ($h << 1) - 0xe1;
		} else {
			$h = ($h << 1) - 0xe0;
		}
	} else {
		if ($l < 0x9f) {
			$h = ($h << 1) - 0x161;
		} else {
			$h = ($h << 1) - 0x160;
		}
	}
	if ($l < 0x7f) {
		$l = $l - 0x1f;
	} elsif ($l < 0x9f) {
		$l = $l - 0x20;
	} else {
		$l = $l - 0x7e;
	}

	return ($h, $l);
}

## main

{
	if (!open($fhi, $infile)) {
		print "Cannot open: ".$infile."\n";
		last;
	}

	# make index
	my $c;
	for(my $ch = 0xb1; $ch <= 0xdd; $ch++) {
		$c = pack("C", $ch);
		$dhash{$c}={
			'ch' => $c,
			'count' => 0,
			'data' => [],
		};
	}
	$c = pack("C", 0xa6);
	$dhash{$c}={
		'ch' => $c,
		'count' => 0,
		'data' => [],
	};

	my $phase = 0;
	my $line_num = 0;
	while (my $line = <$fhi>) {
		chomp($line);
		$line =~ s/[\r\n]+$//;
		$line_num++;
		
		if ($line =~ /^$/) {
			$phase++;
			if ($phase < 2) {
				next;
			} else {
				last;
			}
		}

		if ($phase == 0) {
			# phase 0 : 単漢字辞書
			my $word;
			my $kanji;
#			print $line."\n";
			if ($line =~ /^(.+):(.+)$/) {
				$word = $1;
				$kanji = $2;
			} else {
				# no data ??
				print "Cannot parse or invalid charactor exists in line $line_num\n";
				last;
			}
			my $ch = substr($word,0,1);
			if (!exists($dhash{$ch})) {
				print "First charactor is invalid in line $line_num\n";
				$error = 1;
				last;
			}
			$dhash{$ch}{'count'}++;
			push(@{$dhash{$ch}{'data'}}, { 'word' => $word, 'kanji' => $kanji });
		} elsif ($phase == 1) {
			# phase 1 : 3ストローク辞書
			my $word;
			my $kanji;
			if ($line =~ /^(.{3}):(.+)$/) {
				$word = $1;
				$kanji = $2;
			} else {
				# no data ??
				print "Cannot parse or invalid charactor exists in line $line_num\n";
				last;
			}
			push(@threesk, { 'word' => $word, 'kanji' => $kanji });
		}
	}

	# ｱｲｳｴｵ .... ﾜｦﾝ
	for(my $ch = 0xb1; $ch <= 0xdc; $ch++) {
		$c = pack("C", $ch);
		push(@aindex, $dhash{$c});
	}
	$c = pack("C", 0xa6);
	push(@aindex, $dhash{$c});
	$c = pack("C", 0xdd);
	push(@aindex, $dhash{$c});

	# calculate length
	foreach my $item (@aindex) {
		my $len = 0;
		foreach my $data (@{$item->{'data'}}) {
			$len += length($data->{'word'}) + length($data->{'kanji'}) + 1;
#			print $data->{'word'}." ".$data->{'kanji'}." len:".$len."\n";
		}
		$item->{'length'} = $len;
	}

	# output
	if (!open($fho, ">".$outfile)) {
		print "Cannot open: ".$outfile."\n";
		last;
	}
	binmode($fho);

	# index
	my $addr = 0x008b;
	foreach my $item (@aindex) {
		print {$fho} $item->{'ch'};
		print {$fho} pack("C", $addr >> 8);
		print {$fho} pack("C", $addr & 0xff);
		$addr += $item->{'length'};
	}
	print {$fho} pack("C", 0);

	# tankanji henkan data
	foreach my $item (@aindex) {
		foreach my $data (@{$item->{'data'}}) {
			my $len = length($data->{'word'}) + length($data->{'kanji'})+1;
			print {$fho} pack("C", $len & 0xff);
			print {$fho} substr($data->{'word'}, 1) if (length($data->{'word'}) > 1);
			print {$fho} pack("C", 0);
			for(my $i=0; $i<length($data->{'kanji'}); $i+=2) {
				my @jis = sjis_to_jis(substr($data->{'kanji'}, $i, 2));
				print {$fho} pack("CC",$jis[0],$jis[1]);
			}
		}
	}
	if ($addr >= 0x7000) {
		print "Tankanji dictionary size is overflow. The size should be less than 0x7000.\n";
		last;
	}
	# padding
	for(;$addr < 0x7000; $addr++) {
		print {$fho} pack("C", 0);
	}

	# 3 strokes
	foreach my $data (@threesk) {
		print {$fho} $data->{'word'};
		my @jis = sjis_to_jis(substr($data->{'kanji'}, 0, 2));
		print {$fho} pack("CC",$jis[0],$jis[1]);
		$addr+=5;
	}
	if ($addr >= 0x8000) {
		print "Three strokes dictionary size is overflow. The size should be less than 0x8000.\n";
		last;
	}
	# padding
	for(;$addr < 0x8000; $addr++) {
		print {$fho} pack("C", 0);
	}
}

close($fho) if ($fho);
close($fhi) if ($fhi);

print "Done.\n";
getc(STDIN);

exit 0;
