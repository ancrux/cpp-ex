<?php
require_once("sb.php");

if ( $argc < 2 )
{
	exit("usage: {$argv[0]} [a|b] <total>\n");
}

chdir(dirname($argv[0]));

$type = (strtolower($argv[1]) == 'a')?'a':'b';
$total = intval($argv[2]);
if ( $x_total < 1 ) $x_total = 1;

$content = "\r\n[*]\r\n";
for($n=1; $n <= $total; ++$n)
{
	$content .= "sbxb{$n}=\"./svp -c \"../php5/php ../scripts/sbxb.php {$n} {$total}\" -v \"0.1b\" -b 1 -s 9\"\r\n";
}

file_put_contents("sbx{$type}_{$total}.ini", $content);
?>