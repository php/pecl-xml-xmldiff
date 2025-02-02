/*-
 * Copyright (c) 2013 Anatol Belski
 * All rights reserved.
 *
 * Author: Anatol Belski <ab@php.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php_xmldiff.h"

zend_object_handlers default_xmldiff_handlers;

/* If you declare any globals in php_xmldiff.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(xmldiff)
*/

/* True global resources - no need for thread safety here
static int le_xmldiff;
*/

static zend_class_entry *XMLDiffBase_ce;
static zend_class_entry *XMLDiffDOM_ce;
static zend_class_entry *XMLDiffFile_ce;
static zend_class_entry *XMLDiffMemory_ce;
static zend_class_entry *XMLDiffException_ce;

#ifndef PHP_FE_END
#define PHP_FE_END {NULL, NULL, NULL}
#endif

/* {{{ xmldiff_functions[] */
const zend_function_entry xmldiff_functions[] = {
	PHP_FE_END
};
/* }}} */

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(XMLDiff_construct, 0, 0, 0)
	ZEND_ARG_INFO(0, nsurl)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(XMLDiff_diff, 0, 0, 2)
	ZEND_ARG_INFO(0, from)
	ZEND_ARG_INFO(0, to)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(XMLDiff_merge, 0, 0, 2)
	ZEND_ARG_INFO(0, src)
	ZEND_ARG_INFO(0, diff)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ xmldiff_methods[] */
const zend_function_entry XMLDiffBase_methods[] = {
	PHP_ME(XMLDiffBase, __construct, XMLDiff_construct, ZEND_ACC_PUBLIC)
	PHP_ME(XMLDiffBase, diff, XMLDiff_diff, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_ME(XMLDiffBase, merge, XMLDiff_merge, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_FE_END
};

const zend_function_entry XMLDiffDOM_methods[] = {
	PHP_ME(XMLDiffDOM, diff, XMLDiff_diff, ZEND_ACC_PUBLIC)
	PHP_ME(XMLDiffDOM, merge, XMLDiff_merge, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

const zend_function_entry XMLDiffFile_methods[] = {
	PHP_ME(XMLDiffFile, diff, XMLDiff_diff, ZEND_ACC_PUBLIC)
	PHP_ME(XMLDiffFile, merge, XMLDiff_merge, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

const zend_function_entry XMLDiffMemory_methods[] = {
	PHP_ME(XMLDiffMemory, diff, XMLDiff_diff, ZEND_ACC_PUBLIC)
	PHP_ME(XMLDiffMemory, merge, XMLDiff_merge, ZEND_ACC_PUBLIC)
	PHP_FE_END
};
/* }}} */

static const zend_module_dep xmldiff_deps[] = {/*{{{*/
	ZEND_MOD_REQUIRED("dom")
	ZEND_MOD_REQUIRED("libxml")
	{NULL, NULL, NULL}
};/*}}}*/

/* {{{ xmldiff_module_entry
 */
zend_module_entry xmldiff_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	xmldiff_deps,
	"xmldiff",
	xmldiff_functions,
	PHP_MINIT(xmldiff),
	PHP_MSHUTDOWN(xmldiff),
	NULL,
	NULL,
	PHP_MINFO(xmldiff),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_XMLDIFF_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_XMLDIFF
ZEND_GET_MODULE(xmldiff)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("xmldiff.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_xmldiff_globals, xmldiff_globals)
	STD_PHP_INI_ENTRY("xmldiff.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_xmldiff_globals, xmldiff_globals)
PHP_INI_END()
*/
/* }}} */

#if PHP_MAJOR_VERSION >= 7
void
php_xmldiff_obj_destroy(zend_object *obj)
{/*{{{*/
	struct ze_xmldiff_obj *zxo = php_xmldiff_fetch_obj(obj);

	zend_object_std_dtor(&zxo->zo);

	if (zxo->nsurl) {
		efree(zxo->nsurl);
	}
}/*}}}*/
#else
void
php_xmldiff_object_destroy(void *obj TSRMLS_DC)
{/*{{{*/
	struct ze_xmldiff_obj *zxo = (struct ze_xmldiff_obj *)obj;

	zend_object_std_dtor(&zxo->zo TSRMLS_CC);

	if (zxo->nsurl) {
		efree(zxo->nsurl);
	}

	efree(zxo);
}/*}}}*/
#endif

#if PHP_MAJOR_VERSION >= 7
zend_object *
php_xmldiff_object_init(zend_class_entry *ce)
{/*{{{*/
	struct ze_xmldiff_obj *zxo;

	zxo = (struct ze_xmldiff_obj *)ecalloc(1, sizeof(struct ze_xmldiff_obj));

	zend_object_std_init(&zxo->zo, ce);
	zxo->zo.handlers = &default_xmldiff_handlers;

	zxo->nsurl = NULL;

	return &zxo->zo;
}/*}}}*/
#else
zend_object_value
php_xmldiff_object_init(zend_class_entry *ce TSRMLS_DC)
{/*{{{*/
	zend_object_value ret;
	struct ze_xmldiff_obj *zxo;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 4
	zval *tmp;
#endif

	zxo = (struct ze_xmldiff_obj *)emalloc(sizeof(struct ze_xmldiff_obj));
	memset(&zxo->zo, 0, sizeof(zend_object));

	zend_object_std_init(&zxo->zo, ce TSRMLS_CC);
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 4
	zend_hash_copy(zxo->zo.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref,
												(void *) &tmp, sizeof(zval *));
#else
	object_properties_init(&zxo->zo, ce);
#endif

	zxo->nsurl = NULL;

	ret.handle = zend_objects_store_put(zxo, NULL,
						(zend_objects_free_object_storage_t) php_xmldiff_object_destroy,
						NULL TSRMLS_CC);
	ret.handlers = &default_xmldiff_handlers;

	return ret;
}/*}}}*/
#endif

/* {{{ php_xmldiff_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_xmldiff_init_globals(zend_xmldiff_globals *xmldiff_globals)
{
	xmldiff_globals->global_value = 0;
	xmldiff_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(xmldiff)
{
	zend_class_entry ce;

	/*
	REGISTER_INI_ENTRIES();
	*/

	LIBXML_TEST_VERSION;

	memcpy(&default_xmldiff_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	default_xmldiff_handlers.clone_obj = NULL;
#if PHP_MAJOR_VERSION >= 7
	default_xmldiff_handlers.offset = XtOffsetOf(struct ze_xmldiff_obj, zo);
	default_xmldiff_handlers.free_obj = php_xmldiff_obj_destroy;
#endif

	INIT_CLASS_ENTRY(ce, "XMLDiff\\Base", XMLDiffBase_methods);
	ce.create_object = php_xmldiff_object_init;
	XMLDiffBase_ce = zend_register_internal_class(&ce TSRMLS_CC);
	XMLDiffBase_ce->ce_flags = ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;

	INIT_CLASS_ENTRY(ce, "XMLDiff\\DOM", XMLDiffDOM_methods);
	ce.create_object = php_xmldiff_object_init;
#if PHP_MAJOR_VERSION >= 7
	XMLDiffDOM_ce = zend_register_internal_class_ex(&ce, XMLDiffBase_ce);
#else
	XMLDiffDOM_ce = zend_register_internal_class_ex(&ce, XMLDiffBase_ce, "XMLDiff\\Base" TSRMLS_CC);
#endif

	INIT_CLASS_ENTRY(ce, "XMLDiff\\File", XMLDiffFile_methods);
	ce.create_object = php_xmldiff_object_init;
#if PHP_MAJOR_VERSION >= 7
	XMLDiffFile_ce = zend_register_internal_class_ex(&ce, XMLDiffBase_ce);
#else
	XMLDiffFile_ce = zend_register_internal_class_ex(&ce, XMLDiffBase_ce, "XMLDiff\\Base" TSRMLS_CC);
#endif

	INIT_CLASS_ENTRY(ce, "XMLDiff\\Memory", XMLDiffMemory_methods);
	ce.create_object = php_xmldiff_object_init;
#if PHP_MAJOR_VERSION >= 7
	XMLDiffMemory_ce = zend_register_internal_class_ex(&ce, XMLDiffBase_ce);
#else
	XMLDiffMemory_ce = zend_register_internal_class_ex(&ce, XMLDiffBase_ce, "XMLDiff\\Base" TSRMLS_CC);
#endif

	INIT_CLASS_ENTRY(ce, "XMLDiff\\Exception", NULL);
#if PHP_MAJOR_VERSION >= 7
	XMLDiffException_ce = zend_register_internal_class_ex(&ce, zend_exception_get_default());
#else
	XMLDiffException_ce = zend_register_internal_class_ex(
		&ce, NULL, "exception" TSRMLS_CC
	);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(xmldiff)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(xmldiff)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "xmldiff support", "enabled");
	php_info_print_table_header(2, "xmldiff version", PHP_XMLDIFF_VERSION);
#ifdef HAVE_LIBDIFFMARK
	php_info_print_table_header(2, "diffmark library", "system");
#else
	php_info_print_table_header(2, "diffmark library", "0.10 bundled (patch level 1)");
#endif
	php_info_print_table_row(2, "Revision", "$Id$");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

static char *
php_xmldiff_get_nsurl(struct ze_xmldiff_obj *zxo)
{/*{{{*/
	char *nsurl = (char *)diffmark::nsurl;

	if (NULL != zxo) {
		if (NULL != zxo->nsurl) {
			nsurl = zxo->nsurl;
		}
	}

	return nsurl;
}/*}}}*/

static XDoc
php_xmldiff_do_diff(const XDoc &xFrom, const XDoc &xTo, struct ze_xmldiff_obj *zxo TSRMLS_DC)
{/*{{{*/
	XDoc xDiff;
	xmlNodePtr m, n;

	m = xutil::get_root_element(xFrom);
	n = xutil::get_root_element(xTo);

	Diff dm(diffmark::get_unique_prefix(m, n), php_xmldiff_get_nsurl(zxo));
	xDiff = dm.diff_nodes(m, n);

	return xDiff;
}/*}}}*/

static XDoc
php_xmldiff_do_merge(const XDoc &xSrc, const XDoc &xDiff, struct ze_xmldiff_obj *zxo TSRMLS_DC)
{/*{{{*/
	XDoc xMerge;

	Merge builder(php_xmldiff_get_nsurl(zxo), xSrc);
	xMerge = builder.merge(xutil::get_root_element(xDiff));

	return xMerge;
}/*}}}*/

PHP_XMLDIFF_API void
php_xmldiff_throw_exception_no_va(const char *msg, long code TSRMLS_DC)
{/*{{{*/
	zend_throw_exception_ex(
			XMLDiffException_ce,
			code TSRMLS_CC,
			"%s",
			(NULL == msg ? "" : (char *)msg)
	);
}/*}}}*/

PHP_XMLDIFF_API xmlDocPtr
php_xmldiff_do_diff_doc(xmlDocPtr fromXmlDoc, xmlDocPtr toXmlDoc, struct ze_xmldiff_obj *zxo TSRMLS_DC)
{/*{{{*/
	xmlDocPtr retDoc = NULL;
	XDoc xDiff;
	XDoc xFrom(fromXmlDoc);
	XDoc xTo(toXmlDoc);

	try {
		xDiff = php_xmldiff_do_diff(xFrom, xTo, zxo TSRMLS_CC);
		retDoc = xDiff.yank();
	} catch (std::bad_alloc &x) {
		php_xmldiff_throw_exception_no_va(x.what(), PHP_XMLDIFF_THROW_BADALLOC TSRMLS_CC);
	} catch (std::string &x) {
		php_xmldiff_throw_exception_no_va(x.c_str(), PHP_XMLDIFF_THROW_DIFF TSRMLS_CC);
	}

	// Non-owned, so destructor should not clean the internal documents.
	xFrom.yank();
	xTo.yank();

	return retDoc;
}/*}}}*/

PHP_XMLDIFF_API xmlDocPtr
php_xmldiff_do_merge_doc(xmlDocPtr srcXmlDoc, xmlDocPtr diffXmlDoc, struct ze_xmldiff_obj *zxo TSRMLS_DC)
{/*{{{*/
	xmlDocPtr retDoc = NULL;
	XDoc xMerge;
	XDoc xSrc(srcXmlDoc);
	XDoc xDiff(diffXmlDoc);

	try {
		xMerge = php_xmldiff_do_merge(xSrc, xDiff, zxo TSRMLS_CC);
		retDoc = xMerge.yank();
	} catch (std::bad_alloc &x) {
		php_xmldiff_throw_exception_no_va(x.what(), PHP_XMLDIFF_THROW_BADALLOC TSRMLS_CC);
	} catch (std::string &x) {
		php_xmldiff_throw_exception_no_va(x.c_str(), PHP_XMLDIFF_THROW_MERGE TSRMLS_CC);
	}

	// Non-owned, so destructor should not clean the internal documents.
	xSrc.yank();
	xDiff.yank();

	return retDoc;
}/*}}}*/

PHP_XMLDIFF_API xmlChar *
php_xmldiff_do_diff_file(const char *from, const char *to, struct ze_xmldiff_obj *zxo TSRMLS_DC)
{/*{{{*/
	xmlChar *ret = NULL;

	try {
		XDoc xFrom = xutil::parse_file(from);
		XDoc xTo = xutil::parse_file(to);
		XDoc xDiff;
		int size = 0;

		if (NULL != xFrom && NULL != xTo) {
			xDiff = php_xmldiff_do_diff(xFrom, xTo, zxo TSRMLS_CC);
			xmlDocDumpFormatMemory(xDiff, &ret, &size, 1);
		}
	} catch (string &x) {
		php_xmldiff_throw_exception_no_va(x.c_str(), PHP_XMLDIFF_THROW_DIFF TSRMLS_CC);

		return NULL;
	}

	return ret;
}/*}}}*/

PHP_XMLDIFF_API xmlChar *
php_xmldiff_do_merge_file(const char *src, const char *diff, struct ze_xmldiff_obj *zxo TSRMLS_DC)
{/*{{{*/
	xmlChar *ret = NULL;

	try {
		XDoc xSrc = xutil::parse_file(src);
		XDoc xDiff = xutil::parse_file(diff);
		XDoc xMerge;
		int size = 0;

		if (NULL != xSrc && NULL != xDiff) {
			xMerge = php_xmldiff_do_merge(xSrc, xDiff, zxo TSRMLS_CC);
			xmlDocDumpFormatMemory(xMerge, &ret, &size, 1);
		}
	} catch (string &x) {
		php_xmldiff_throw_exception_no_va(x.c_str(), PHP_XMLDIFF_THROW_MERGE TSRMLS_CC);

		return NULL;
	}

	return ret;
}/*}}}*/

PHP_XMLDIFF_API xmlChar *
php_xmldiff_do_diff_memory(const char *from, size_t from_len, const char *to, size_t to_len, struct ze_xmldiff_obj *zxo TSRMLS_DC)
{/*{{{*/
	xmlDocPtr fromXmlDoc = xmlParseMemory(from, from_len);
	xmlDocPtr toXmlDoc = xmlParseMemory(to, to_len);
	xmlDocPtr retDoc;
	xmlChar *ret = NULL;
	int size = 0;

	if (NULL != fromXmlDoc && NULL != toXmlDoc) {
		retDoc = php_xmldiff_do_diff_doc(fromXmlDoc, toXmlDoc, zxo TSRMLS_CC);
		xmlDocDumpFormatMemory(retDoc, &ret, &size, 1);
		xmlFreeDoc(retDoc);
	}

	xmlFreeDoc(fromXmlDoc);
	xmlFreeDoc(toXmlDoc);

	return ret;
}/*}}}*/

PHP_XMLDIFF_API xmlChar *
php_xmldiff_do_merge_memory(const char *src, size_t src_len, const char *diff, size_t diff_len, struct ze_xmldiff_obj *zxo TSRMLS_DC)
{/*{{{*/
	xmlDocPtr srcXmlDoc = xmlParseMemory(src, src_len);
	xmlDocPtr diffXmlDoc = xmlParseMemory(diff, diff_len);
	xmlDocPtr retDoc;
	xmlChar *ret = NULL;
	int size = 0;

	if (NULL != srcXmlDoc && NULL != diffXmlDoc) {
		retDoc = php_xmldiff_do_merge_doc(srcXmlDoc, diffXmlDoc, zxo TSRMLS_CC);
		xmlDocDumpFormatMemory(retDoc, &ret, &size, 1);
		xmlFreeDoc(retDoc);
	}

	xmlFreeDoc(srcXmlDoc);
	xmlFreeDoc(diffXmlDoc);

	return ret;
}/*}}}*/

static int
php_xmldiff_is_valid_dom_obj(zval *arg, const char *name TSRMLS_DC)
{/*{{{*/
	if (!instanceof_function(Z_OBJCE_P(arg), dom_node_class_entry TSRMLS_CC)) {
		zend_throw_exception_ex(
				XMLDiffException_ce,
				PHP_XMLDIFF_THROW_INVALID_DOM_OBJECT TSRMLS_CC,
				"Expected the $%s argument to be an instance of DOMNode",
				name
		);
		return 0;
	}

	return 1;
}/*}}}*/

static void
php_xmldiff_set_out_dom_props(zval *obj TSRMLS_DC)
{/*{{{*/
#if PHP_MAJOR_VERSION >= 7
	dom_object *domObj = (dom_object *)php_dom_obj_from_obj(Z_OBJ_P(obj));
#else
	dom_object *domObj = (dom_object *)zend_object_store_get_object(obj TSRMLS_CC);
#endif

	if (!domObj->document->doc_props) {
		domObj->document->doc_props = (libxml_doc_props *)emalloc(sizeof(libxml_doc_props));
	}

	domObj->document->doc_props->formatoutput = 1;
	domObj->document->doc_props->validateonparse = 0;
	domObj->document->doc_props->resolveexternals = 0;
	domObj->document->doc_props->preservewhitespace = 0;
	domObj->document->doc_props->substituteentities = 0;
	domObj->document->doc_props->stricterror = 1;
	domObj->document->doc_props->recover = 0;
	domObj->document->doc_props->classmap = NULL;
}/*}}}*/

/* {{{ public void XMLDiff\Base::__construct([string $nsurl]) */
PHP_METHOD(XMLDiffBase, __construct)
{
	struct ze_xmldiff_obj *zxo;
	char *nsurl = NULL;
#if PHP_MAJOR_VERSION >= 7
	size_t nsurl_len = 0;
#else
	int nsurl_len = 0;
#endif

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &nsurl, &nsurl_len) == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zxo = php_xmldiff_fetch_obj(Z_OBJ_P(getThis()));
#else
	zxo = (struct ze_xmldiff_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (nsurl_len > 0) {
		zxo->nsurl = estrdup(nsurl);
	}

} /* }}} */

/*{{{ abstract public mixed XMLDiff\Base::diff(mixed $from, mixed $to) */
PHP_METHOD(XMLDiffBase, diff)
{
}/*}}}*/

/*{{{ abstract public mixed XMLDiff\Base::diff(mixed $from, mixed $to) */
PHP_METHOD(XMLDiffBase, merge)
{
}/*}}}*/

/*{{{  public DOMDocument XMLDiff\DOM::diff(DOMNode $from, DOMNode $to) */
PHP_METHOD(XMLDiffDOM, diff)
{
	struct ze_xmldiff_obj *zxo;
	zval *zfrom, *zto;
	dom_object *from, *to;
	xmlDocPtr fromXmlDoc, toXmlDoc, retDoc;
	int domRetStatus, old_keep_blanks;
	xmlNodePtr retNode;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "oo", &zfrom, &zto) == FAILURE) {
		return;
	}

	if (!php_xmldiff_is_valid_dom_obj(zfrom, "from" TSRMLS_CC)) {
		return;
	}

	if (!php_xmldiff_is_valid_dom_obj(zto, "to" TSRMLS_CC)) {
		return;
	}

	old_keep_blanks = xmlKeepBlanksDefault(0);

#if PHP_MAJOR_VERSION >= 7
	zxo = php_xmldiff_fetch_obj(Z_OBJ_P(getThis()));

	/* prepare diff data */
	from = php_dom_obj_from_obj(Z_OBJ_P(zfrom));
	to = php_dom_obj_from_obj(Z_OBJ_P(zto));
#else
	zxo = (struct ze_xmldiff_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	/* prepare diff data */
	from = (dom_object *)zend_object_store_get_object(zfrom TSRMLS_CC);
	to = (dom_object *)zend_object_store_get_object(zto TSRMLS_CC);
#endif

	fromXmlDoc = (xmlDocPtr)from->document->ptr;
	toXmlDoc = (xmlDocPtr)to->document->ptr;

	/* do diff */
	retDoc = php_xmldiff_do_diff_doc(fromXmlDoc, toXmlDoc, zxo TSRMLS_CC);
	if (NULL == retDoc) {
		RETURN_NULL();
	}
	retNode = (xmlNodePtr)retDoc;

	/* return the resulting dom object */
	XMLDIFF_DOM_RET_OBJ(retNode, &domRetStatus, NULL);

	/* set return object properties */
	php_xmldiff_set_out_dom_props(return_value TSRMLS_CC);

	/* xmlCleanupParser(); */

	xmlKeepBlanksDefault(old_keep_blanks);
}/*}}}*/

/*{{{ public DOMDocument XMLDiff\DOM::merge(DOMNode $m, DOMNode $to) */
PHP_METHOD(XMLDiffDOM, merge)
{
	struct ze_xmldiff_obj *zxo;
	zval *zsrc, *zdiff;
	dom_object *src, *diff;
	xmlDocPtr srcXmlDoc, diffXmlDoc, retDoc;
	int domRetStatus, old_keep_blanks;
	xmlNodePtr retNode;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "oo", &zsrc, &zdiff) == FAILURE) {
		return;
	}

	if (!php_xmldiff_is_valid_dom_obj(zsrc, "src" TSRMLS_CC)) {
		return;
	}

	if (!php_xmldiff_is_valid_dom_obj(zdiff, "diff" TSRMLS_CC)) {
		return;
	}

	old_keep_blanks = xmlKeepBlanksDefault(0);

#if PHP_MAJOR_VERSION >= 7
	zxo = php_xmldiff_fetch_obj(Z_OBJ_P(getThis()));

	/* prepare merge data */
	src = php_dom_obj_from_obj(Z_OBJ_P(zsrc));
	diff = php_dom_obj_from_obj(Z_OBJ_P(zdiff));
#else
	zxo = (struct ze_xmldiff_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	/* prepare merge data */
	src = (dom_object *)zend_object_store_get_object(zsrc TSRMLS_CC);
	diff = (dom_object *)zend_object_store_get_object(zdiff TSRMLS_CC);
#endif

	srcXmlDoc = (xmlDocPtr)src->document->ptr;
	diffXmlDoc = (xmlDocPtr)diff->document->ptr;

	/* do merge */
	retDoc = php_xmldiff_do_merge_doc(srcXmlDoc, diffXmlDoc, zxo TSRMLS_CC);
	if (NULL == retDoc) {
		RETURN_NULL();
	}
	retNode = (xmlNodePtr)retDoc;

	/* return the resulting dom object */
	XMLDIFF_DOM_RET_OBJ(retNode, &domRetStatus, NULL);

	/* set return object properties */
	php_xmldiff_set_out_dom_props(return_value TSRMLS_CC);

	/* xmlCleanupParser(); */

	xmlKeepBlanksDefault(old_keep_blanks);
}/* }}} */

/*{{{ public string XMLDiff\File::diff(string $from, string $to) */
PHP_METHOD(XMLDiffFile, diff)
{
	struct ze_xmldiff_obj *zxo;
	char *from_fl, *to_fl, *ret;
#if PHP_MAJOR_VERSION >= 7
	size_t from_fl_len, to_fl_len;
	int old_keep_blanks;
#else
	int from_fl_len, to_fl_len, old_keep_blanks;
#endif

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &from_fl, &from_fl_len, &to_fl, &to_fl_len) == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zxo = php_xmldiff_fetch_obj(Z_OBJ_P(getThis()));
#else
	zxo = (struct ze_xmldiff_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	old_keep_blanks = xmlKeepBlanksDefault(0);

	ret = (char *)php_xmldiff_do_diff_file(from_fl, to_fl, zxo TSRMLS_CC);

	if (NULL == ret) {
		RETURN_NULL();
	}

#if PHP_MAJOR_VERSION >= 7
	RETVAL_STRING(ret);
#else
	RETVAL_STRING(ret, 1);
#endif

	xmlFree(ret);
	/* xmlCleanupParser(); */

	xmlKeepBlanksDefault(old_keep_blanks);
} /* }}} */

/*{{{ public string XMLDiff\File::merge(string $src, string $diff) */
PHP_METHOD(XMLDiffFile, merge)
{
	struct ze_xmldiff_obj *zxo;
	char *src_fl, *diff_fl, *ret;
#if PHP_MAJOR_VERSION >= 7
	size_t src_fl_len, diff_fl_len;
	int old_keep_blanks;
#else
	int src_fl_len, diff_fl_len, old_keep_blanks;
#endif

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &src_fl, &src_fl_len, &diff_fl, &diff_fl_len) == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zxo = php_xmldiff_fetch_obj(Z_OBJ_P(getThis()));
#else
	zxo = (struct ze_xmldiff_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	old_keep_blanks = xmlKeepBlanksDefault(0);

	ret = (char *)php_xmldiff_do_merge_file(src_fl, diff_fl, zxo TSRMLS_CC);

	if (NULL == ret) {
		RETURN_NULL();
	}

#if PHP_MAJOR_VERSION >= 7
	RETVAL_STRING(ret);
#else
	RETVAL_STRING(ret, 1);
#endif

	xmlFree(ret);
	/* xmlCleanupParser(); */

	xmlKeepBlanksDefault(old_keep_blanks);
} /* }}} */

/*{{{ public string XMLDiff\Memory::diff(string $from, string $to) */
PHP_METHOD(XMLDiffMemory, diff)
{
	struct ze_xmldiff_obj *zxo;
	char *from, *to, *ret;
#if PHP_MAJOR_VERSION >= 7
	size_t from_len, to_len;
	int old_keep_blanks;
#else
	int from_len, to_len, old_keep_blanks;
#endif

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &from, &from_len, &to, &to_len) == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zxo = php_xmldiff_fetch_obj(Z_OBJ_P(getThis()));
#else
	zxo = (struct ze_xmldiff_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	old_keep_blanks = xmlKeepBlanksDefault(0);

	ret = (char *)php_xmldiff_do_diff_memory(from, from_len, to, to_len, zxo TSRMLS_CC);

	if (NULL == ret) {
		RETURN_NULL();
	}

#if PHP_MAJOR_VERSION >= 7
	RETVAL_STRING(ret);
#else
	RETVAL_STRING(ret, 1);
#endif

	xmlFree(ret);
	/* xmlCleanupParser(); */

	xmlKeepBlanksDefault(old_keep_blanks);
} /* }}} */

/*{{{ public string XMLDiff\Memory::merge(string $src, string $diff) */
PHP_METHOD(XMLDiffMemory, merge)
{
	struct ze_xmldiff_obj *zxo;
	char *src, *diff, *ret;
#if PHP_MAJOR_VERSION >= 7
	size_t src_len, diff_len;
	int old_keep_blanks;
#else
	int src_len, diff_len, old_keep_blanks;
#endif

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &src, &src_len, &diff, &diff_len) == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zxo = php_xmldiff_fetch_obj(Z_OBJ_P(getThis()));
#else
	zxo = (struct ze_xmldiff_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	old_keep_blanks = xmlKeepBlanksDefault(0);

	ret = (char *)php_xmldiff_do_merge_memory(src, src_len, diff, diff_len, zxo TSRMLS_CC);

	if (NULL == ret) {
		RETURN_NULL();
	}

#if PHP_MAJOR_VERSION >= 7
	RETVAL_STRING(ret);
#else
	RETVAL_STRING(ret, 1);
#endif

	xmlFree(ret);
	/* xmlCleanupParser(); */

	xmlKeepBlanksDefault(old_keep_blanks);
} /* }}} */

/*
* Local variables:
* tab-width: 4
* c-basic-offset: 4
* End:
* vim600: noet sw=4 ts=4 fdm=marker
* vim<600: noet sw=4 ts=4
*/
