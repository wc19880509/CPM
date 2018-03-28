/*
Copyright (C) Chen Wang

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef _PROJECT_H_
#define _PROJECT_H_

#include <stdio.h>
#include <vector>

template <class T>
void _Release1DBuffer(T* pBuffer)
{
	if(pBuffer != NULL)
		delete []pBuffer;
	pBuffer = NULL;
}

template <class T>
void _Rlease2DBuffer(T** pBuffer,size_t nElements)
{
	for(size_t i = 0; i < nElements; i++)
		delete [](pBuffer[i]);
	delete []pBuffer;
	pBuffer = NULL;
}

#ifdef _MATLAB
#include "mex.h"
#endif


#ifndef WIN32

#define strcmpi strcasecmp

template <class T1,class T2>
T1 __min(T1 a, T2 b)
{
  return (a > b) ? b : a;
}

template <class T1,class T2>
T1 __max(T1 a, T2 b)
{
  return (a < b) ? b : a;
}

#endif	//WIN32

#endif	//_PROJECT_H_