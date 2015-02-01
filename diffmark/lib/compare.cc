#include "compare.hh"
#include "compareimpl.hh"
#include <sstream>
#include <iostream>
#include <assert.h>
#include <string.h>

using std::stringstream;

static int compare_elem(xmlNodePtr m, xmlNodePtr n, bool deep);
static int compare_content(xmlNodePtr m, xmlNodePtr n);
static int compare_pi(xmlNodePtr m, xmlNodePtr n);
static int compare_children(xmlNodePtr m, xmlNodePtr n);

// A wrapper around xmlGetProp/xmlGetNsProp. Must be passed a valid
// attribute (belonging to some node). Throws on error (never returns
// NULL); the returned value must be freed (by xmlFree).
static xmlChar *get_value(xmlAttrPtr a);

#if 0
#define TRACE(trace_arg) std::cerr << trace_arg << std::endl
#else
#define TRACE(trace_arg)
#endif

int compare(xmlNodePtr m, xmlNodePtr n, bool deep)
{
    TRACE("enter compare");

    assert(m);
    assert(n);

    int type = m->type - n->type;
    if (type) {
	return type;
    }

    switch (m->type) {
    case XML_ELEMENT_NODE:
	return compare_elem(m, n, deep);
    case XML_TEXT_NODE:
    case XML_COMMENT_NODE:
    case XML_CDATA_SECTION_NODE:
	return compare_content(m, n);
    case XML_PI_NODE:
	return compare_pi(m, n);
    }

    stringstream s;
    s << "unsupported node type " << m->type;
    throw s.str();
}

bool compareimpl::have_nulls(const void *p, const void *q, int &res)
{
    if (!p) {
	res = q ? -1 : 0;
	return true;
    }

    if (!q) {
	res = 1;
	return true;
    }
    
    return false;
}

int compareimpl::compare_ns(xmlNsPtr a, xmlNsPtr b)
{
    assert(a);
    assert(b);
    assert(a->href);
    assert(b->href);

    int href = strcmp(reinterpret_cast<const char *>(a->href),
	reinterpret_cast<const char *>(b->href));
    if (href) {
	return href;
    }

    int res;
    if (have_nulls(a->prefix, b->prefix, res)) {	
	return res;
    } else {
	return strcmp(reinterpret_cast<const char *>(a->prefix),
	    reinterpret_cast<const char *>(b->prefix));
    }
}

int compareimpl::compare_attr(xmlAttrPtr a, xmlAttrPtr b)
{
    int name = compareimpl::compare_name<xmlAttrPtr>(a, b);
    if (name) {
	return name;
    }

    xmlChar *v = get_value(a);
    xmlChar *w = get_value(b);

    int rv = strcmp(reinterpret_cast<char *>(v),
	reinterpret_cast<char *>(w));

    xmlFree(w);
    xmlFree(v);

    return rv;
}

static int compare_elem(xmlNodePtr m, xmlNodePtr n, bool deep)
{
    int name = compareimpl::compare_name<xmlNodePtr>(m, n);
    if (name) {
	return name;
    }

    int ns_def = compareimpl::compare_set<xmlNsPtr>(m->nsDef,
	n->nsDef);
    if (ns_def) {
	return ns_def;
    }

    int properties = compareimpl::compare_set<xmlAttrPtr>(
	m->properties,
	n->properties);
    if (properties) {
	return properties;
    }

    return deep ? compare_children(m, n) : 0;
}

static xmlChar *get_value(xmlAttrPtr a)
{
    assert(a);
    assert(a->parent);

    xmlChar *out;

    if (a->ns) {
	assert(a->ns->href);
	out = xmlGetNsProp(a->parent, a->name, a->ns->href);
    } else {
	out = xmlGetProp(a->parent, a->name);
    }

    if (!out) {
	// should be pretty rare, but let's say an allocation might
	// have failed
	throw std::string("cannot get attribute value");
    }

    return out;
}

static int compare_pi(xmlNodePtr p, xmlNodePtr q)
{
    assert(p->name);
    assert(q->name);

    int name = strcmp(reinterpret_cast<const char *>(p->name),
	reinterpret_cast<const char *>(q->name));
    if (name) {
	return name;
    }
    
    return compare_content(p, q);
}

static int compare_content(xmlNodePtr m, xmlNodePtr n)
{
    int res;
    if (compareimpl::have_nulls(m->content, n->content, res)) {
	return res;
    }

    return strcmp(reinterpret_cast<char *>(m->content),
	reinterpret_cast<char *>(n->content));
}

static int compare_children(xmlNodePtr m, xmlNodePtr n)
{
    xmlNodePtr mch = m->children;
    xmlNodePtr nch = n->children;

    while (mch && nch) {
	int res = compare(mch, nch, true);
	if (res) {
	    return res;
	}

	mch = mch->next;
	nch = nch->next;
    }

    if (!mch) {
	return nch ? -1 : 0;
    } else {
	assert(!nch);
	return 1;
    }
}

