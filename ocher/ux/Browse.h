#ifndef OCHER_UX_BROWSE_H
#define OCHER_UX_BROWSE_H

class Renderer;

class Browse
{
public:
    Browse() {}
    virtual ~Browse() {}

    virtual bool init() { return true; }
    // TODO:  instead return some epub meta record and/or requested action
    virtual void browse() = 0;
    virtual void read(Renderer& renderer) = 0;
};


#endif

