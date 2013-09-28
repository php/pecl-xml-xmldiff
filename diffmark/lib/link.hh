#ifndef link_hh
#define link_hh

class LinkBody;

class Link
{
public:
    Link();
    Link(const Link *n, int f, int t);
    Link(const Link &other);
    Link &operator=(const Link &other);
    ~Link();

    const Link *next() const;
    int from() const;
    int to() const;

private:
    LinkBody *body;
};

class LinkBody
{
public:
    LinkBody():next(0), from(0), to(0), refCount(1) { }
    LinkBody(const Link *n, int f, int t):
        next(dupl(n)), from(f), to(t), refCount(1) { }
    ~LinkBody() { delete next; }

    const Link *next;
    const int from;
    const int to;

    void addRef() { ++refCount; }
    void delRef();

private:
    int refCount;

    static Link *dupl(const Link *n) { return n ? new Link(*n) : 0; }
};

inline Link::Link():
    body(new LinkBody())
{
}

inline Link::Link(const Link *n, int f, int t):
    body(new LinkBody(n, f, t))
{
}

inline Link::Link(const Link &other):
    body(other.body)
{
    body->addRef();
}

inline Link::~Link()
{
    body->delRef();
}

inline const Link *Link::next() const
{
    return body->next;
}

inline int Link::from() const
{
    return body->from;
}

inline int Link::to() const
{
    return body->to;
}

#endif

