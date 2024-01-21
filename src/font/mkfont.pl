#!/usr/bin/perl
# S1エミュレータのフォントファイルを作成
#
# Copyright (c) 2011-2015, Sasaji

my $infile;
my $outfile="S1FONT.ROM";
my $indata="";
my %outdatas=();
my $outdata="";


	$infile="display_font.bmp";
	print "-------- ".$infile."\n";
	$indata="";
	if (read_file($infile, \$indata)) {
		return 1;
	}
	if (check_bmp_header(\$indata)) {
		return 1;
	}

my $pos;
my $val;
my $chcode;

for($chcode=0; $chcode<0x200; $chcode++) {
	$outdatas{$chcode}=[];
	my $x = $chcode % 32;
	my $y = int((0x1ff - $chcode) / 32);
	print sprintf("%02X: X:%02d Y:%02d",$chcode,$x,$y);
	for(my $yy=0; $yy<16; $yy++) {
		$pos = $y * 32 * 16 + (15 - $yy) * 32 + $x + $offset;
		$val = substr($indata,$pos,1);
		printf(" %02X", unpack("C",$val));
		push(@{$outdatas{$chcode}}, $val);
	}
	print"\n";
}

for($chcode = 0; $chcode < 0x200; $chcode++) {
	for(my $x = 0; $x < 16; $x++) {
		$outdata .= $outdatas{$chcode}[$x];
	}
}

write_file($outfile, \$outdata);

print "\nComplete.";
getc(STDIN);



sub read_file {
	my($filename,$rdata)=@_;

	my $fh;
	my $buf="";;
	my $data="";

	if (!open($fh, $filename)) {
		print "$filename: file not open.\n";
		return 1;
	}
	binmode($fh);
	while(read($fh, $buf, 1024)) {
		$data .= $buf;
	}
	close($fh);
	$$rdata .= $data;
	return 0;
}

sub write_file {
	my($filename,$rdata)=@_;

	my $fh;
	my $buf="";;
	my $data="";

	if (!open($fh, "> ".$filename)) {
		print "$filename: file not open.\n";
		return 1;
	}
	binmode($fh);
	print {$fh} $$rdata;
	close($fh);
	return 0;
}

sub check_bmp_header {
	my($rdata)=@_;

	# file header 14 bytes
	if (substr($$rdata,0,2) ne "BM") {
		print "This is not BMP format.\n";
		return 1;
	}
	$offset = unpack("V",substr($$rdata,0x0a,4));
	print "offset:".$offset."\n";

	# info header 
	my $infosize = unpack("V",substr($$rdata,0x0e,4));
	if ($infosize != 40) {
		print "Windows BMP format only.\n";
		return 1;
	}
	my $width = unpack("V",substr($$rdata,0x12,4));
	if ($width != 256) {
		print "width must be 256 pixel.\n";
		return 1;
	}
	my $height = unpack("V",substr($$rdata,0x16,4));
	if ($height != 256) {
		print "height must be 256 pixel.\n";
		return 1;
	}
	my $bpp = unpack("v",substr($$rdata,0x1c,2));
	if ($bpp != 1) {
		print "Supported data is only 1bit per pixel(B/W data).\n";
		return 1;
	}
	my $compress = unpack("V",substr($$rdata,0x1e,4));
	if ($compress != 0) {
		print "Supported data is only no compression.\n";
		return 1;
	}

	return 0;
}
