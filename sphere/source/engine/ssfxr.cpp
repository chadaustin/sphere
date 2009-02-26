#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include "ssfxr.hpp"
#include "../common/minmax.hpp"

////////////////////////////////////////////////////////////////////////////////

SSFXR::SSFXR()
{
    m_Sfxr.ResetParams();
}

////////////////////////////////////////////////////////////////////////////////

SSFXR::~SSFXR()
{
    Destroy();
}

////////////////////////////////////////////////////////////////////////////////

//bool
//SSFXR::Define(const char* filename, IFileSystem& fs)
//{
//    return Initialize();
//}

////////////////////////////////////////////////////////////////////////////////

void
SSFXR::Destroy()
{
	//delete m_Sfxr;
	//m_Sfxr = NULL;
	m_Sfxr.~sSfxr();
}

////////////////////////////////////////////////////////////////////////////////

SSFXR*
SSFXR::Clone()
{
    SSFXR* sfxr_new = new SSFXR;
    if (sfxr_new)
    {
		// TODO: Copy the parameters
    }
    return sfxr_new;
}

////////////////////////////////////////////////////////////////////////////////

bool
SSFXR::Save(const char* filename)
{
    return m_Sfxr.SaveWav(filename);
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

bool
SSFXR::Reset()
{
	m_Sfxr.ResetParams();
    return true;
}

////////////////////////////////////////////////////////////////////////////////
