--TEST--
Check for xmldiff DOM diff
--SKIPIF--
<?php if (!extension_loaded("xmldiff")) print "skip"; ?>
--FILE--
<?php

$zxo = new XMLDiff\DOM;

$dir = dirname(__FILE__) . "/testdata/roundup";

for ($i = 0; $i < 24; ++$i) {

	$f = sprintf("$dir/%02da.xml", $i);
	$t = sprintf("$dir/%02db.xml", $i);

	$opts = LIBXML_NOENT;
	$from = new DOMDocument;
	$from->preserveWhiteSpace = false;
	$from->load($f, $opts);
	$to = new DOMDocument;
	$to->preserveWhiteSpace = false;
	$to->load($t, $opts);

	$diff = $zxo->diff($from, $to);

	$d = sprintf("$dir/%02dd.xml", $i);

	$rep = array(' ', "\n");
	$d0 = str_replace($rep, '', $diff->saveXml());
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
TEST 05 pass
TEST 06 pass
TEST 07 pass
TEST 08 pass
TEST 09 pass
TEST 10 pass
TEST 11 pass
TEST 12 pass
TEST 13 pass
TEST 14 pass
TEST 15 pass
TEST 16 pass
TEST 17 pass
TEST 18 pass
TEST 19 pass
TEST 20 pass
TEST 21 pass
TEST 22 pass
TEST 23 pass
==DONE==

