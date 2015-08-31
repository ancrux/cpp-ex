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
    echo "Usage: $argc[0] <cbl_file>\n";
    exit(0);
}

$bl_file = $argv[1];
$path_parts = explode('.', basename($bl_file));
$ips_file = '';
for($i=0; $i < count($path_parts)-1; ++$i)
{
    $ips_file .= $path_parts[$i];
} 
$ips_file .= ".ips";

$fh = fopen($bl_file, "rb");
if ( !$fh )
{
    echo "Cannot open '$cbl_file'!\n";
    exit(0);
}

$output = '';
file_put_contents($ips_file, $output);

$count = -1;
while( !feof($fh) )
{
    $line = fgets($fh);
	if ( $count < 0 )
	{
		if ( strncmp($line, "127.0.0.", strlen("127.0.0.")) == 0 )
			++$count;
		continue;
	}
	
	$ip = trim($line); if ( $ip == '' ) continue; //echo "$ip\n";
	$output .= ip_to_5byte($ip, 0x00); // always Poor IP
	++$count;
	
	if ( strlen($output) >= 200 * 1024 )
	{
	    file_put_contents($ips_file, $output, FILE_APPEND);
	    echo '.';
	    $output = '';
	    
	}
}
file_put_contents($ips_file, $output, FILE_APPEND);
echo '.';
fclose($fh);

echo "\n$ips_file created! (count:$count)\n";

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