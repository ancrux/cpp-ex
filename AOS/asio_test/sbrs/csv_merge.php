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

if ( $argc < 3 )
{
	exit("usage: {$argv[0]} <input_dir> <output_dir>\n");
}

chdir(dirname($argv[0]));
$in_dir = trim($argv[1]);
echo "in_dir: $in_dir\n";
$out_dir = trim($argv[2]);
echo "out_dir: $out_dir\n";

scan_folder($in_dir, $out_dir);

function scan_folder($folder, $out_folder)
{
$d = @dir($folder);
if ( !$d )
	exit("open '$folder' error!\n");	

while (false !== ($entry = $d->read())) {
	if ( $entry == '.' || $entry == '..' ) continue;
	
	$path = $d->path . '/' . $entry;
	if ( is_dir($path) )
		scan_folder($path, $out_folder);
		
	$dot = strrpos($entry, ".");
	if ( $dot === false || strtolower(substr($entry, $dot)) != ".csv" ) continue;
	echo "merging '$path'...\n";
	merge_csv($path, $out_folder);
}
$d->close();
}

function merge_csv($path, $out_folder)
{
	$fname = explode('.', basename($path));
	if ( $fname[0] )
	{
		$content = file_get_contents($path);
		$out_file = $out_folder . "/" . $fname[0] . ".csv";
		file_put_contents($out_file, $content, FILE_APPEND);
	}
}
?>