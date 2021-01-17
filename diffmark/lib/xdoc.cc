#include "xdoc.hh"
#include <assert.h>
#include <iostream>

using std::string;

XDoc::XDoc():
    doc(xmlNewDoc(reinterpret_cast<const xmlChar *>("1.0"))),
    ref_cnt(new int(1))
{
}

XDoc::XDoc(xmlDocPtr doc):
    doc(doc),
    ref_cnt(new int(1))
{
}

XDoc::XDoc(const XDoc &other):
    doc(other.doc),
    ref_cnt(other.ref_cnt)
{
    add_ref();
}

XDoc::~XDoc()
{
    del_ref();
}

XDoc &XDoc::operator=(const XDoc &other)
{
    other.add_ref();
    del_ref();

    doc = other.doc;
    ref_cnt = other.ref_cnt;

    return *this;
}

void XDoc::del_ref()
{
    if (!(--*ref_cnt)) {
        if (doc) {
	    xmlFreeDoc(doc);
	}

	delete ref_cnt;
    }
}

xmlDocPtr XDoc::yank()
{
    if (*ref_cnt != 1) {
	throw string("shared reference cannot be yanked");
    }

    if (!doc) {
	throw string("document already yanked");
    }

    xmlDocPtr rv = doc;
    doc = 0;
    return rv;
}
