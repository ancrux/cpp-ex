<?
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

$bl_file = "bl.data";

echo "downloading $bl_file...\n";
system("rsync -e ssh -L bl@blrsync.spamcop.net:bl.data ~/bl.data");
echo "download completed!\n";

$arrLine = @file($bl_file);
$timestamp = date('Y-m-d_H_i_s');

// create ips
$bl_ips_file = "bl.ips";

echo "creating $bl_ips_file...\n";
$output = '';
$count = -1;
foreach($arrLine as $i => $line)
{
	if ( $count < 0 )
	{
		if ( strncmp($line, "127.0.0.", strlen("127.0.0.")) == 0 )
			++$count;
		continue;
	}
	
	$ip = trim($line);
	$output .= ip_to_5byte($ip, 0x00); // always Poor IP
	++$count;
}
file_put_contents($bl_ips_file, $output);
echo "$bl_ips_file created! (count:$count)\n";


// create isd
$bl_now_file = "bl.now";
$bl_isd_file = "{$timestamp}.isd";

// load bl.now
if ( !file_exists($bl_now_file) )
{
	$bl = array();
	file_put_contents($bl_now_file, serialize($bl));
}
$bl = unserialize(file_get_contents($bl_now_file));

// calculate ADD
echo "diff ADD...\n";
reset($arrLine);
$output = '';
$count = -1;
foreach($arrLine as $i => $line)
{
	if ( $count < 0 )
	{
		if ( strncmp($line, "127.0.0.", strlen("127.0.0.")) == 0 )
			++$count;
		continue;
	}
	
	$ip = trim($line);
	if ( isset($bl[$ip]) )
	{
		$bl[$ip] = 0;
		continue;
	}
	else
	{
		$bl[$ip] = 0;
		$output .= ip_to_5byte($ip, 0x00); // diff ADD Poor IP
		++$count; // count for ADD
	}
}
echo "writing $bl_isd_file for ADD (count:$count)...\n";
file_put_contents("A{$timestamp}.ips", $output); // isd & ips is only compatible if contains all POOR ips
//file_put_contents("A{$bl_isd_file}", $output);
//file_put_contents($bl_isd_file, $output);
echo "writing done!\n";

echo "diff SUB...\n";
$output = '';
$count = 0;
foreach($bl as $ip => $val)
{
	if ( $val != 0 )
	{
		$output .= ip_to_5byte($ip, -1); // diff SUB Poor IP
		unset($bl[$ip]);
		++$count; // count for SUB
	}
	else
		$bl[$ip] = -1;
}
echo "writing $bl_isd_file for SUB (count:$count)...\n";
file_put_contents("B{$bl_isd_file}", $output);
//file_put_contents($bl_isd_file, $output, FILE_APPEND);
echo "writing done!\n";

// save bl.now
echo "saving $bl_now_file...\n";
file_put_contents($bl_now_file, serialize($bl));
echo "$bl_now_file saved!\n";

// copy ips to gas1
system("scp A{$timestamp}.ips gas1.email-home.com:/home/bl");

//ssh root@<host> "mv /tmp/M%d%02d%02d.icc /usr/local/mozart/rtcb/cgac"

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