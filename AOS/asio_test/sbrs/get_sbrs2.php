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
	exit("usage: {$argv[0]} <classA_ip> <output_folder>\n");
}

$classA = intval($argv[1]);
if ( $classA < 0 ) $classA = 0;
if ( $classA > 255 ) $classA = 255;
$folder = "$classA";
if ( isset($argv[2]) ) $folder = $argv[2];
system("mkdir $folder"); //mkdir($folder, 0777, true);

if ( !file_exists($folder) )
{
	exit("'$folder' isn't existed!\n");
}

$arrTry = array();
for($i=0;$i<256;++$i)
{
	$arrTry[$i] = 0;
}


while( count($arrTry) > 0 )
{
	foreach($arrTry as $i => $try)
	{
		$ip = $classA . "." . $i . ".1.1";
		$ret = query_sbrs_ip_16($ip, $folder);
		if ( $ret == 'OK' )
			unset($arrTry[$i]);
		else
			++$arrTry[$i];
	}
	
	print_r($arrTry);
}


function query_sbrs_ip_16($ip, $folder)
{
	$end_mark = "No address list shown since no email was detected";
	$re_range = '/Showing (\d+) - (\d+) out of (\d+)/'; //Showing 1 - 50 out of 83
	
	$path = '';
	
	$row = 50;
	$start = 0;
	$total = 0;
	
	//$ip = "9.17.1.1"; //@
	
	$path = "{$folder}/{$ip}_{$start}_{$total}.htm";
	$query = build_sbrs_query($ip, $start, $row);
	$page = query_page2($query);
	
	if ( preg_match($re_range, $page, $matches) > 0 )
		$total = $matches[3];
	echo "total: $total\n";
	
	$path = "{$folder}/{$ip}_{$start}_{$total}.htm";
	echo "path=$path\n";
	file_put_contents($path, $page);
	$start += $row;
	
	while( $start <= $total )
	{
		//sleep(1);
		$path = "{$folder}/{$ip}_{$start}_{$total}.htm";
		$query = build_sbrs_query($ip, $start, $row);
		$page = query_page2($query);
		echo "path=$path\n";
		file_put_contents($path, $page);
		$start += $row;
	}
	//sleep(1);
	
	if ( $page != '' ) return 'OK';
}

/*
if ( strpos($page, $end_mark) !== false )
	echo "end_mark found!\n";
else
	echo "more...\n";
//*/

function query_page($query)
{
	$page = '';
	$fh = fopen($query, "rb");
	if ( !$fh )
		return $page;
		
	while( !feof($fh) )
		$page .= fread($fh, 8192);
	fclose($fh);
	
	return $page;
}

function build_sbrs_query($ip, $start = 0, $row = 50)
{
	if ($start==0)
	{
		$query = "http://www.freeproxyserver.ca/index.php?btxmnercdeqt=http://www.senderbase.org/senderbase_queries/detailip?search_string=" . $ip . "/16&hl=3e5";
	}
	else
	{
		$query = "http://www.freeproxyserver.ca/index.php?btxmnercdeqt=http://www.senderbase.org/senderbase_queries/detailip?search_string=" . $ip . "/16;amp;max_rows=" . $row . ";amp;dnext_set=" . $start . ";amp;tddorder=ip+asc;amp;which_others=/16&hl=3e5";
	}
	
	return $query;
}

function query_page2($query)
{
	$page = '';
	
	do
	{
		$pos1 = strpos($query, "://");
		if ( $pos1 === false )
		{
			break;
		}
		$pos1 += 3;
		$pos2 = strpos($query, "/", $pos1);
		if ( $pos2 === false )
		{
			break;
		}
		
		$host = substr($query, $pos1, $pos2-$pos1);
		$path = substr($query, $pos2);
		
		echo "host=$host, path=$path\r\n";
		
		$fh = fsockopen($host, 80, $errno, $errstr, 30);
		if ( !$fh ) break;
		
		$req = '';
		//$req .= "GET /index.php?btxmnercdeqt=http://www.senderbase.org/senderbase_queries/detailip%3Fsearch_string%3D188.58.1.1%252F16&hl=3e5 HTTP/1.1\r\n";
		$req .= "GET $query HTTP/1.1\r\n";
		$req .= "Host: $host\r\n";
		$req .= "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 6.1; zh-TW; rv:1.9.2) Gecko/20100115 Firefox/3.6\r\n";
		$req .= "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
		$req .= "Accept-Language: zh-tw,en-us;q=0.7,en;q=0.3\r\n";
		$req .= "Accept-Charset: UTF-8,*\r\n";
		//$req .= "Connection: Close\r\n";
		$req .= "ProxyConnection: keep-alive\r\n";
		$req .= "Cache-Control: max-age=0\r\n";
		
		$req .= "\r\n";
		
		fwrite($fh, $req);
		
		while( !feof($fh) )
			$page .= fread($fh, 4096);
		fclose($fh);
	}
	while(0);
	
	return $page;
}
?>

