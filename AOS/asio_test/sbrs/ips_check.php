<?php
if (!function_exists('file_put_contents'))
{
	
define(FILE_APPEND, 1);
function file_put_contents($filename, &$data, $file_append = 0)
{
	$fp = fopen($filename, (!$file_append ? 'w+' : 'a+'));
	if(!$fp)
	{
		trigger_error('file_put_contents cannot write in file.', E_USER_ERROR);
		return;
	}
	fputs($fp, $data);
	fclose($fp);
}

}

set_time_limit(0);

if ( $argc < 2 )
{
	echo "Usage: $argv[0] ips_file\n";
	exit(0);
}

$ips_file = $argv[1];


$fp = fopen($ips_file, "rb");
if ( !$fp )
{
	echo "open file failed: $ips_file\n";
	exit(0);
}

$n_total = 0;
$n_poor = 0;
$n_neutral = 0;
$n_good = 0;

$n_error = 0;

while( !feof($fp) )
{
	$buf = fread($fp, 1000 * 1000);
	$len = strlen($buf);
	
	for($i=0; $i<$len; $i+=5)
	{
		$ipA = ord($buf[$i]);
		$ipB = ord($buf[$i+1]);
		$ipC = ord($buf[$i+2]);
		$ipD = ord($buf[$i+3]);
		$score = ord($buf[$i+4]);
		
		$ip24_new = ($ipA * 256 * 256 * 256) + ($ipB * 256 * 256) + ($ipC * 256);
		$ip32 = $ip24_new + $ipD;
		++$n_total;
		//echo "$ipA.$ipB.$ipC.$ipD=$score\n";
		
		if ( $score == 0 )
			++$n_poor;
		else if ( $score == 128 )
		{
			++$n_neutral;
			//echo "$ipA.$ipB.$ipC.$ipD=$score\n";
		}
		else if ( $score == 255 )
			++$n_good;
		else
		{
		    ++$n_error;
		    //echo "$ipA.$ipB.$ipC.$ipD=$score\n";
		}
	}
	
	echo ".";
	//break;
}

echo "\n";
echo "\nError/Total:$n_error/$n_total\n";
?>