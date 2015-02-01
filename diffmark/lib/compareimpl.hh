#ifndef compareimpl_hh
#define compareimpl_hh

#include <set>
#include <string.h>
#include <assert.h>

namespace compareimpl {

bool have_nulls(const void *p, const void *q, int &res);

int compare_ns(xmlNsPtr a, xmlNsPtr b);

template<typename TPtr>
int compare_name(TPtr p, TPtr q)
{
    assert(p->name);
    assert(q->name);

    // 9Aug2003: There's also xmlStrcmp(), but the library routine has
    // a pretty good chance of being faster than straight C...
    int name = strcmp(reinterpret_cast<const char *>(p->name),
	reinterpret_cast<const char *>(q->name));
    if (name) {
	return name;
    }

    int res;
    if (have_nulls(p->ns, q->ns, res)) {
	return res;
    } else {
	return compare_ns(p->ns, q->ns);
    }
}

template<typename TPtr>
std::set<TPtr> get_set(TPtr p)
{
    std::set<TPtr> out;

    while (p) {
	out.insert(p);
	p = p->next;
    }

    return out;
}

template<typename TPtr>
int compare_set(TPtr p, TPtr q)
{
    std::set<TPtr> a = get_set<TPtr>(p);
    std::set<TPtr> b = get_set<TPtr>(q);

    std::equal_to<TPtr> is_equal;
    std::less<TPtr> is_less;

    typename std::set<TPtr>::const_iterator i = a.begin();
    typename std::set<TPtr>::const_iterator j = b.begin();

    while ((i != a.end()) && (j != b.end())) {
	if (!is_equal(*i, *j)) {
	    return is_less(*i, *j) ? -1 : 1;
	}

	++i;
	++j;
    }

    if (i == a.end()) {
	return j != b.end() ? -1 : 0;
    } else {
	return 1;
    }
}

}

#endif
