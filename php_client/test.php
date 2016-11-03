<?php

require("./vendor/autoload.php");

use Vega\Socket\NsHead;

$params = array(
    'a' => 12,
    'b' => 19,
);
$host = '127.0.0.1';
$port = 8010;
$timeout = 100; 
$pack = mc_pack_array2pack($params, PHP_MC_PACK_V2);
$head = array(
    'provider' => 'edu-as',
    'version'  => 1,
    'body_len' => strlen($pack),
);

$nshead = new NsHead($timeout);

if($nshead->connect($host, $port) === false) {
    echo $nshead->getErrorMsg() . PHP_EOL;
    return;
}

if ($nshead->nshead_write($head, $pack) === false) {
    echo $nshead->getErrorMsg() . PHP_EOL;
    return;
}

$rtn = $nshead->nshead_read(false);
if ($rtn === false) {
    echo $nshead->getErrorMsg() . PHP_EOL;
    return;
}

$result = mc_pack_pack2array($rtn['buf']);
print_r($result);

$nshead->close();
