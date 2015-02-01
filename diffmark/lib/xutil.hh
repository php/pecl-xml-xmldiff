// based on dom.c from XML::LibXML, which is Copyright (c) 2001
// Christian Glahn

#ifndef xutil_hh
#define xutil_hh

#include <string>
#include <libxml/tree.h>

class XDoc;

namespace xutil {

xmlNodePtr get_root_element(xmlDocPtr doc);

XDoc parse_file(const char *fname);

std::string get_node_name(xmlNodePtr n);

void append_child(xmlNodePtr ex_parent, xmlNodePtr new_child);

void remove_child(xmlNodePtr parent, xmlNodePtr child);

void remove_children(xmlNodePtr n);

// Remove all namespace declarations equivalent to top (which must
// have both local name and href and must be declared above n) from n
// and its descendants. Make them use top instead of the removed
// copies.
void unify_namespace(xmlNsPtr top, xmlNodePtr n);

std::string flatten(xmlNodePtr n);

}

#endif

