#include "nspace.hh"
#include "namespacecollector.hh"
#include <sstream>

using std::string;
using std::stringstream;

string diffmark::get_unique_prefix(xmlNodePtr m, xmlNodePtr n)
{
    string prefix("dm");

    NamespaceCollector col(prefix, nsurl);
    int top = col.get_unused_number(m, n);

    if (top != -1) {
	stringstream s;
	s << prefix << top;
	prefix = s.str();
    }

    return prefix;
}

