--TEST--
Check for xmldiff DOM diff fail
--SKIPIF--
<?php if (!extension_loaded("xmldiff")) print "skip"; ?>
--FILE--
<?php 

$zxo = new XMLDiff\DOM;

$dir = dirname(__FILE__) . "/testdata/failmerge";

for ($i = 0; $i < 9; ++$i) {
	$s = sprintf("$dir/%02da.xml", $i);
	$d = sprintf("$dir/%02dd.xml", $i);

	$src = new DOMDocument;
	$src->load($s);
	$diff = new DOMDocument;
	$diff->load($d);

	try {
		$merge = $zxo->merge($src, $diff);
	} catch (Exception $e) {
		$m0 = $e->getMessage();
	}
	$m = sprintf("$dir/%02d.err", $i);

	$rep = array(' ', "\n");
	$m0 = 'dm-merge:' . str_replace($rep, '', $m0);
	$m1 = str_replace($rep, '', file_get_contents($m));

	$pass = ($m0 == $m1);
	printf("TEST %02d %s\n", $i, ($pass ? 'pass': 'fail'));
	if (!$pass) {
		echo "COMPUTED: '$m0'\nEXPECTED: '$m1'\n";
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
==DONE==

