--TEST--
Ensure libxml behaviour isn't changed after diff/merge
--SKIPIF--
<?php if (!extension_loaded("xmldiff")) print "skip"; ?>
--FILE--
<?php

$original = new DOMDocument();
$original->loadXML('<p></p>');
$compare = new DOMDocument();
$compare->loadXML('<p></p>');

$diff = new XMLDiff\DOM;
/* This was changing the blanks handling in libxml*/
$diff->diff($original, $compare);

$dom_document = new DOMDocument();
$dom_document->preserveWhiteSpace = true;

$dom_document->loadXML('<p><ins>something</ins> <ins>something else</ins></p>');
echo $dom_document->saveXML();

?>
==DONE==
--EXPECT--
<?xml version="1.0"?>
<p><ins>something</ins> <ins>something else</ins></p>
==DONE==

