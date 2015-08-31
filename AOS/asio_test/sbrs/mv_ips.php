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

chdir('/home/bl');
$log_file = "mv_" . date('Y-m-d') . ".log";

$rc1 = -1;
$rc2 = -1;

file_put_contents($log_file, "moving *.ips to gas1...", FILE_APPEND); 
system("scp /home/bl/*.ips localhost:/opt/mozart", $rc1);
system("ssh root@localhost \"mv /opt/mozart/*.ips /opt/mozart/sbrs_server/update\"", $rc2);
file_put_contents($log_file, "OK (scp=$rc1, ssh=$rc2)\n", FILE_APPEND);

for($i=2; $i<=3; ++$i)
{
	file_put_contents($log_file, "moving *.ips to gas{$i}...", FILE_APPEND);
	system("scp /home/bl/*.ips gas{$i}:/opt/mozart", $rc1);
	system("ssh root@gas{$i} \"mv /opt/mozart/*.ips /opt/mozart/sbrs_server/update\"", $rc2);
	file_put_contents($log_file, "OK (scp=$rc1, ssh=$rc2)\n", FILE_APPEND);
}

system("rm /home/bl/*.ips -f", $rc1);
file_put_contents($log_file, "cleaning *.ips...OK (rm=$rc1)\n", FILE_APPEND);
?>