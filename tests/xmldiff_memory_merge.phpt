--TEST--
Check for xmldiff Memory merge
--SKIPIF--
<?php if (!extension_loaded("xmldiff")) print "skip"; ?>
--FILE--
<?php

$zxo = new XMLDiff\Memory;

$dir = dirname(__FILE__) . "/testdata/merge";

for ($i = 0; $i < 5; ++$i) {

	$from = file_get_contents(sprintf("$dir/%02da.xml", $i));
	$to = file_get_contents(sprintf("$dir/%02db.xml", $i));

	$diff = $zxo->diff($from, $to);

	$d = sprintf("$dir/%02dd.xml", $i);

	$rep = array(' ', "\n");
	$d0 = str_replace($rep, '', $diff);
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
TEST 02 pass
TEST 03 pass
TEST 04 pass
==DONE==

