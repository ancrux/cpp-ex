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

$count24 = array();

$fp = fopen($ips_file, "rb");
if ( !$fp )
{
	echo "open file failed: $ips_file\n";
	exit(0);
}

$t24_count = 0;
$t32_count = 0;

while( !feof($fp) )
{
	$buf = fread($fp, 1000 * 1000);
	$len = strlen($buf);
	
	$ip24 = 0;
	$n24_good = 0;
	$n24_poor = 0;
	$n24_neutral = 0;
	$n24_total = 0;
	for($i=0; $i<$len; $i+=5)
	{
		$ipA = ord($buf[$i]);
		$ipB = ord($buf[$i+1]);
		$ipC = ord($buf[$i+2]);
		$ipD = ord($buf[$i+3]);
		$score = ord($buf[$i+4]);
		
		$ip24_new = ($ipA * 256 * 256 * 256) + ($ipB * 256 * 256) + ($ipC * 256);
		if ( $ip24_new != $ip24 )
		{
			if ( !$n24_poor && !$n24_neutral && $n24_good)
			{
				++$t24_count;
				$t32_count += $n24_good;
			//}
			
			if ( !isset($count24[$n24_good]) )
				$count24[$n24_good] = 0;
			$count24[$n24_good] += 1;
			
			$ip24A = $ip24 >> 24;
			$ip24B = ($ip24 >> 16) % 256;
			$ip24C = ($ip24 >> 8) % 256;
			//echo "$ip24A.$ip24B.$ip24C.0=$n24_good/$n24_neutral/$n24_poor/$n24_total\n";
			}
			
			$ip24 = $ip24_new;
			$n24_good = 0;
			$n24_poor = 0;
			$n24_neutral = 0;
			$n24_total = 0;
		}
		
		$ip32 = $ip24_new + $ipD;
		//echo "$ipA.$ipB.$ipC.$ipD=$score\n";
		
		if ( $score == 0 )
			++$n24_poor;
		else if ( $score == 128 )
			++$n24_neutral;
		else if ( $score == 255 )
			++$n24_good;
			
		++$n24_total;
	}
	
	echo ".";
	//break;
}

echo "\n";
ksort($count24);
print_r($count24);
echo "\nGood:t24/t32=$t24_count/$t32_count\n";
?>