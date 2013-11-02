#ifndef xdoc_hh
#define xdoc_hh

#include <string>
#include <libxml/tree.h>

class XDoc
{
public:
    XDoc();
    XDoc(xmlDocPtr doc);
    XDoc(const XDoc &other);
    ~XDoc();
    XDoc &operator=(const XDoc &other);

    operator xmlDocPtr() { return doc; }
    operator const xmlDocPtr() const { return doc; }

    xmlDocPtr yank();

private:
    xmlDocPtr doc;
    int *ref_cnt;

    void add_ref() const { ++*ref_cnt; }
    void del_ref();
};

#endif
