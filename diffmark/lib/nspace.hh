#ifndef nspace_hh
#define nspace_hh

#include <string>
#include <libxml/tree.h>

namespace diffmark {

char const * const nsurl = "http://www.locus.cz/diffmark";

std::string get_unique_prefix(xmlNodePtr m, xmlNodePtr n);

};

#endif
