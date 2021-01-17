#include "xutil.hh"
#include "xdoc.hh"
#include "xbuffer.hh"
#include <assert.h>
#include <string.h>

using std::string;

static void add_last(xmlNodePtr leader, xmlNodePtr cur)
{
    assert(leader);
    assert(cur);
    assert(leader != cur);
    assert(cur->type != XML_DOCUMENT_FRAG_NODE);

    xmlNodePtr p = leader->parent;
    cur->parent = p;
       
    leader->next = cur;
    cur->prev = leader;
	   
    if (p) {
	p->last = cur;
    }
}

static void unlink_node(xmlNodePtr n)
{
    assert(n != 0);

    if ( n->prev != NULL ) {
        n->prev->next = n->next;
    }

    if ( n->next != NULL ) {
        n->next->prev = n->prev;
    }

    if ( n->parent != NULL ) {
        if ( n == n->parent->last ) {
            n->parent->last = n->prev;
        }

        if ( n == n->parent->children ) {
            n->parent->children = n->next;
        }
    }

    n->prev = 0;
    n->next = 0;
    n->parent = 0;
}

static bool equal_to(xmlNsPtr top, xmlNsPtr ns)
{
    assert(top);
    assert(top->prefix);
    assert(top->href);
    assert(ns);

    return ns->prefix &&
	!strcmp(reinterpret_cast<const char *>(top->prefix),
	    reinterpret_cast<const char *>(ns->prefix)) &&
	ns->href &&
	!strcmp(reinterpret_cast<const char *>(top->href),
	    reinterpret_cast<const char *>(ns->href));
}

static void repoint(xmlNsPtr top, xmlNodePtr n)
{
    assert(n);

    if (n->type != XML_ELEMENT_NODE) {
	return;
    }

    if (n->ns && equal_to(top, n->ns)) {
	n->ns = top;
    }

    xmlAttrPtr attr = n->properties;
    while (attr) {
	if (attr->ns && equal_to(top, attr->ns)) {
	    attr->ns = top;
	}

	attr = attr->next;
    }

    xmlNodePtr ch = n->children;
    while (ch) {
	repoint(top, ch);
	ch = ch->next;
    }
}

// Remove the namespace equivalent to top (which must have both local
// name and href and must be declared above n) from the list starting
// at ns_head and return the new head.
static xmlNsPtr purge_one(xmlNsPtr top, xmlNsPtr ns_head)
{
    xmlNsPtr ns = ns_head;
    xmlNsPtr prev = 0;
    while (ns) {
	if (equal_to(top, ns)) {
	    if (prev) {
		prev->next = ns->next;
	    } else {
		ns_head = ns->next;
	    }

	    ns->next = 0; // necessary?

	    xmlFreeNs(ns);

	    return ns_head;
	}

	prev = ns;
	ns = ns->next;
    }

    return ns_head;
}

static void purge(xmlNsPtr top, xmlNodePtr n)
{
    if (n->type != XML_ELEMENT_NODE) {
	return;
    }

    // 7Aug2003: Once is enough: the parser won't allow more than 1
    // equivalent ns declaration in one node.
    n->nsDef = purge_one(top, n->nsDef);

    xmlNodePtr ch = n->children;
    while (ch) {
	purge(top, ch);
	ch = ch->next;
    }
}

xmlNodePtr xutil::get_root_element(xmlDocPtr doc)
{
    assert(doc);

    xmlNodePtr de = xmlDocGetRootElement(doc);
    if (!de) {
	throw string("empty document");
    }

    return de;
}

XDoc xutil::parse_file(const char *fname)
{
    xmlDocPtr doc = xmlParseFile(fname);
    if (!doc) {
	string s = "error parsing ";
	s += fname;
	throw s;
    }

    return XDoc(doc);
}

string xutil::get_node_name(xmlNodePtr n)
{
    string out;

    if (n->ns && n->ns->prefix) {
	out = reinterpret_cast<const char *>(n->ns->prefix);
	out += ':';
    }

    if (n->name)
    {
	out += reinterpret_cast<const char *>(n->name);
    }
    else // 21Sep2009: does happen, although perhaps only in a buggy
	 // program...
    {
	out += "<unnamed>";
    }

    return out;
}

void xutil::append_child(xmlNodePtr ex_parent, xmlNodePtr new_child)
{
    assert(ex_parent != 0);
    assert(ex_parent->doc == new_child->doc);

    unlink_node(new_child);

    // 24Jul2003: does this need to be implemented?
    assert(new_child->type != XML_DOCUMENT_FRAG_NODE);

    if (ex_parent->children) {
        add_last(ex_parent->last, new_child);
    } else {
        ex_parent->children = new_child;
        ex_parent->last = new_child;
        new_child->parent= ex_parent;
    }

    xmlReconciliateNs(ex_parent->doc, new_child);     
}

void xutil::remove_child(xmlNodePtr parent, xmlNodePtr child)
{
    assert(parent &&
	child &&
	(child->type != XML_ATTRIBUTE_NODE) &&
	(child->type != XML_NAMESPACE_DECL) &&
	(parent == child->parent));

    unlink_node(child);
}

void xutil::remove_children(xmlNodePtr n)
{
    if (n->children)
    {
	xmlFree(n->children);
	n->children = 0;
	n->last = 0;
    }
}

// 10Aug2003: should probably be moved to Target (equal_to is already
// duplicated in Merge).
void xutil::unify_namespace(xmlNsPtr top, xmlNodePtr n)
{
    repoint(top, n);
    purge(top, n);
}

std::string xutil::flatten(xmlNodePtr n)
{
    assert(n);
    assert(n->doc);

    XBuffer buf;
    int rv = xmlNodeDump(buf, n->doc, n, 0, 0);
    if (rv < 0) {
	throw std::string("cannot dump node");
    }

    return std::string(
	reinterpret_cast<char *>(((xmlBufferPtr)buf)->content));
}

