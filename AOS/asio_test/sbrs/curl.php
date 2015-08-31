<?php

$headers = array();

$headers[] = "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 6.1; zh-TW; rv:1.9.2) Gecko/20100115 Firefox/3.6";
$headers[] = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";
$headers[] = "Accept-Language: zh-tw,en-us;q=0.7,en;q=0.3";
$headers[] = "Accept-Charset: UTF-8,*";
$headers[] = "Connection: keep-alive";

$ch = curl_init();

curl_setopt($ch, CURLOPT_URL, "http://solarproxy.com/");
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_HEADER, true);
curl_setopt($ch, CURLOPT_AUTOREFERER, true);
curl_setopt($ch, CURLOPT_COOKIEFILE, 'cookie.txt');
curl_setopt($ch, CURLOPT_COOKIEJAR, 'cookie.txt');
//curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
$output1 = curl_exec($ch);

///*
curl_setopt($ch, CURLOPT_URL, "http://solarproxy.com/browse.php?u=http%3A%2F%2Fwww.senderbase.org&b=12&f=norefer");
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_HEADER, true);
curl_setopt($ch, CURLOPT_AUTOREFERER, true);
curl_setopt($ch, CURLOPT_COOKIEFILE, 'cookie.txt');
curl_setopt($ch, CURLOPT_COOKIEJAR, 'cookie.txt');
//curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
$output2 = curl_exec($ch);
//*/

curl_setopt($ch, CURLOPT_URL, "http://solarproxy.com/browse.php?u=http%3A%2F%2Fwww.senderbase.org%2Fsenderbase_queries%2Fdetailip%3Fsearch_string%3D209.85.218.215&b=12");
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_HEADER, true);
curl_setopt($ch, CURLOPT_AUTOREFERER, true);
curl_setopt($ch, CURLOPT_COOKIEFILE, 'cookie.txt');
curl_setopt($ch, CURLOPT_COOKIEJAR, 'cookie.txt');
//curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
$output3 = curl_exec($ch);

curl_close($ch);

//echo $output1;
echo "\r\n-------------------------------------------------\r\n";
echo $output2;
echo "\r\n-------------------------------------------------\r\n";
echo $output3;
?>