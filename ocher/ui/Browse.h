#ifndef OCHER_BROWSE_H
#define OCHER_BROWSE_H

class Browse
{
public:
    Browse() {}
    virtual ~Browse() {}

    // TODO:  instead return some epub meta record and/or requested action
    virtual void browse() = 0;
};


#endif

