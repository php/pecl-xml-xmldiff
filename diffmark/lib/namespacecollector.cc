#include "namespacecollector.hh"
#include <string>
#include <string.h>

using std::string;

NamespaceCollector::NamespaceCollector(const std::string &stem,
    const char *nsurl):
    stem(stem),
    nsurl(nsurl)
{
}

int NamespaceCollector::get_unused_number(xmlNodePtr m, xmlNodePtr n)
{
    fill(m);
    fill(n);

    bool use_max = false;
    int max = 1;
    for (std::set<TNsDecl>::const_iterator i = namespaces.begin();
	 i != namespaces.end();
	 ++i)
    {
	if (i->first == stem) {
	    use_max = true;
	} else {
	    if ((i->first.length() > stem.length()) &&
		    !strncmp(stem.c_str(), i->first.c_str(),
			stem.length())) {
		int potential_max = 0;

		for (string::const_iterator j = i->first.begin() +
			stem.length();
		    j != i->first.end();
		    ++j)
		{
		    if ((*j < '0') || (*j > '9')) {
			potential_max = 0;
			break;
		    }

		    potential_max = 10 * potential_max + (*j - '0');
		}

		if (potential_max > max) {
		    max = potential_max;
		}
	    }
	}
    }

    return use_max ? (max + 1) : -1;
}

void NamespaceCollector::fill(xmlNodePtr n)
{
    xmlNsPtr ns = n->nsDef;
    while (ns) {
	if (!(ns->href)) {
	    if (ns->prefix) {
		string s = "invalid XML: no namespace declaration "
		    "for prefix ";
		s += string(reinterpret_cast<const char *>(ns->prefix));
		throw s;
	    } else {
		throw string("invalid XML: empty namespace declaration");
	    }
	}

	if (!strcmp(nsurl.c_str(),
		reinterpret_cast<const char *>(ns->href))) {
	    string s = "input tree contains the reserved namespace ";
	    s += nsurl;
	    throw s;
	}

	if (ns->prefix) {
	    namespaces.insert(TNsDecl(
		string(reinterpret_cast<const char *>(ns->prefix)),
		string(reinterpret_cast<const char *>(ns->href))));
	}

	ns = ns->next;
    }

    xmlNodePtr ch = n->children;
    while (ch) {
	fill(ch);
	ch = ch->next;
    }
}
