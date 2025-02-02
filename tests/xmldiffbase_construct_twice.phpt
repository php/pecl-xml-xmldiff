--TEST--
Check for xmldiff presence
--SKIPIF--
<?php if (!extension_loaded("xmldiff")) print "skip"; ?>
--FILE--
<?php 
$test = new XMLDiff\Memory("hello");
$test->__construct("world");
echo $test->diff('<root/>', '<root2/>');
$test->__construct();
echo $test->diff('<root/>', '<root2/>');
?>
--EXPECT--
<?xml version="1.0"?>
<dm:diff xmlns:dm="world">
  <dm:delete>
    <root/>
  </dm:delete>
  <dm:insert>
    <root2/>
  </dm:insert>
</dm:diff>
<?xml version="1.0"?>
<dm:diff xmlns:dm="http://www.locus.cz/diffmark">
  <dm:delete>
    <root/>
  </dm:delete>
  <dm:insert>
    <root2/>
  </dm:insert>
</dm:diff>
