<?php
require_once("sb.php");

if ( $argc < 2 )
{
	exit("usage: {$argv[0]} <classN> [proxy_id]\n");
}

chdir(dirname($argv[0]));

$classN = intval($argv[1]);
if ( $classN < 0 ) $classN = 0;
if ( $classN > 255 ) $classN = 255;
if ( $argc > 2 )
{
	$proxy_id = trim($argv[2]);
	if ( isset($arrProxy[$proxy_id]) )
		$GLOBALS['proxy'] = $arrProxy[$proxy_id];
}

$cfg = parse_ini_file("sb.ini", true);

// use n of m to calculate try_array
$arrTry = array();
for($i=0;$i<256;++$i)
{
	$arrTry[$i] = 0;
}

$folder_tmp = "B_{$classN}";
while( count($arrTry) > 0 )
{
	foreach($arrTry as $i => $try)
	{
		$ip = $i . "." . $classN . ".1.1";
		$ret = query_sbrs_ip_16($ip, $folder_tmp, false, intval($cfg['*']['Debug']));
		if ( $ret == 'OK' )
			unset($arrTry[$i]);
		else
			++$arrTry[$i];
	}
	
	print_r($arrTry);
}

$folder_out = "csv_" . date("Y-m-d");
system("mkdir $folder_out"); //mkdir($folder_out, 0777, true);
if ( is_dir($folder_tmp) && is_dir($folder_out) )
{
	$d = @dir($folder_tmp);
	if ( !$d )
		exit("open '$folder_tmp' error!\n");	
	
	while (false !== ($entry = $d->read())) {
		if ( $entry == '.' || $entry == '..' || is_dir($d->path . '/' . $entry) ) continue;
		$dot = strrpos($entry, ".");
		if ( $dot === false || strtolower(substr($entry, $dot)) != ".csv" ) continue;
		echo "moving '$entry'...\n";
		rename("{$folder_tmp}/{$entry}", "{$folder_out}/{$entry}");
	}
	$d->close();
}
?>

