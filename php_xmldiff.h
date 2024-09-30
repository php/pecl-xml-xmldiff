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


#ifndef PHP_XMLDIFF_H
#define PHP_XMLDIFF_H

#define U_SHOW_CPLUSPLUS_API 0

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __STDC_LIMIT_MACROS
# define __STDC_LIMIT_MACROS 1
#endif
#ifndef __STDC_CONSTANT_MACROS
# define __STDC_CONSTANT_MACROS 1
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_exceptions.h"

#ifdef PHP_WIN32
# define _ALLOW_KEYWORD_MACROS
#endif

#ifdef PHP_WIN32
#	define PHP_XMLDIFF_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_XMLDIFF_API __attribute__ ((visibility("default")))
#else
#	define PHP_XMLDIFF_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#include "ext/libxml/php_libxml.h"
#include "ext/dom/xml_common.h"

#ifdef __cplusplus
} /* extern C */
#endif

extern zend_module_entry xmldiff_module_entry;
#define phpext_xmldiff_ptr &xmldiff_module_entry

#include "merge.hh"
#include "diff.hh"
#include "nspace.hh"
#include "xdoc.hh"
#include "xutil.hh"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

#if PHP_MAJOR_VERSION >= 7
struct ze_xmldiff_obj {
	char *nsurl;
	zend_object zo;
};
static zend_always_inline struct ze_xmldiff_obj *
php_xmldiff_fetch_obj(zend_object *obj)
{/*{{{*/
	return (struct ze_xmldiff_obj *)((char *)obj - XtOffsetOf(struct ze_xmldiff_obj, zo));
}/*}}}*/
#else
struct ze_xmldiff_obj {
	zend_object zo;
	char *nsurl;
};
#endif

#define PHP_XMLDIFF_VERSION "1.1.4-dev"

PHP_MINIT_FUNCTION(xmldiff);
PHP_MSHUTDOWN_FUNCTION(xmldiff);
PHP_MINFO_FUNCTION(xmldiff);

PHP_METHOD(XMLDiffBase, __construct);
PHP_METHOD(XMLDiffBase, diff);
PHP_METHOD(XMLDiffBase, merge);

PHP_METHOD(XMLDiffDOM, diff);
PHP_METHOD(XMLDiffDOM, merge);

PHP_METHOD(XMLDiffFile, diff);
PHP_METHOD(XMLDiffFile, merge);

PHP_METHOD(XMLDiffMemory, diff);
PHP_METHOD(XMLDiffMemory, merge);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(xmldiff)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(xmldiff)
*/

/* In every utility function you add that needs to use variables 
   in php_xmldiff_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as XMLDIFF_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifndef TSRMLS_DC
#define TSRMLS_D void
#define TSRMLS_DC
#define TSRMLS_C
#define TSRMLS_CC
#define TSRMLS_FETCH()
#endif

#ifdef ZTS
#define XMLDIFF_G(v) TSRMG(xmldiff_globals_id, zend_xmldiff_globals *, v)
#else
#define XMLDIFF_G(v) (xmldiff_globals.v)
#endif

#define PHP_XMLDIFF_THROW_INVALID_DOM_OBJECT (1 << 0L)
#define PHP_XMLDIFF_THROW_MERGE (1 << 1L)
#define PHP_XMLDIFF_THROW_DIFF (1 << 2L)
#define PHP_XMLDIFF_THROW_NOMEM (1 << 3L)
#define PHP_XMLDIFF_THROW_BADALLOC (1 << 4L)

PHP_XMLDIFF_API void
php_xmldiff_throw_exception_no_va(const char *msg, long code TSRMLS_DC);

PHP_XMLDIFF_API xmlDocPtr
php_xmldiff_do_diff_doc(xmlDocPtr fromXmlDoc, xmlDocPtr toXmlDoc, struct ze_xmldiff_obj *zxo TSRMLS_DC);

PHP_XMLDIFF_API xmlDocPtr
php_xmldiff_do_merge_doc(xmlDocPtr srcXmlDoc, xmlDocPtr diffXmlDoc, struct ze_xmldiff_obj *zxo TSRMLS_DC);

PHP_XMLDIFF_API xmlChar *
php_xmldiff_do_diff_file(const char *from, const char *to, struct ze_xmldiff_obj *zxo TSRMLS_DC);

PHP_XMLDIFF_API xmlChar *
php_xmldiff_do_merge_file(const char *src, const char *diff, struct ze_xmldiff_obj *zxo TSRMLS_DC);

PHP_XMLDIFF_API xmlChar *
php_xmldiff_do_diff_memory(const char *from, size_t from_len, const char *to, size_t to_len, struct ze_xmldiff_obj *zxo TSRMLS_DC);

PHP_XMLDIFF_API xmlChar *
php_xmldiff_do_merge_memory(const char *src, size_t src_len, const char *diff, size_t diff_len, struct ze_xmldiff_obj *zxo TSRMLS_DC);

#if PHP_MAJOR_VERSION > 8 || PHP_MAJOR_VERSION == 8 && PHP_MINOR_VERSION > 3
#  define XMLDIFF_DOM_RET_OBJ(obj, ret, domobject) DOM_RET_OBJ(obj, domobject)
#elif PHP_MAJOR_VERSION >= 7 || PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
#  define XMLDIFF_DOM_RET_OBJ DOM_RET_OBJ
#elif PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 3 && PHP_RELEASE_VERSION > 6
#  define XMLDIFF_DOM_RET_OBJ DOM_RET_OBJ_EX
#else
#  define XMLDIFF_DOM_RET_OBJ(obj, ret, domobject); do { \
       zval *rv = NULL; \
       DOM_RET_OBJ(rv, obj, ret, domobject); \
     } while(0);
#endif

#endif	/* PHP_XMLDIFF_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
