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

$g_total = 0;
$g_good = 0;
$g_poor = 0;
$g_neutral = 0;
while (false !== ($entry = $d->read())) {
	if ( $entry == '.' || $entry == '..' || is_dir($d->path . '/' . $entry) ) continue;
	//$prefix = "spm_2010-03-1";
	//if ( strncmp($entry, $prefix, strlen($prefix)) != 0 ) continue;
	echo "reading '$entry'...\n";
	$path = $d->path . '/' . $entry;		
	process_file($path, $folder);
}
$d->close();
//*/

echo "Total:good/neutral/poor/total={$g_good}/{$g_neutral}/{$g_poor}/{$g_total}\n";
$g_diff = $g_total - $g_good - $g_neutral - $g_poor;
echo "Total:check sum: $g_diff\n";

function process_file($path, $folder)
{
	global $g_total, $g_good, $g_poor, $g_neutral;
	
	$fh = fopen($path, "rb");
	if ( !$fh ) return;
	
	//$fp = fsockopen("192.168.1.233", 19990);
	
	$output = '';
	
	$line = ''; //$line = fgets($fh); // skip first line (headers)
	$n_total = 0;
	$n_good = 0;
	$n_poor = 0;
	$n_neutral = 0;
	while( ($line = fgets($fh)) !== false )
	{
		++$n_total;
		$t = explode(",", $line);
		
		$ip = trim($t[0]);
		$sbrs = trim($t[6]);
		if ( strcasecmp($sbrs, "Good") == 0 )
		{
			$output .= ip_to_5byte($ip, 0xFF);
			++$n_good;

		}
		else if ( strcasecmp($sbrs, "Poor") == 0 )
		{
			$output .= ip_to_5byte($ip, 0x00);
			++$n_poor;
		}
		else if ( strcasecmp($sbrs, "Neutral") == 0 )
		{
			$output .= ip_to_5byte($ip, 0xF0);
			++$n_neutral;
			//$score = ask_srl($fp, $ip);
			//$output .= "$ip\t$score\n";
		}
		else
		{
			//echo "$ip:$sbrs\n";
		}
	}
	
	//fclose($fp);
	
	fclose($fh);
	
	file_put_contents("{$folder}.ips", $output, FILE_APPEND); // FILE_APPEND
	
	echo "good/neutral/poor/total={$n_good}/{$n_neutral}/{$n_poor}/{$n_total}\n";
	$n_diff = $n_total - $n_good - $n_neutral - $n_poor;
	echo "check sum: $n_diff\n";
	
	$g_total += $n_total;
	$g_good += $n_good;
	$g_neutral += $n_neutral;
	$g_poor += $n_poor;
}

function ask_srl($fp, $ip)
{
	fwrite($fp, "SRL $ip\r\n\r\n");
	$line = fgets($fp);
	//echo $line;
	$score = floatval($line); //$line = fgets($fp);
	
	return $score;
}

function ip_to_5byte($ip, $flag)
{
	$octet = '';
	$arr = explode('.', $ip);
	if ( count($arr) != 4 ) return $octet;
	
	for($i=0;$i<4;++$i)
		$octet .= chr(intval($arr[$i]));
	$octet .= chr($flag);
	
	return $octet;
}
?>