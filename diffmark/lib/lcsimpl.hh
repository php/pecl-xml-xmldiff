#ifndef lcsimpl_hh
#define lcsimpl_hh

#include <map>
#include <queue>
#include <string>
#include <iostream>
#include "link.hh"

// #define DEBUG_lcsimpl

namespace lcsimpl {

using std::cout; // for debugging

typedef std::deque<int> TPosMapValue;
typedef std::map<int, int> TSparseVector;
typedef std::map<int, Link> TLinkMap;

// see _withPositionsOfInInterval
template<typename TItem, typename TSequence>
std::map<TItem, TPosMapValue> get_eq_pos(const TSequence &s,
    int start,
    int end)
{
    typedef std::map<TItem, TPosMapValue> TPosMap;

    TPosMap d;

    for (int index = start; index <= end; ++index) {
	TItem key = s[index];
	typename TPosMap::iterator iter = d.find(key);
	if (iter == d.end()) {
	    TPosMapValue value;
	    value.push_front(index);
	    d.insert(typename TPosMap::value_type(key, value));
	} else {
	    iter->second.push_front(index);
	}
    }

    return d;
}

// see _replaceNextLargerWith
template<typename TItem, typename TSequence>
int replace_next(TSparseVector &array, int avalue, int high)
{
    // cout << "replace_next(" << avalue << ", " << high << ')';

    if (!high) {
	high = array.empty() ? -1 : array.rbegin()->first;
    }

    if ((high == -1) || (avalue > array.rbegin()->second)) {
	array.insert(TSparseVector::value_type(high + 1, avalue));
	// cout << " returns " << high + 1 << endl;
	return high + 1;
    }

    int low = 0;
    while (low <= high) {
	int index = (high + low) / 2;
	int found = array[index];

	if (avalue == found) {
	    return -1;
	} else if (avalue > found) {
	    low = index + 1;
	} else {
	    high = index - 1;
	}
    }

    array[low] = avalue;
    // cout << " returns " << low << endl;
    return low;
}

// see _longestCommonSubsequence
template<typename TItem, typename TSequence>
TSparseVector longest_common_subsequence(
    const TSequence &a,
    const TSequence &b)
{
#ifdef DEBUG_lcsimpl
    cout << "enter longest_common_subsequence(" << a.size() <<
        ", " << b.size() << ")\n";
#endif

    typedef std::map<TItem, TPosMapValue> TPosMap;
    std::equal_to<TItem> is_equal;

    int astart = 0;
    int afinish = a.size() - 1;
    int bstart = 0;
    int bfinish = b.size() - 1;

    TSparseVector matchvector;

    while ((astart <= afinish) &&
	(bstart <= bfinish) &&
	is_equal(a[astart], b[bstart])) {
	matchvector[astart++] = bstart++;
    }

    while ((astart <= afinish) &&
	(bstart <= bfinish) &&
	is_equal(a[afinish], b[bfinish])) {
	matchvector[afinish--] = bfinish--;
    }

    TPosMap bmatches = get_eq_pos<TItem, TSequence>(b,
	bstart,
	bfinish);
#ifdef DEBUG_lcsimpl
    cout << "bmatches:\n";
    for (typename TPosMap::iterator i = bmatches.begin();
	i != bmatches.end();
	++i) {
        std::string pref = "[";
        cout << i->first << " => ";
	for (TPosMapValue::const_iterator j = i->second.begin();
	    j != i->second.end();
	    ++j) {
	    cout << pref << *j;
	    pref = ", ";
	}
	cout << "]\n";
    }
    cout << "\n";
#endif

    TSparseVector thresh;
    TLinkMap links;

    // std::cout << "astart = " << astart << ", afinish = " << afinish << endl;

    for (int i = astart; i <= afinish; ++i) {
#ifdef DEBUG_lcsimpl
        cout << "i = " << i << "\n";
#endif
	TItem ai = a[i];
	typename TPosMap::iterator iter = bmatches.find(ai);
	if (iter != bmatches.end()) {
	    int k = 0;
	    for (TPosMapValue::const_iterator j = iter->second.begin();
		j != iter->second.end();
		++j) {
#ifdef DEBUG_lcsimpl
  	        cout << "*j = " << *j << ", k = " << k << '\n';
#endif
		if ((k > 0) &&
		    (thresh[k] > *j) &&
		    (thresh[k - 1] < *j)) {
		    thresh[k] = *j;
		} else {
		    k = replace_next<TItem, TSequence>(thresh, *j, k);
		}

#ifdef DEBUG_lcsimpl
		cout << "thresh:\n";
		for (typename TSparseVector::iterator m = thresh.begin();
		     m != thresh.end();
		     ++m) {
  	  	    cout << m->first << " => " << m->second << "\n";
		}
		cout << "\n";
#endif

		if (k >= 0) {
		    links[k] = Link(k ? &(links[k - 1]) : 0, i, *j);
		}

#ifdef DEBUG_lcsimpl
		cout << "links:\n";
		for (typename TLinkMap::iterator m = links.begin();
		     m != links.end();
		     ++m) {
		    cout << m->first << " => {" << m->second.next <<
		        ", " << m->second.from <<
		        ", " << m->second.to << "}\n";
		}
		cout << "\n";
#endif
	    }
	}
    }

    if (!thresh.empty()) {
	for (const Link *link = &(links[thresh.rbegin()->first]);
	    link != 0;
	    link = link->next()) {
	    matchvector[link->from()] = link->to();
	}
    }

    return matchvector;
}

} // end namespace

#endif
