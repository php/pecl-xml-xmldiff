--TEST--
Check for xmldiff File diff fail
--SKIPIF--
<?php if (!extension_loaded("xmldiff")) print "skip"; ?>
--FILE--
<?php 

$zxo = new XMLDiff\File;

$dir = dirname(__FILE__) . "/testdata/faildiff";

for ($i = 0; $i < 2; ++$i) {
	$from = sprintf("$dir/%02da.xml", $i);
	$to = sprintf("$dir/%02db.xml", $i);

	try {
		$diff = $zxo->diff($from, $to);
	} catch (Exception $e) {
		$d0 = $e->getMessage();
	}
	$d = sprintf("$dir/%02d.err", $i);

	$rep = array(' ', "\n");
	$d0 = 'dm:' . str_replace($rep, '', $d0);
	$d1 = str_replace($rep, '', file_get_contents($d));

	$pass = ($d0 == $d1);
	printf("TEST %02d %s\n", $i, ($pass ? 'pass': 'fail'));
	if (!$pass) {
		echo "COMPUTED: '$d0'\nEXPECTED: '$d1'\n";
	}
}
?>
==DONE==
--EXPECT--
TEST 00 pass
TEST 01 pass
==DONE==

