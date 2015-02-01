#ifndef xbuffer_hh
#define xbuffer_hh

#include <libxml/tree.h>

class XBuffer
{
public:
    XBuffer();
    ~XBuffer();

    operator xmlBufferPtr() { return buffer; }

private:
    XBuffer(const XBuffer &other);
    XBuffer &operator=(const XBuffer &other);

    xmlBufferPtr buffer;
};

#endif
