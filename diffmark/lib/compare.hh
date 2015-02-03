#ifndef compare_hh
#define compare_hh

#include <functional>
#include <libxml/tree.h>

int compare(xmlNodePtr m, xmlNodePtr n, bool deep);

namespace compareimpl {

int compare_attr(xmlAttrPtr a, xmlAttrPtr b);

int compare_ns(xmlNsPtr a, xmlNsPtr b);

}

namespace std {

template<>
struct less<xmlNsPtr>
{
    bool operator()(const xmlNsPtr &a, const xmlNsPtr &b)
    {
	return compareimpl::compare_ns(a, b) < 0;
    }
};

template<>
struct equal_to<xmlNsPtr>
{
    bool operator()(const xmlNsPtr &a, const xmlNsPtr &b)
    {
	return compareimpl::compare_ns(a, b) == 0;
    }
};

template<>
struct less<xmlAttrPtr>
{
    bool operator()(const xmlAttrPtr &a, const xmlAttrPtr &b)
    {
	return compareimpl::compare_attr(a, b) < 0;
    }
};

template<>
struct equal_to<xmlAttrPtr>
{
    bool operator()(const xmlAttrPtr &a, const xmlAttrPtr &b)
    {
	return compareimpl::compare_attr(a, b) == 0;
    }
};

}

#endif
