#ifndef diff_hh
#define diff_hh

#include <string>
#include <libxml/tree.h>
#include "compare.hh"
#include "lcs.hh"
#include "target.hh"
#include "xdoc.hh"

namespace std {

template<>
struct less<xmlNodePtr>
{
    bool operator()(xmlNodePtr m, xmlNodePtr n)
    {
	return compare(m, n, true) < 0;
    }
};

template<>
struct equal_to<xmlNodePtr>
{
    bool operator()(xmlNodePtr m, xmlNodePtr n)
    {
	return compare(m, n, true) == 0;
    }
};

}

class Diff : private Target, private LCS<xmlNodePtr>
{
public:
    Diff(const std::string &nsprefix, const std::string &nsurl);

    // call at most once per Diff instance
    xmlDocPtr diff_nodes(xmlNodePtr m, xmlNodePtr n);

protected:
    virtual std::string get_ns_prefix() const;
    virtual XDoc get_dest();

private:    
    virtual void on_match();
    virtual void on_insert(xmlNodePtr n);
    virtual void on_delete(xmlNodePtr n);

    const std::string nsprefix;

    XDoc dest;
    xmlNsPtr dest_ns;
    xmlNodePtr dest_point;

    void diff(xmlNodePtr m, xmlNodePtr n);
    bool do_diff_nodes(xmlNodePtr m, xmlNodePtr n, bool use_upd_attr);

    void descend(xmlNodePtr m, xmlNodePtr n);
    void replace(xmlNodePtr m, xmlNodePtr n);
    bool combine_pair(xmlNodePtr n, bool reverse);

    bool combine_first_child(xmlNodePtr first_child,
	const std::string &checked_name);

    void append_insert(xmlNodePtr n);
    void append_delete(xmlNodePtr n);
    void append_copy();

    xmlNodePtr new_dm_node(const char *name);
};

#endif

