#ifndef LIBCLC_TEXT_EDITOR_H
#define LIBCLC_TEXT_EDITOR_H

#include "clc/data/DynamicBuffer.h"

namespace clc
{

/**
 */
class TextEditor
{
public:

	// access:
	//  current char
	//  next line
	//  prev line
	//  first
	//  last
	//
	// movement
	// set position
	// set line, col
	//
	// get range
	// del range
	// set range
	//
	// position
	//
	// insert char
	// insert buffer
	// 
	// delete char
	//
protected:
	DynamicBuffer m_buf;
};

}

#endif

