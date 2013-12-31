#include "xbuffer.hh"
#include <string>

XBuffer::XBuffer():
    buffer(xmlBufferCreate())
{
    if (!buffer) {
	throw std::string("cannot create buffer");
    }
}

XBuffer::~XBuffer()
{
    xmlBufferFree(buffer);
}

