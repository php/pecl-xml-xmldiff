#include "diff.hh"
#include "xutil.hh"
#include <libxml/tree.h>
#include <sstream>
#include <vector>
#include <assert.h>

#if 0
#include <iostream>

#define TRACE(trace_arg) std::cerr << trace_arg << std::endl
#else
#define TRACE(trace_arg)
#endif

using std::string;
using std::stringstream;
using namespace xutil;

static int get_node_count(xmlNodePtr n)
{
    int count = 1;

    xmlNodePtr ch = n->children;
    while (ch) {
	count += get_node_count(ch);
	ch = ch->next;
    }

    return count;
}

static xmlNsPtr new_ns(xmlNodePtr n, const string &prefix, const string &url)
{
    xmlNsPtr ns = xmlNewNs(n,
	reinterpret_cast<const xmlChar *>(url.c_str()),
	reinterpret_cast<const xmlChar *>(prefix.c_str()));
    if (!ns) {
	string s = "cannot create ";
	s += prefix;
	s += ':';
	s += url;
	throw s;
    }

    return ns;
}

static xmlNodePtr new_node(const char *name)
{
    xmlNodePtr n = xmlNewNode(0,
	reinterpret_cast<const xmlChar *>(name));
    if (!n) {
	string s = "cannot create ";
	s += name;
	throw s;
    }

    return n;
}

static void prune(xmlNodePtr n)
{
    xmlNodePtr ch = n->children;
    while (ch) {
	remove_children(ch);
	ch = ch->next;
    }
}

static std::vector<xmlNodePtr> get_children(xmlNodePtr n)
{
    std::vector<xmlNodePtr> out;
    xmlNodePtr ch = n->children;
    while (ch) {
	out.push_back(ch);
	ch = ch->next;
    }

    return out;
}

Diff::Diff(const std::string &nsprefix, const std::string &nsurl):
    Target(nsurl),
    nsprefix(nsprefix),
    dest(),
    dest_ns(0),
    dest_point(0)
{
}

string Diff::get_ns_prefix() const
{
    return nsprefix;
}

XDoc Diff::get_dest()
{
    return dest;
}

xmlDocPtr Diff::diff_nodes(xmlNodePtr m, xmlNodePtr n)
{
    diff(m, n);

    xmlNodePtr top = get_root_element(dest);
    xmlNodePtr ch = top->children;
    while (ch) {
	unify_namespace(dest_ns, ch);
	ch = ch->next;
    }

    return dest.yank();
}

void Diff::diff(xmlNodePtr m, xmlNodePtr n)
{
    if (!do_diff_nodes(m, n, true)) {
	return;
    }

    XDoc wa_dest = dest;
    xmlNsPtr wa_dest_ns = dest_ns;

    dest_point = 0;
    dest_ns = 0;
    dest = XDoc();
    do_diff_nodes(m, n, false);

    if (get_node_count(get_root_element(wa_dest)) <
	get_node_count(get_root_element(dest))) {
	dest = wa_dest;
	dest_ns = wa_dest_ns;
    }
}

bool Diff::do_diff_nodes(xmlNodePtr m, xmlNodePtr n, bool use_upd_attr)
{
    bool has_upd_attr = false;

    dest_point = new_node("diff");
    dest_ns = new_ns(dest_point, nsprefix, get_ns_url());
    xmlSetNs(dest_point, dest_ns);

    xmlDocSetRootElement(dest, dest_point);

    std::equal_to<xmlNodePtr> is_equal;
    if (is_equal(m, n)) {
	append_copy();
    } else {
	if (compare(m, n, false)) {
	    if (use_upd_attr && m->children && n->children) {
		descend(m, n);

		string old_name = get_node_name(m);
		xmlSetNsProp(dest_point,
		    dest_ns,
		    reinterpret_cast<const xmlChar *>("update"),
		    reinterpret_cast<const xmlChar *>(old_name.c_str()));

		// std::cerr << lcsimpl::flatten(dest_point) << std::endl;

		has_upd_attr = true;
	    } else {
		replace(m, n);
	    }
	} else {
	    descend(m, n);
	}
    }

    return has_upd_attr;
}

void Diff::descend(xmlNodePtr m, xmlNodePtr n)
{
    xmlNodePtr seq = import_tip(n);

    append_child(dest_point, seq);
    dest_point = seq;

    std::vector<xmlNodePtr> a = get_children(m);
    std::vector<xmlNodePtr> b = get_children(n);
    traverse_balanced(a, b);

    xmlNodePtr last = seq->last;
    if (last &&
	(get_node_name(last) == get_scoped_name("delete"))) {
	prune(last);
    }
}

void Diff::replace(xmlNodePtr m, xmlNodePtr n)
{
    xmlNodePtr del = new_dm_node("delete");
    append_child(dest_point, del);

    append_child(del, import_tip(m));
    append_insert(n);
}

bool Diff::combine_pair(xmlNodePtr n, bool reverse)
{
    TRACE(this << "->combine_pair(" << get_node_name(n) << ", " \
	<< reverse << ')');

    assert(dest_point);

    xmlNodePtr last = dest_point->last;
    assert(last);

    xmlNodePtr m = last->last;
    assert(m);

    if ((m->type != XML_ELEMENT_NODE) ||
	(n->type != XML_ELEMENT_NODE)) {
	return false;
    }

    if (reverse) {
	std::swap(m, n);
    }

    Diff dm(nsprefix, get_ns_url());
    dm.diff(m, n);
    XDoc dom = dm.dest;
    xmlNodePtr root = get_root_element(dom);
    xmlNodePtr ch = root->children;
    assert(ch);

    xmlNodePtr moved = last->last;
    if (!moved->prev) {
	remove_child(dest_point, last);
    } else {
	remove_child(last, moved);
    }

    if (combine_first_child(ch, get_scoped_name("delete")) ||
	combine_first_child(ch, get_scoped_name("insert"))) {
	ch = ch->next;
    }

    while (ch) {
	xmlNodePtr subtree = import_node(ch);
	append_child(dest_point, subtree);

	ch = ch->next;
    }

    return true;
}

bool Diff::combine_first_child(xmlNodePtr first_child,
    const std::string &checked_name)
{
    xmlNodePtr last = dest_point->last;
    if (!last) {
	return false;
    }

    if ((get_node_name(last) != checked_name) ||
	(get_node_name(first_child) != checked_name)) {
	return false;
    }

    xmlNodePtr cnt = first_child->children;
    while (cnt) {
	append_child(last, import_node(cnt));
	cnt = cnt->next;
    }

    return true;
}

void Diff::append_insert(xmlNodePtr n)
{
    xmlNodePtr ins = new_dm_node("insert");
    append_child(dest_point, ins);
    append_child(ins, import_node(n));
}

void Diff::append_delete(xmlNodePtr n)
{
    xmlNodePtr del = new_dm_node("delete");
    append_child(dest_point, del);
    append_child(del, import_node(n));
}

void Diff::append_copy()
{
    xmlNodePtr copy = new_dm_node("copy"); 
    append_child(dest_point, copy);
    xmlSetProp(copy,
	reinterpret_cast<const xmlChar *>("count"),
	reinterpret_cast<const xmlChar *>("1"));
}
    
void Diff::on_match()
{
    assert(dest_point);

    xmlNodePtr last = dest_point->last;
    if (!last) {
	append_copy();
    } else if (get_node_name(last) != get_scoped_name("copy")) {
	if (get_node_name(last) == get_scoped_name("delete")) {
	    prune(last);
	}
	append_copy();
    } else {
	int count = 1 + get_count_attr(last);
	stringstream s;
	s << count;
	xmlSetProp(last,
	    reinterpret_cast<const xmlChar *>("count"),
	    reinterpret_cast<const xmlChar *>(s.str().c_str()));
    }
}

void Diff::on_insert(xmlNodePtr n)
{
    TRACE(this << "->on_insert(" << get_node_name(n) << ')');
    assert(n);

    xmlNodePtr last = dest_point->last;
    if (!last) {
	append_insert(n);
    } else if (get_node_name(last) == get_scoped_name("insert")) {
	append_child(last, import_node(n));
    } else if (get_node_name(last) != get_scoped_name("delete")) {
	append_insert(n);
    } else {
	if (!combine_pair(n, false)) {
	    append_insert(n);
	}
    }
}

void Diff::on_delete(xmlNodePtr n)
{
    TRACE(this << "->on_delete(" << get_node_name(n) << ')');
    assert(n);

    xmlNodePtr last = dest_point->last;
    if (!last) {
	append_delete(n);
    } else if (get_node_name(last) == get_scoped_name("delete")) {
	prune(last);
	append_child(last, import_node(n));
    } else if (get_node_name(last) != get_scoped_name("insert")) {
	append_delete(n);
    } else {
	if (!combine_pair(n, true)) {
	    append_delete(n);
	}
    }
}

xmlNodePtr Diff::new_dm_node(const char *name)
{
    xmlNodePtr n = xmlNewNode(dest_ns,
	reinterpret_cast<const xmlChar *>(name));
    if (!n) {
	string s = "cannot create ";
	s += name;
	throw s;
    }

    xmlSetTreeDoc(n, dest);

    return n;
}
