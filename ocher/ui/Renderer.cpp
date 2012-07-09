#include "ocher/ui/Renderer.h"


Renderer::Renderer(clc::Buffer layout) :
    m_layout(layout)
{
}


#if 0
void Renderer::pushOp(uint16_t op)
{
}

void Renderer::pushTextAttr(TextAttr attr, uint8_t arg)
{
    pushOp((uint16_t)attr | (arg<<8));
}
#endif


