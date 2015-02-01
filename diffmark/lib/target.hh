#ifndef target_hh
#define target_hh

#include <string>
#include <libxml/tree.h>

class XDoc;

class Target
{
protected:
    Target(const std::string &nsurl);

    std::string get_scoped_name(const char *tail);
    std::string get_ns_url();

    xmlNodePtr import_node(xmlNodePtr n);
    xmlNodePtr import_tip(xmlNodePtr n);

    virtual std::string get_ns_prefix() const = 0;
    virtual XDoc get_dest() = 0;

    static int get_count_attr(xmlNodePtr instr);

private:
    std::string nsurl;

    xmlNodePtr do_import_node(xmlNodePtr n);
};

inline std::string Target::get_ns_url()
{
    return nsurl;
}

#endif
