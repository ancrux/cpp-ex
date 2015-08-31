<?php
set_time_limit(0);

if (!function_exists('file_put_contents'))
{
	function file_put_contents($filename, &$data, $file_append = false)
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

if ( $argc < 2 )
{
	exit("Usage: {$argv[1]} <input_folder>");
}

$in_folder = $argv[1];

///*
$folder = $in_folder;
$d = @dir($folder);
if ( !$d )
	exit("open '$folder' error!\n");
	
//echo "Handle: " . $d->handle . "\n";
//echo "Path: " . $d->path . "\n";

$all_file = "c:/all_ipv4.good";
$all_total = 0;
$all_good = 0;
$all_poor = 0;
$all_neutral = 0;

$no_content = '';
file_put_contents($all_file, $no_content);

while (false !== ($entry = $d->read())) {
	if ( $entry == '.' || $entry == '..' || is_dir($d->path . '/' . $entry) ) continue;
	//$prefix = "spm_2010-03-1";
	//if ( strncmp($entry, $prefix, strlen($prefix)) != 0 ) continue;
	echo "reading '$entry'...\n";
	$path = $d->path . '/' . $entry;		
	process_file($path);
}
$d->close();
//*/

echo "ChangeTo Total:good/neutral/poor/total={$all_good}/{$all_neutral}/{$all_poor}/{$all_total}\n";
$all_diff = $all_total - $all_good - $all_neutral - $all_poor;
echo "ChangeTo Total:check sum: $all_diff\n";

function process_file($path)
{
	global $all_file, $all_total, $all_good, $all_poor, $all_neutral;
	
	$fh = fopen($path, "rb");
	if ( !$fh ) return;
	
	$fp = fsockopen("localhost", 56789);
	
	$output = '';
	
	$line = fgets($fh); // skip first line (headers)
	$n_total = 0;
	$n_good = 0;
	$n_poor = 0;
	$n_neutral = 0;
	
	$g_total = 0;
	$g_good = 0;
	$g_neutral = 0;
	$g_poor = 0;
	$g_na = 0;
	while( ($line = fgets($fh)) !== false )
	{
		++$n_total;
		$t = explode(",", $line);
		
		$ip = trim($t[0]);
		$sbrs = trim($t[6]);
		if ( strcasecmp($sbrs, "Good") == 0 )
		{
			//$output .= ip_to_5byte($ip, 0xFF);
			++$n_good;
			
			++$g_total; //if ( $g_total > 10 ) exit(0);
			$score = ask_srl($fp, $ip);
			if ( $score >= 0.95 )
				++$g_good;
			else if ( $score >= 0.5 )
				++$g_neutral;
			else if ( $score >= 0.0 )
				++$g_poor;
			else
				++$g_na;
			
			$output .= "$ip\t$score\n";
		}
		else if ( strcasecmp($sbrs, "Poor") == 0 )
		{
			//$output .= ip_to_5byte($ip, 0x00);
			++$n_poor;
			//$output .= "$ip\n";
		}
		else if ( strcasecmp($sbrs, "Neutral") == 0 )
		{
			//$output .= ip_to_5byte($ip, 0x80);
			++$n_neutral;
			//$score = ask_srl($fp, $ip);
			//$output .= "$ip\t$score\n";
		}
		else
		{
			//echo "$ip:$sbrs\n";
		}
	}
	
	fclose($fp);
	
	fclose($fh);
	
	/*
	file_put_contents("c:/all_ipv4.dat", $output, true);
	echo "good/neutral/poor/total={$n_good}/{$n_neutral}/{$n_poor}/{$n_total}\n";
	$n_diff = $n_total - $n_good - $n_neutral - $n_poor;
	echo "check sum: $n_diff\n";
	//*/
	
	file_put_contents($all_file, $output, true);
	echo "ChangeTo:good/neutral/poor/total={$g_good}/{$g_neutral}/{$g_poor}/{$g_total}\n";
	$g_diff = $g_total - $g_good - $g_neutral - $g_poor;
	echo "check sum: $g_diff, N/A: $g_na\n";

	$all_total += $g_total;
	$all_good += $g_good;
	$all_neutral += $g_neutral;
	$all_poor += $g_poor;
}

function ask_srl($fp, $ip)
{
	fwrite($fp, "SBRS $ip\r\n\r\n");
	
	$line = fgets($fp); fgets($fp);
	//echo $line;
	$score = floatval($line);
	
	return $score;
}

function ip_to_5byte($ip, $flag)
{
	$octet = '';
	$arr = explode('.', $ip);
	for($i=0;$i<4;++$i)
		$octet .= chr(intval($arr[$i]));
	$octet .= chr($flag);
	
	return $octet;
}
?>