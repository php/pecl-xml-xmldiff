#ifndef namespacecollector_hh
#define namespacecollector_hh

#include <string>
#include <set>
#include <libxml/tree.h>

class NamespaceCollector
{
public:
    NamespaceCollector(const std::string &stem, const char *nsurl);

    int get_unused_number(xmlNodePtr m, xmlNodePtr n);

private:
    // first: prefix, second: URL
    typedef std::pair<std::string, std::string> TNsDecl;

    std::string stem;
    std::string nsurl;
    std::set<TNsDecl> namespaces;

    void fill(xmlNodePtr n);
};

#endif
