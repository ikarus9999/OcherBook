#include "clc/support/Debug.h"
#include "clc/support/Logger.h"

#include "ocher/ux/Pagination.h"


Pagination::Pagination() :
    m_numPages(0)
{
}

Pagination::~Pagination()
{
    flush();
}

void Pagination::flush()
{
    m_numPages = 0;
    // TODO: delete
}

void Pagination::set(unsigned int pageNum, unsigned int layoutOffset, unsigned int strOffset /* TODO attrs */)
{
    ASSERT(pageNum <= m_numPages);
    unsigned int chunk = pageNum / pagesPerChunk;
    if (pageNum == m_numPages) {
        if (chunk == m_pages.countItems()) {
            m_pages.add(new PageMapping[pagesPerChunk]);
        }
    }
    struct PageMapping *mapping = (struct PageMapping*)m_pages.ItemAtFast(chunk);
    mapping += pageNum % pagesPerChunk;
    mapping->layoutOffset = layoutOffset;
    mapping->strOffset = strOffset;
    m_numPages = pageNum + 1;
    clc::Log::debug("ocher.pagination", "set page %u breaks at layoutOffset %u strOffset %u", pageNum, layoutOffset, strOffset);
}

bool Pagination::get(unsigned int pageNum, unsigned int *layoutOffset, unsigned int *strOffset /* TODO attrs */)
{
    unsigned int chunk = pageNum / pagesPerChunk;
    if (chunk > m_pages.countItems()) {
        return false;
    }
    struct PageMapping *mapping = (struct PageMapping*)m_pages.ItemAtFast(chunk);
    mapping += pageNum % pagesPerChunk;
    *layoutOffset = mapping->layoutOffset;
    *strOffset = mapping->strOffset;
    clc::Log::debug("ocher.pagination", "found page %u breaks at layoutOffset %u strOffset %u", pageNum, *layoutOffset, *strOffset);
    return true;
}


