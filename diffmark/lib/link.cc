#include "link.hh"

void LinkBody::delRef()
{
    if (!--refCount)
    {
	delete this;
    }
}

Link &Link::operator=(const Link &other)
{
    other.body->addRef();
    body->delRef();
    body = other.body;
    return *this;
}
