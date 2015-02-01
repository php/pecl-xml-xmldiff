#ifndef merge_hh
#define merge_hh

#include <string>
#include <libxml/tree.h>
#include "xdoc.hh"
#include "target.hh"

class Merge : private Target
{
public:
    Merge(const std::string &nsurl,
	xmlDocPtr src_tree);

    // call at most once per Merge instance
    xmlDocPtr merge(xmlNodePtr diff_node);

protected:
    virtual std::string get_ns_prefix() const;
    virtual XDoc get_dest();

private:
    std::string nsprefix;
    xmlNsPtr reserved_ns;

    xmlDocPtr src;
    xmlNodePtr src_point;
    XDoc dest;
    xmlNodePtr dest_point;

    std::string init_ns_prefix(xmlNodePtr diff_node);
    void check_top_node_name(xmlNodePtr diff_node);

    void do_merge(xmlNodePtr tree);

    void handle_insert(xmlNodePtr insert_instruction);
    void handle_delete(xmlNodePtr delete_instruction);
    void handle_copy(xmlNodePtr copy_instruction);

    void copy_shallow(xmlNodePtr tip);
    void copy_deep();

    void advance_src_point();
    void elevate_src_point();
    void elevate_dest_point();

    void append(xmlNodePtr n);
    void purge_dm(xmlNodePtr n);
    void check_attr(xmlNodePtr n);
    bool is_reserved(xmlNsPtr ns) const;
};

#endif
