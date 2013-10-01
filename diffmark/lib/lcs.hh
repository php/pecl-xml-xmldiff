// ported from Algorithm::Diff, which is Copyright (c) 2000-2002 Ned
// Konz.

#ifndef lcs_hh
#define lcs_hh

#include <vector>
#include "lcsimpl.hh"

// Uses std::less and std::equals_to to compare instances of TItem -
// specialize the comparators for types which cannot be compared with
// < and == .
template<typename TItem, typename TSequence = std::vector<TItem> >
class LCS
{
public:
    virtual ~LCS() { }

    void traverse_balanced(const TSequence &a, const TSequence &b);

private:
    virtual void on_match() = 0;
    virtual void on_insert(TItem n) = 0;
    virtual void on_delete(TItem m) = 0;

    void discard_a(const TSequence &a, int i);
    void discard_b(const TSequence &b, int j);
};

template<typename TItem, typename TSequence>
inline void LCS<TItem, TSequence>::discard_a(const TSequence &a, int i)
{
    on_delete(a[i]);
}

template<typename TItem, typename TSequence>
inline void LCS<TItem, TSequence>::discard_b(const TSequence &b, int j)
{
    on_insert(b[j]);
}

template<typename TItem, typename TSequence>
void LCS<TItem, TSequence>::traverse_balanced(const TSequence &a,
    const TSequence &b)
{
    lcsimpl::TSparseVector matchvector =
	lcsimpl::longest_common_subsequence<TItem, TSequence>(a, b);

    int lasta = a.size() - 1;
    int lastb = b.size() - 1;
    int ai = 0;
    int bi = 0;
    int ma = -1;
    int mb;

    int sentinel = 0;
    if (!matchvector.empty()) {
	sentinel = matchvector.rbegin()->first;
    }

    while (true) {
	do {
	    ma++; 
	} while ((ma <= sentinel) &&
	    (matchvector.find(ma) == matchvector.end()));

	if (ma > sentinel) {
	    goto second;
	}

	mb = matchvector[ma];

	while ((ai < ma) || (bi < mb)) {
	    if ((ai < ma) && (bi < mb)) {
		discard_a(a, ai++);
		discard_b(b, bi++);
	    } else if (ai < ma) {
		discard_a(a, ai++);
	    } else {
		discard_b(b, bi++);
	    }
	}

	on_match();
	++ai; ++bi;
    }

second:
    while ((ai <= lasta) || (bi <= lastb)) {
	if ((ai <= lasta) && (bi <= lastb)) {
	    discard_a(a, ai++);
	    discard_b(b, bi++);
	} else if (ai <= lasta) {
	    discard_a(a, ai++);
	} else {
	    discard_b(b, bi++);
	}
    }
}

#endif

