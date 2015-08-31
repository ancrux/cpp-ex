<?php

define ( 'ROOTD_SOCKET_PATH',  	"/tmp/rootd"   );
define ( 'SUDO', "/usr/bin/sudo" );

// ---------------------------------------------------------------------

function daemon_exec( $cmd )
{
    $new_cmd = sprintf( SUDO . " -b sh -c '%s'", $cmd );
    shell_exec( $new_cmd );
    return TRUE;
}

// ---------------------------------------------------------------------

function sudo_exec( $cmd, & $rsl )
{
    $new_cmd = sprintf( SUDO . ' sh -c "%s"', $cmd );
    ob_start();
    system( $new_cmd, $rsl );
    $ret = ob_get_contents();
    ob_end_clean();    

    return $ret;
}


// ---------------------------------------------------------------------
function root_exec( $cmd, & $rsl )
{
    for ( $i = 0; $i < 10; $i++ )
    {
        $tcpRootd = fsockopen( ROOTD_SOCKET_PATH, 0 );
        if ( $tcpRootd ) break;
        sleep( 1 );
    }

    if( !$tcpRootd )
    {
        syslog( LOG_DEBUG, "Can not connect to rootd" );
        return "Can not connect to rootd";
    }
    else
    {
        fwrite( $tcpRootd, $cmd );
        stream_set_timeout($tcpRootd, 1);
        $result = "";
        $rsl = trim(fread($tcpRootd, 5));
        $size = trim(fread($tcpRootd, 5));
        $result = fread( $tcpRootd, $size);
        fclose( $tcpRootd );
        return $result;
    }
}

// ---------------------------------------------------------------------

function exec_cmd( $cmd, & $result, $legacy = FALSE )
{
    $result = "";
	if( $legacy == TRUE )
      $result = root_exec( $cmd, $rsl );
    else
	  $result = sudo_exec( $cmd, $rsl );

    //echo "return=$rsl\n";
    //echo "result=$result\n";

    if ( $rsl != 0 ) 
    {
        syslog( LOG_DEBUG, "exec_cmd (cmd) : {$cmd}" );
        syslog( LOG_DEBUG, "exec_cmd (result) : {$result}" );
    }

	if ($rsl == "" ) // timeout
		return 0;
	else
		return $rsl;
}

// --------------------------------------------------------------------

?>
