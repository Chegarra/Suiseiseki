
<?php
//error_reporting(E_ALL);
error_reporting(E_ERROR | E_PARSE);

$service_port = getservbyname('www', 'tcp');
$address = gethostbyname('192.168.1.177');
$dirAddres = "192.168.1.177";
//$address = gethostbyname('158.42.181.60');
//$dirAddres = "158.42.181.60";
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);

if ($socket === false) {
    echo "socket_create() falló: razón: " . socket_strerror(socket_last_error()) . "\n";
} else {
	$result = socket_connect($socket, $address, $service_port);
	if ($result === false) {
		echo "socket_connect() falló.\nRazón: ($result) " . socket_strerror(socket_last_error($socket)) . "\n";
	} else {
		
		$ruta = trim($_GET["path"]);
		
		if (strlen($ruta) > 0) {
			$in = "GET " . $ruta . " HTTP/1.1\r\n";
			$in .= "Host: " . $dirAddres . "\r\n";
			$in .= "Connection: Close\r\n\r\n";
			$out = '';
			socket_write($socket, $in, strlen($in));

			//PHP_BINARY_READ
			//PHP_NORMAL_READ
			$DatosRecibidos = "";
			while ($out = socket_read($socket, 2048, PHP_NORMAL_READ)) {
				$DatosRecibidos = $DatosRecibidos . $out;
			}
			
			echo $DatosRecibidos;
			
			socket_close($socket);
		}
	}
}

?>
