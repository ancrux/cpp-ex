<?php
$arr = array();

$fh = fopen("c:/poor_ip.txt", "rb");
while( ($line = fgets($fh)) !== false )
{
	$t = explode("\t", $line);
	$score = floatval($t[1]);
	if ( $score > 0 )
	{
		$score = $score * 10 % 10;
		++$arr["0.$score"];
	}
	else
	{
		++$arr["-1.0"];
	}
}

ksort($arr);
print_r($arr);
?>