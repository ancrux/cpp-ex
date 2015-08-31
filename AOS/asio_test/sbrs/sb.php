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

// Web Proxy Configuration

$arrProxy = array();

/*
$arrServer = array();
$arrServer['id'] = 'senderbase.org';
$arrServer['prefix'] = '';
$arrServer['suffix'] = '';
$arrServer['url_encode'] = false;
$arrServer['url_base64'] = false;
$arrServer['url_cookie'] = false;
$arrProxy["{$arrServer['id']}"] = $arrServer;
//*/

// Good speed
$arrServer = array();
$arrServer['id'] = 'freeproxyserver.ca';
$arrServer['prefix'] = "http://{$arrServer['id']}/index.php?btxmnercdeqt=";
$arrServer['suffix'] = "&hl=3e5";
$arrServer['url_encode'] = false;
$arrServer['url_base64'] = false;
$arrServer['url_cookie'] = false;
$arrProxy["{$arrServer['id']}"] = $arrServer;

// Good speed
$arrServer = array();
$arrServer['id'] = 'privacywant.info';
$arrServer['prefix'] = "http://{$arrServer['id']}/index.php?q=";
$arrServer['suffix'] = "&hl=3e7";
$arrServer['url_encode'] = false;
$arrServer['url_base64'] = false;
$arrServer['url_cookie'] = false;
$arrProxy["{$arrServer['id']}"] = $arrServer;

$arrServer = array();
$arrServer['id'] = 'ninjavanish.net';
$arrServer['prefix'] = "http://www.ninjavanish.net/browse.php?u=";
$arrServer['suffix'] = "&b=12";
$arrServer['url_encode'] = true;
$arrServer['url_base64'] = false;
$arrServer['url_cookie'] = true;
$arrProxy["{$arrServer['id']}"] = $arrServer;

$arrServer = array();
$arrServer['id'] = 'solidproxy.com';
$arrServer['prefix'] = "http://solidproxy.com/browse.php?u=";
$arrServer['suffix'] = "&b=12";
$arrServer['url_encode'] = true;
$arrServer['url_base64'] = false;
$arrServer['url_cookie'] = true;
$arrProxy["{$arrServer['id']}"] = $arrServer;

$arrServer = array();
$arrServer['id'] = 'netproxy.info';
$arrServer['prefix'] = "http://netproxy.info/browse.php?u=";
$arrServer['suffix'] = "&b=12";
$arrServer['url_encode'] = true;
$arrServer['url_base64'] = false;
$arrServer['url_cookie'] = true;
$arrProxy["{$arrServer['id']}"] = $arrServer;

$arrServer = array();
$arrServer['id'] = 'solarproxy.com';
$arrServer['prefix'] = "http://solarproxy.com/browse.php?u=";
$arrServer['suffix'] = "&b=12";
$arrServer['url_encode'] = true;
$arrServer['url_base64'] = false;
$arrServer['url_cookie'] = true;
$arrProxy["{$arrServer['id']}"] = $arrServer;

$arrServer = array();
$arrServer['id'] = 'serverunblock.info';
$arrServer['prefix'] = "http://{$arrServer['id']}/browse.php?u=";
$arrServer['suffix'] = "&b=12";
$arrServer['url_encode'] = true;
$arrServer['url_base64'] = false;
$arrServer['url_cookie'] = true;
$arrProxy["{$arrServer['id']}"] = $arrServer;

// Good speed, but will ban ip
$arrServer = array();
$arrServer['id'] = 'orkutpass.com';
$arrServer['prefix'] = "http://{$arrServer['id']}/index.php?URL=";
$arrServer['suffix'] = "&b=12";
$arrServer['url_encode'] = true;
$arrServer['url_base64'] = false;
$arrServer['url_cookie'] = false;
$arrProxy["{$arrServer['id']}"] = $arrServer;

// Good speed, but will ban ip
$arrServer = array();
$arrServer['id'] = 'jumboproxy.com';
$arrServer['prefix'] = "http://{$arrServer['id']}/index.php?URL=";
$arrServer['suffix'] = "&b=12";
$arrServer['url_encode'] = true;
$arrServer['url_base64'] = false;
$arrServer['url_cookie'] = false;
$arrProxy["{$arrServer['id']}"] = $arrServer;

$arrServer = array();
$arrServer['id'] = 'googal.info';
$arrServer['prefix'] = "http://{$arrServer['id']}/index.php?q=";
$arrServer['suffix'] = "";
$arrServer['url_encode'] = false;
$arrServer['url_base64'] = true;
$arrServer['url_cookie'] = false;
$arrProxy["{$arrServer['id']}"] = $arrServer;

$arrServer = array();
$arrServer['id'] = 'aquaproxy.net';
$arrServer['prefix'] = "http://{$arrServer['id']}/index.php?q=";
$arrServer['suffix'] = "";
$arrServer['url_encode'] = false;
$arrServer['url_base64'] = true;
$arrServer['url_cookie'] = false;
$arrProxy["{$arrServer['id']}"] = $arrServer;

$GLOBALS['proxy'] = $arrProxy['freeproxyserver.ca'];

function query_sbrs_ip_16($ip, $folder, $format_folder_with_date = false, $debug = 0)
{
	$end_mark = "No address list shown since no email was detected";
	$re_range = '/Showing (\d+) - (\d+) out of (\d+)/'; //Showing 1 - 50 out of 83
	
	if ( $format_folder_with_date )
		$folder = date($folder);
	
	system("mkdir $folder"); //mkdir($folder, 0777, true);
	if ( !file_exists($folder) )
	{
		exit("'$folder' isn't existed!\n");
	}
	
	$path = '';
	
	$row = 50;
	$start = 0;
	$total = 0;
	
	//$ip = "9.17.1.1"; //@
	
	$proxy = $GLOBALS['proxy'];
	if ( $proxy['url_cookie'] == true )
	{
		query_page($ip, $start, $row, $proxy);
	}
	
	$arrIP = explode(".", $ip);
	$ipAB = "{$arrIP[0]}.{$arrIP[1]}";
	
	$log_file = "{$folder}/{$folder}.log";
	$csv_file = "{$folder}/{$ipAB}.csv";
	$path = "{$folder}/{$ipAB}_{$total}_{$start}.htm";
	//$query = build_sbrs_query($ip, $start, $row);
	$page = query_page($ip, $start, $row, $proxy);
	
	if ( preg_match($re_range, $page, $matches) > 0 )
		$total = $matches[3];
	echo "total: $total\n";
	
	$path = "{$folder}/{$ipAB}_{$total}_{$start}.htm";
	echo "path=$path\r\n";
	$n_record = html_to_csv($page, $csv_file);
	$error = '';
	if ( !$n_record )
	{
		if ( $debug == 1 ) file_put_contents($path, $page);
		$error = "\tERROR!";
	}
	$log_text = "{$path}\t({$proxy['id']}){$error}\r\n";
	file_put_contents($log_file, $log_text, FILE_APPEND);
	$start += $row;
	
	while( $start <= $total )
	{
		//sleep(1);
		$path = "{$folder}/{$ipAB}_{$total}_{$start}.htm";
		//$query = build_sbrs_query($ip, $start, $row);
		$page = query_page($ip, $start, $row, $proxy);
		echo "path=$path\r\n";
		$n_record = html_to_csv($page, $csv_file);
		$error = '';
		if ( !$n_record )
		{
			if ( $debug == 1 ) file_put_contents($path, $page);
			$error = "\tERROR!";
		}
		$log_text = "{$path}\t({$proxy['id']}){$error}\r\n";
		file_put_contents($log_file, $log_text, FILE_APPEND);
		$start += $row;
	}
	//sleep(1);
	
	if ( $page != '' ) return 'OK';
}

function build_query($ip, $start, $row, $proxy)
{
	$query = '';
	if ( $proxy['url_encode'] == true )
	{
		$query = "http%3A%2F%2Fwww.senderbase.org%2Fsenderbase_queries%2Fdetailip%3Fsearch_string%3D" . $ip . "%2F16";
		if ( $start > 0 )
			$query = "http%3A%2F%2Fwww.senderbase.org%2Fsenderbase_queries%2Fdetailip%3Fsearch_string%3D" . $ip . "%2F16%3Bamp%3Bmax_rows%3D" . $row . "%3Bamp%3Bdnext_set%3D" . $start . "%3Bamp%3Btddorder%3Dip%2Basc%3Bamp%3Bwhich_others%3D%2F16";
	}
	else
	{
		$query = "http://www.senderbase.org/senderbase_queries/detailip?search_string=" . $ip . "/16";
		if ( $start > 0 )
			$query = "http://www.senderbase.org/senderbase_queries/detailip?search_string=" . $ip . "/16;amp;max_rows=" . $row . ";amp;dnext_set=" . $start . ";amp;tddorder=ip+asc;amp;which_others=/16";
	}
	if ( $proxy['url_base64'] == true )
	{
		$query = base64_encode($query);
	}
	$query = $proxy['prefix'] . $query . $proxy['suffix'];
	
	return $query;
}

function query_page($ip, $start = 0, $row = 50, $proxy)
{
	return query_page3($ip, $start, $row, $proxy);
}

function query_page1($ip, $start, $row, $proxy)
{

	/*
	$query = "http://www.senderbase.org/senderbase_queries/detailip?search_string=" . $ip . "/16";
	if ( $start > 0 )
		$query = "http://www.senderbase.org/senderbase_queries/detailip?search_string=" . $ip . "/16;amp;max_rows=" . $row . ";amp;dnext_set=" . $start . ";amp;tddorder=ip+asc;amp;which_others=/16";
	//*/
	
	$query = build_query($ip, $start, $row, $proxy);
	
	$page = '';
	$fh = fopen($query, "rb");
	if ( !$fh )
		return $page;
		
	while( !feof($fh) )
		$page .= fread($fh, 8192);
	fclose($fh);
	
	return $page;
}

function query_page2($ip, $start, $row, $proxy)
{
	/*
	$query = "http://www.freeproxyserver.ca/index.php?btxmnercdeqt=http://www.senderbase.org/senderbase_queries/detailip?search_string=" . $ip . "/16&hl=3e5";
	if ( $start > 0 )
		$query = "http://www.freeproxyserver.ca/index.php?btxmnercdeqt=http://www.senderbase.org/senderbase_queries/detailip?search_string=" . $ip . "/16;amp;max_rows=" . $row . ";amp;dnext_set=" . $start . ";amp;tddorder=ip+asc;amp;which_others=/16&hl=3e5";
	//*/
	
	$query = build_query($ip, $start, $row, $proxy);
	
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
		
		//echo "host=$host, path=$path\r\n"; //@
		
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
		$req .= "ProxyConnection: keep-alive\r\n";
		//$req .= "ProxyConnection: close\r\n";
		//$req .= "Connection: Close\r\n";
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

function query_page3($ip, $start, $row, $proxy)
{
	/*
	$query = "http://www.freeproxyserver.ca/index.php?btxmnercdeqt=http://www.senderbase.org/senderbase_queries/detailip?search_string=" . $ip . "/16&hl=3e5";
	if ( $start > 0 )
		$query = "http://www.freeproxyserver.ca/index.php?btxmnercdeqt=http://www.senderbase.org/senderbase_queries/detailip?search_string=" . $ip . "/16;amp;max_rows=" . $row . ";amp;dnext_set=" . $start . ";amp;tddorder=ip+asc;amp;which_others=/16&hl=3e5";	
	//*/
	
	$query = build_query($ip, $start, $row, $proxy);
	
	$page = '';
	
	$headers = array();
	$headers[] = "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 6.1; zh-TW; rv:1.9.2) Gecko/20100115 Firefox/3.6";
	$headers[] = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";
	$headers[] = "Accept-Language: zh-tw,en-us;q=0.7,en;q=0.3";
	$headers[] = "Accept-Charset: UTF-8,*";
	$headers[] = "Connection: keep-alive";
	$cookie_file = 'cookie.txt';
	
	do
	{
		$ch = curl_init();
		
		curl_setopt($ch, CURLOPT_URL, $query);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_HEADER, true);
		curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
		curl_setopt($ch, CURLOPT_AUTOREFERER, true);
		curl_setopt($ch, CURLOPT_COOKIEFILE, $cookie_file);
		curl_setopt($ch, CURLOPT_COOKIEJAR, $cookie_file);
		curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
		curl_setopt($ch, CURLOPT_ENCODING, 'gzip,deflate'); // gzip encoding
		
		$page = curl_exec($ch);
		
		curl_close($ch);
	}
	while(0);
	
	return $page;
}

function html_to_csv(&$contents, $csv_file)
{
	$n_record = 0;
	
	//$mark_record_start = "<a href=\"(.+)detailip(.+)search_string";
	
	$lines = explode("\n", $contents);
	$n_line = count($lines);
	for($i=0;$i<$n_line;++$i)
	{
		//if ( preg_match('/search_string%3D(\d+\.\d+\.\d+\.\d+)\D+/i', $lines[$i], $matches) > 0 )
		if ( preg_match('/<a href="http:.+php.+>.*\b(\d+\.\d+\.\d+\.\d+)\b/i', $lines[$i], $matches) > 0 )
		{
			$ip = $matches[1];
			$hostname = ( preg_match('/([\w\.]+)<\/div>/i', $lines[$i+3], $matches) > 0 )?$matches[1]:'';
			$dns_match = ( preg_match('/([\w\.]+)<\/div>/i', $lines[$i+5], $matches) > 0 )?$matches[1]:'';
			$daily = ( preg_match('/([\w\.]+)<\/div>/i', $lines[$i+6], $matches) > 0 )?$matches[1]:'';
			$monthly = ( preg_match('/([\w\.]+)<\/div>/i', $lines[$i+7], $matches) > 0 )?$matches[1]:'';
			$dnsbl = ( preg_match('/([\w\.]+)<\/div>/i', $lines[$i+8], $matches) > 0 )?$matches[1]:'';
			$sbrs = ( preg_match('/([\w\.]+)<\/div>/i', $lines[$i+9], $matches) > 0 )?$matches[1]:'';
			
			
			$record = "{$ip},{$hostname},{$dns_match},{$daily},{$monthly},{$dnsbl},{$sbrs}\n";
			++$n_record;
			//echo $record;
			if ( trim($sbrs) != '' )
				file_put_contents($csv_file, $record, FILE_APPEND);
			
			$i+=10;
		}
	}
	
	return $n_record;
}

function array_key_set_range(&$arr, $range, $mode = '+')
{
	$keys = explode('-', trim($range));
	$n_key = count($keys);
	if ( $n_key == 1 )
	{
		do
		{
			$key = intval($keys[0]);
			if ( $key == 0 ) break;
			
			if ( $mode == '+' )
				$arr[$key] = $key;
			else
				unset($arr[$key]);
		}
		while(0);
	}
	else if ( $n_key == 2 )
	{
		do
		{
			$key1 = intval($keys[0]);
			$key2 = intval($keys[1]);
			if ( $key1 == 0 || $key2 == 0 ) break;
			
			if ( $mode == '+' )
				for($k=$key1; $k<=$key2; ++$k)
					$arr[$k] = $k;
			else
				for($k=$key1; $k<=$key2; ++$k)
					unset($arr[$k]);
		}
		while(0);
	}
}

// deprecated!!!
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

set_time_limit(0);
?>