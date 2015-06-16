/*
Copyright (c) 2003, Thees Christian Winkler
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the 
	  above copyright notice, this list of conditions and 
	  the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, 
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.
    * Neither the name of the <ORGANIZATION> nor the names of its contributors
      may be used to endorse or promote products derived from this software without 
	  specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
	SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
	OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
	TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
	EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INI_H_INCLUDED
#define INI_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CIni  
{
private:

	CString	m_strFileName;

public:
	BOOL	SetValue( const CString& strSection, const CString& strItem, const CString& strVal );
	BOOL	SetValue( const CString& strSection, const CString& strItem, const int iVal );
	BOOL	SetValue( const CString& strSection, const CString& strItem, const long lVal );
	BOOL	SetValue( const CString& strSection, const CString& strItem, const DWORD lVal );


	CString GetValue( const CString& strSection, const CString& strItem, CString strDefault );
	INT		GetValue( const CString& strSection, const CString& strItem, const INT nDefault );
	LONG	GetValue( const CString& strSection, const CString& strItem, const LONG nDefault );
	DWORD	GetValue( const CString& strSection, const CString& strItem, const DWORD nDefault );

	CIni( );
	virtual ~CIni();

	void SetIniFileName( const CString& strFileName ) { m_strFileName = strFileName; }
};

#endif
