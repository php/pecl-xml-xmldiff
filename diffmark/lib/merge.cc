#include "merge.hh"
#include "xutil.hh"
#include <sstream>
#include <iostream>
#include <assert.h>
#include <string.h>

#if 0
#define TRACE(trace_arg) std::cerr << trace_arg << std::endl
#else
#define TRACE(trace_arg)
#endif

#include "lcs.hh"

using std::string;
using std::stringstream;
using namespace xutil;

Merge::Merge(const std::string &nsurl,
    xmlDocPtr src_tree):
    Target(nsurl),
    nsprefix(),
    reserved_ns(0),
    src(src_tree),
    src_point(get_root_element(src_tree)),
    dest(),
    dest_point(0)
{
}

xmlDocPtr Merge::merge(xmlNodePtr diff_node)
{
    assert(diff_node);

    nsprefix = init_ns_prefix(diff_node);
    check_top_node_name(diff_node);

    xmlNodePtr ch = diff_node->children;
    if (!ch) {
	throw string("diff node has no children");
    }

    do_merge(ch);
    ch = ch->next;

    while (ch) {
	do_merge(ch);
	ch = ch->next;
    }

    return dest.yank();
}

std::string Merge::get_ns_prefix() const
{
    assert(!nsprefix.empty()); // don't call this function too soon
    return nsprefix;
}

XDoc Merge::get_dest()
{
    return dest;
}

void Merge::do_merge(xmlNodePtr tree)
{
    assert(tree);

    string name = get_node_name(tree);
    TRACE("do_merge(" << name << ')');

    if (name == get_scoped_name("delete")) {
	handle_delete(tree);
    } else if (name == get_scoped_name("copy")) {
	handle_copy(tree);
    } else if (name == get_scoped_name("insert")) {
	handle_insert(tree);
    } else {
	if (tree->ns &&
	    tree->ns->prefix &&
	    (get_ns_prefix() == 
		reinterpret_cast<const char *>(tree->ns->prefix))) {
	    assert(tree->name);
	    string s = "unknown instruction ";
	    s += reinterpret_cast<const char *>(tree->name);
	    throw s;
	}

	copy_shallow(tree);

	xmlNodePtr ch = tree->children;
	while (ch) {
	    do_merge(ch);
	    ch = ch->next;
	}

	elevate_dest_point();
    }
}

void Merge::handle_insert(xmlNodePtr insert_instruction)
{
    TRACE("handle_insert");

    xmlNodePtr ch = insert_instruction->children;
    if (!ch) {
	throw string("insert node has no children");
    }

    while (ch) {
	xmlNodePtr n = import_node(ch);
	append(n);

	ch = ch->next;
    }
}

void Merge::handle_delete(xmlNodePtr delete_instruction)
{
    xmlNodePtr ch = delete_instruction->children;
    if (!ch) {
	throw string("delete node has no children");
    }

    if (!src_point) {
	throw string("nothing to delete");
    }

    bool finished = false;
    while (ch) {
	if (get_node_name(ch) != get_node_name(src_point)) {
	    string s = get_node_name(ch);
	    s += " isn't there to be deleted; source has ";
	    s += get_node_name(src_point);
	    s += " instead";
	    throw s;
	}

	xmlNodePtr checked_sibling = src_point->next;
	if (checked_sibling) {
	    src_point = checked_sibling;
	} else {
	    if (finished) {
		throw string("too many nodes to delete");
	    }

	    finished = true;
	}

	ch = ch->next;
    }

    if (finished) {
	elevate_src_point();
    }
}

void Merge::handle_copy(xmlNodePtr copy_instruction)
{
    if (!src_point) {
	throw string("nothing to copy");
    }

    int count = get_count_attr(copy_instruction);
    while (count > 0) {
	copy_deep();
	--count;
    }
}

void Merge::copy_shallow(xmlNodePtr tip)
{
    assert(src_point);

    xmlNodePtr n = import_tip(tip);
    purge_dm(n);
    append(n);

    xmlNodePtr checked_child = src_point->children;
    if (checked_child) {
	src_point = checked_child;
    } else {
	advance_src_point();
    }

    dest_point = n;
}

void Merge::copy_deep()
{
    assert(src_point);

    xmlNodePtr n = import_node(src_point);
    append(n);
    advance_src_point();
}

void Merge::advance_src_point()
{
    assert(src_point);
    
    xmlNodePtr sibling = src_point->next;
    if (sibling) {
	src_point = sibling;
    } else {
	elevate_src_point();
    }
}

void Merge::elevate_src_point()
{
    assert(src_point);

    xmlNodePtr top = get_root_element(src);
    if (src_point == top) {
	return;
    }

    xmlNodePtr previous = src_point->parent;
    while (!previous->next) {
	if (previous == top) {
	    return;
	}

	previous = previous->parent;
    }

    src_point = previous->next;
}

void Merge::elevate_dest_point()
{
    assert(dest_point);

    xmlNodePtr top = get_root_element(dest);
    if (dest_point != top) {
	dest_point = dest_point->parent;
    }
}

string Merge::init_ns_prefix(xmlNodePtr diff_node)
{
    assert(diff_node);

    reserved_ns = diff_node->nsDef;
    if (!reserved_ns) {
	throw string("document node has no namespace declarations");
    }

    if (reserved_ns->next) {
	throw string("document node has more than 1 namespace declaration");
    }

    const char *url = reinterpret_cast<const char *>(reserved_ns->href);
    if (!(url && (get_ns_url() == url))) {
	if (!url) {
	    url = "empty";
	}

	stringstream s;
	s << "document node namespace declaration must be " <<
	    get_ns_url() << " (not " << url << ')';
	throw s.str();
    }

    if (!reserved_ns->prefix) {
	throw string("document node namespace declaration has no prefix");
    }

    assert(*(reserved_ns->prefix));
    return string(reinterpret_cast<const char *>(reserved_ns->prefix));
}

void Merge::check_top_node_name(xmlNodePtr diff_node)
{
    string actual = get_node_name(diff_node);
    if (actual != get_scoped_name("diff")) {
	string s = "invalid document node ";
	s += actual;
	throw s;
    }
}

void Merge::append(xmlNodePtr n)
{
    assert(n);
    TRACE("append(" << get_node_name(n) << ')');

    if (!dest_point) {
	xmlDocSetRootElement(dest, n);
    } else {
	append_child(dest_point, n);
    }
}

void Merge::purge_dm(xmlNodePtr n)
{
    xmlUnsetNsProp(n,
	reserved_ns,
	reinterpret_cast<const xmlChar *>("update"));

    check_attr(n);

    unify_namespace(reserved_ns, n);
}

void Merge::check_attr(xmlNodePtr n)
{
    assert(n);

    if (n->type != XML_ELEMENT_NODE) {
	return;
    }

    xmlAttrPtr attr = n->properties;
    while (attr) {
	assert(attr->name);

	if (attr->ns && is_reserved(attr->ns)) {
	    string msg = "unknown attribute \"";
	    msg += reinterpret_cast<const char *>(attr->name);
	    msg += "\" in the reserved namespace";
	    throw msg;
	}

	attr = attr->next;
    }
}

bool Merge::is_reserved(xmlNsPtr ns) const
{
    assert(reserved_ns);
    assert(reserved_ns->prefix);
    assert(reserved_ns->href);
    assert(ns);

    return ns->prefix &&
	!strcmp(reinterpret_cast<const char *>(reserved_ns->prefix),
	    reinterpret_cast<const char *>(ns->prefix)) &&
	ns->href &&
	!strcmp(reinterpret_cast<const char *>(reserved_ns->href),
	    reinterpret_cast<const char *>(ns->href));
}
