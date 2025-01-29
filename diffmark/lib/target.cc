#include "target.hh"
#include "xdoc.hh"
#include "xutil.hh"
#include <iostream>
#include <stdlib.h>
#include <assert.h>

#if 0
#define TRACE(trace_arg) std::cerr << trace_arg << std::endl
#else
#define TRACE(trace_arg)
#endif

using std::string;

Target::Target(const std::string &nsurl):
    nsurl(nsurl)
{
}

string Target::get_scoped_name(const char *tail)
{
    string name = get_ns_prefix();
    name += ':';
    name += tail;
    return name;
}

bool Target::equals_scoped_name(const xmlNode *n, const char *tail) const
{
    return xutil::node_name_equals(n, get_ns_prefix().c_str(), tail);
}

xmlNodePtr Target::import_node(xmlNodePtr n)
{
    xmlNodePtr nn = do_import_node(n);
    xmlReconciliateNs(get_dest(), nn);
    return nn;
}

xmlNodePtr Target::do_import_node(xmlNodePtr n)
{
    assert(n->type != XML_DTD_NODE);
    assert(n->type != XML_ENTITY_REF_NODE);

    XDoc dest = get_dest();

    xmlNodePtr nn = xmlDocCopyNode(n, dest, 1);
    if (!nn) {
	throw string("cannot copy node");
    }

    return nn;
}

xmlNodePtr Target::import_tip(xmlNodePtr n)
{
    assert(n->type != XML_DTD_NODE);
    assert(n->type != XML_ENTITY_REF_NODE);

    xmlNodePtr nc = n->children;
    n->children = 0;
    try {
	xmlNodePtr nn = do_import_node(n);
	n->children = nc;
	xmlReconciliateNs(get_dest(), nn);
	return nn;
    } catch (...) {
	n->children = nc;
	throw;
    }
}

int Target::get_count_attr(xmlNodePtr instr)
{
    xmlChar *attr = xmlGetProp(instr, 
	reinterpret_cast<const xmlChar *>("count"));
    if (!attr) {
	throw string("no count attribute");
    }

    int count = atoi(reinterpret_cast<const char *>(attr));
    if (count <= 0 ) {
	string s = "invalid count ";
	s += reinterpret_cast<const char *>(attr);
	xmlFree(attr);
	throw s;
    }

    xmlFree(attr);
    return count;
}
