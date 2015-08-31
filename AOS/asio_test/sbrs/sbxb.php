<?php
require_once("sb.php");

if ( $argc < 3 )
{
	exit("usage: {$argv[0]} <x> <x_total> [proxy_id]\n");
}

chdir(dirname($argv[0]));

$x = intval($argv[1]);
$x_total = intval($argv[2]);
if ( $x_total < 1 || $x_total > 256 ) $x_total = 1;
if ( $x < 1 || $x > $x_total ) $x = 1;
if ( $argc > 3 )
{
	$proxy_id = trim($argv[3]);
	if ( isset($arrProxy[$proxy_id]) )
		$GLOBALS['proxy'] = $arrProxy[$proxy_id];
}

$cfg = parse_ini_file("sb.ini", true);

$arrA = array();
do
{
	if ( trim($cfg['A']['All']) != '' )
	{
		$allA = explode(',', $cfg['A']['All']);
		foreach($allA as $key => $val)
			array_key_set_range($arrA, $val, '+');
		
		$exA = explode(',', $cfg['A']['Exclude']);
		foreach($exA as $key => $val)
			array_key_set_range($arrA, $val, '-');
			
		break;
	}
	
	$tmpA = array();
	for($i=0;$i<256;++$i)
	{
		if ( $i == 10 || $i == 127 || ($i >= 224 && $i < 240) ) continue;
		$tmpA[$i] = $i;
	}
	$exA = explode(',', $cfg['A']['Exclude']);
	foreach($exA as $key => $val)
		array_key_set_range($tmpA, $val, '-');
	$hiA = explode(',', $cfg['A']['HiPriority']);
	foreach($hiA as $key => $val)
		array_key_set_range($tmpA, $val, '-');
	$loA = explode(',', $cfg['A']['LoPriority']);
	krsort($loA);
	foreach($loA as $key => $val)
		array_key_set_range($tmpA, $val, '-');
	$orA = $cfg['A']['Order'];
	if ( $orA == 'DESC' )
		krsort($tmpA);
	
	foreach($hiA as $key => $val)
	{
		if ( $val === '' ) continue;
		array_key_set_range($arrA, $val, '+'); //$arrA[] = $val;
	}
	foreach($tmpA as $key => $val)
	{
		if ( $val === '' ) continue;
		array_key_set_range($arrA, $val, '+'); //$arrA[] = $val;
	}
	foreach($loA as $key => $val)
	{
		if ( $val === '' ) continue;
		array_key_set_range($arrA, $val, '+'); //$arrA[] = $val;
	}

}
while(0);

//print_r($arrA);
//exit(0);

$arrB = array();
do
{
	if ( trim($cfg['B']['All']) != '' )
	{
		$allB = explode(',', $cfg['B']['All']);
		foreach($allB as $key => $val)
			array_key_set_range($arrB, $val, '+');
			
		$exB = explode(',', $cfg['B']['Exclude']);
		foreach($exB as $key => $val)
			array_key_set_range($arrB, $val, '-');
			
		break;
	}
	
	$tmpB = array();
	for($i=0;$i<256;++$i)
	{
		$tmpB[$i] = $i;
	}
	
	$exB = explode(',', $cfg['B']['Exclude']);
	foreach($exB as $key => $val)
		array_key_set_range($tmpB, $val, '-');
	$hiB = explode(',', $cfg['B']['HiPriority']);
	foreach($hiB as $key => $val)
		array_key_set_range($tmpB, $val, '-');
	$loB = explode(',', $cfg['B']['LoPriority']);
	krsort($loB);
	foreach($loB as $key => $val)
		array_key_set_range($tmpB, $val, '-');
	$orB = $cfg['B']['Order'];
	if ( $orB == 'DESC' )
		krsort($tmpB);
	
	$arrB = array();
	foreach($hiB as $key => $val)
	{
		if ( $val === '' ) continue;
		array_key_set_range($arrB, $val, '+'); //$arrB[] = $val;
	}
	foreach($tmpB as $key => $val)
	{
		if ( $val === '' ) continue;
		array_key_set_range($arrB, $val, '+'); //$arrB[] = $val;
	}
	foreach($loB as $key => $val)
	{
		if ( $val === '' ) continue;
		array_key_set_range($arrB, $val, '+'); //$arrB[] = $val;
	}

}
while(0);

//print_r($arrB);
//exit(0);

$arrTry = array();
foreach($arrA as $ia => $va)
{
	foreach($arrB as $ib => $vb)
	{
		if ( ($vb % $x_total)+1 == $x )
		{
			$arrTry["{$va}.{$vb}"] = 0;
		}
	}
}
$try_total = count($arrTry);
//print_r($arrTry);
print "x/total=$x/$x_total, try_total=$try_total\n";
//exit(0); //@

$folder_tmp = "NB_{$x_total}_{$x}";
$is_folder_date_format = false;
if ( trim($cfg['*']['FolderDateFormat']) != '' )
{
	$folder_tmp = trim($cfg['*']['FolderDateFormat']);
	$is_folder_date_format = true;
}
while( count($arrTry) > 0 )
{
	foreach($arrTry as $ipAB => $n_try)
	{
		$ip = $ipAB . ".1.1";
		$ret = query_sbrs_ip_16($ip, $folder_tmp, $is_folder_date_format, intval($cfg['*']['Debug']));
		unset($arrTry[$ipAB]);
		/*
		if ( $ret == 'OK' )
			unset($arrTry[$ipAB]);
		else
			++$arrTry[$ipAB];
		//*/
	}
	print_r($arrTry);
}

if ( !$is_folder_date_format )
{
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
}
?>

