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
	exit("Usage: {$argv[1]} <input_folder> <output_file>");
}

$in_folder = $argv[1];
$out_file = $argv[2];
if ( trim($out_file) == '' ) $out_file = $in_folder;
$out_file .= ".csv";

///*
$folder = $in_folder;
$d = @dir($folder);
if ( !$d )
	exit("open '$folder' error!\n");
//echo "Handle: " . $d->handle . "\n";
//echo "Path: " . $d->path . "\n";

$record = "IP,Hostname,RevDNS,Daily,Monthly,DNSBL,SBRS\n";
file_put_contents($out_file, $record);
while (false !== ($entry = $d->read())) {
	if ( $entry == '.' || $entry == '..' || is_dir($d->path . '/' . $entry) ) continue;
	//$prefix = "spm_2010-03-1";
	//if ( strncmp($entry, $prefix, strlen($prefix)) != 0 ) continue;
	echo "reading '$entry'...\n";
	$path = $d->path . '/' . $entry;
	process_file($path, $out_file);
}
$d->close();
//*/

function process_file($path, $out_file)
{
	$lines = file($path);
	$n_line = count($lines);
	
	$mark_record_start = "<a href=\"(.+)detailip(.+)search_string";
	
	for($i=0;$i<$n_line;++$i)
	{
		if ( preg_match('/search_string%3D(\d+\.\d+\.\d+\.\d+)"/', $lines[$i], $matches) > 0 )
		{
			$ip = $matches[1];
			$hostname = ( preg_match('/([\w\.]+)<\/div>/i', $lines[$i+3], $matches) > 0 )?$matches[1]:'';
			$dns_match = ( preg_match('/([\w\.]+)<\/div>/i', $lines[$i+5], $matches) > 0 )?$matches[1]:'';
			$daily = ( preg_match('/([\w\.]+)<\/div>/i', $lines[$i+6], $matches) > 0 )?$matches[1]:'';
			$monthly = ( preg_match('/([\w\.]+)<\/div>/i', $lines[$i+7], $matches) > 0 )?$matches[1]:'';
			$dnsbl = ( preg_match('/([\w\.]+)<\/div>/i', $lines[$i+8], $matches) > 0 )?$matches[1]:'';
			$sbrs = ( preg_match('/([\w\.]+)<\/div>/i', $lines[$i+9], $matches) > 0 )?$matches[1]:'';
			
			
			$record = "{$ip},{$hostname},{$dns_match},{$daily},{$monthly},{$dnsbl},{$sbrs}\n";
			//echo $record;
			file_put_contents($out_file, $record, FILE_APPEND);
			
			$i+=10;
		}
	}
}
?>