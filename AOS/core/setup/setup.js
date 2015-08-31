// setup script for dynamically modify predefined template variables
// usage: VARS['key'] = 'value';

if ( VARS['HTTPS'] == 'on' )
	VARS['HTTPS_PORT'] = VARS['HTTPS_PORT'];

"executed";