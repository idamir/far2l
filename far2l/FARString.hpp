#pragma once

/*
FARString.hpp

Unicode строка
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "local.hpp"

#define __US_DELTA 20

class FARStringData
{
	private:
		size_t m_nLength;
		size_t m_nSize;
		size_t m_nDelta;
		int m_nRefCount;
		wchar_t *m_pData;

		wchar_t *AllocData(size_t nSize, size_t *nNewSize)
		{
			if (nSize <= m_nDelta)
				*nNewSize = m_nDelta;
			else if (nSize%m_nDelta > 0)
				*nNewSize = (nSize/m_nDelta + 1) * m_nDelta;
			else
				*nNewSize = nSize;

			return new wchar_t[*nNewSize];
		}

		void FreeData(wchar_t *pData) { delete [] pData; }

	public:
		FARStringData(size_t nSize=0, size_t nDelta=0)
		{
			m_nDelta = nDelta? nDelta : __US_DELTA;
			m_nLength = 0;
			m_nRefCount = 1;
			m_pData = AllocData(nSize,&m_nSize);
			//Так как ни где выше в коде мы не готовы на случай что памяти не хватит
			//то уж лучше и здесь не проверять а сразу падать
			*m_pData = 0;
		}

		size_t SetLength(size_t nLength)
		{
			//if (nLength<m_nSize) //Эту проверку делает верхний класс, так что скажем что это оптимизация
			{
				m_nLength = nLength;
				m_pData[m_nLength] = 0;
			}
			return m_nLength;
		}

		void Inflate(size_t nSize)
		{
			if (nSize <= m_nSize)
				return;

			if (nSize >= m_nDelta << 3)
				nSize = nSize << 1;
			else
				nSize = (nSize/m_nDelta + 1) * m_nDelta;

			wchar_t *pOldData = m_pData;
			m_pData = AllocData(nSize,&m_nSize);
			//Так как ни где выше в коде мы не готовы на случай что памяти не хватит
			//то уж лучше и здесь не проверять а сразу падать
			wmemcpy(m_pData,pOldData,m_nLength);
			m_pData[m_nLength] = 0;
			FreeData(pOldData);
		}

		wchar_t *GetData() { return m_pData; }
		size_t GetLength() const { return m_nLength; }
		size_t GetSize() const { return m_nSize; }

		int GetRef() const { return m_nRefCount; }
		void AddRef() { m_nRefCount++; }
		void DecRef()
		{
			m_nRefCount--;

			if (!m_nRefCount)
				delete this;
		}

		~FARStringData() { FreeData(m_pData); }
};

class FARString
{
	private:
		FARStringData *m_pData;

		void SetEUS();

	public:

		FARString() { SetEUS(); }
		FARString(const FARString &strCopy) { SetEUS(); Copy(strCopy); }
		FARString(const wchar_t *lpwszData) { SetEUS(); Copy(lpwszData); }
		FARString(const wchar_t *lpwszData, size_t nLength) { SetEUS(); Copy(lpwszData, nLength); }
		FARString(const char *lpszData, UINT CodePage=CP_UTF8) { SetEUS(); Copy(lpszData, CodePage); }
		FARString(const std::string &strData, UINT CodePage=CP_UTF8) { SetEUS(); Copy(strData.c_str(), CodePage); }
		FARString(const std::wstring &strData) { SetEUS(); Copy(strData.c_str()); }
		FARString(size_t nSize, size_t nDelta=0) { m_pData = new FARStringData(nSize, nDelta); }

		~FARString() { /*if (m_pData) он не должен быть nullptr*/ m_pData->DecRef(); }

		void Inflate(size_t nSize);
		wchar_t *GetBuffer(size_t nSize = (size_t)-1);
		void ReleaseBuffer(size_t nLength = (size_t)-1);

		size_t GetLength() const { return m_pData->GetLength(); }
		size_t SetLength(size_t nLength);

		size_t GetSize() const { return m_pData->GetSize(); }

		wchar_t At(size_t nIndex) const { return m_pData->GetData()[nIndex]; }

		bool IsEmpty() const { return !(m_pData->GetLength() && *m_pData->GetData()); }

		size_t GetCharString(char *lpszStr, size_t nSize, UINT CodePage=CP_UTF8) const;
		std::string GetMB() const;

		int __cdecl Format(const wchar_t * format, ...);

		FARString& Replace(size_t Pos, size_t Len, const wchar_t* Data, size_t DataLen);
		FARString& Replace(size_t Pos, size_t Len, const FARString& Str) { return Replace(Pos, Len, Str.CPtr(), Str.GetLength()); }
		FARString& Replace(size_t Pos, size_t Len, const wchar_t* Str) { return Replace(Pos, Len, Str, StrLength(NullToEmpty(Str))); }
		FARString& Replace(size_t Pos, size_t Len, wchar_t Ch) { return Replace(Pos, Len, &Ch, 1); }

		FARString& Append(const wchar_t* Str, size_t StrLen) { return Replace(GetLength(), 0, Str, StrLen); }
		FARString& Append(const FARString& Str) { return Append(Str.CPtr(), Str.GetLength()); }
		FARString& Append(const wchar_t* Str) { return Append(Str, StrLength(NullToEmpty(Str))); }
		FARString& Append(wchar_t Ch) { return Append(&Ch, 1); }
		FARString& Append(const char *lpszAdd, UINT CodePage=CP_UTF8);

		FARString& Insert(size_t Pos, const wchar_t* Str, size_t StrLen) { return Replace(Pos, 0, Str, StrLen); }
		FARString& Insert(size_t Pos, const FARString& Str) { return Insert(Pos, Str.CPtr(), Str.GetLength()); }
		FARString& Insert(size_t Pos, const wchar_t* Str) { return Insert(Pos, Str, StrLength(NullToEmpty(Str))); }
		FARString& Insert(size_t Pos, wchar_t Ch) { return Insert(Pos, &Ch, 1); }

		FARString& Copy(const wchar_t *Str, size_t StrLen) { return Replace(0, GetLength(), Str, StrLen); }
		FARString& Copy(const wchar_t *Str) { return Copy(Str, StrLength(NullToEmpty(Str))); }
		FARString& Copy(wchar_t Ch) { return Copy(&Ch, 1); }
		FARString& Copy(const FARString &Str);
		FARString& Copy(const char *lpszData, UINT CodePage=CP_UTF8);

		FARString& Remove(size_t Pos, size_t Len = 1) { return Replace(Pos, Len, nullptr, 0); }
		FARString& LShift(size_t nShiftCount, size_t nStartPos=0) { return Remove(nStartPos, nShiftCount); }

		FARString& Clear();

		const wchar_t *CPtr() const { return m_pData->GetData(); }
		operator const wchar_t *() const { return m_pData->GetData(); }

		FARString SubStr(size_t Pos, size_t Len = -1);

		const FARString& operator=(const FARString &strCopy) { return Copy(strCopy); }
		const FARString& operator=(const char *lpszData) { return Copy(lpszData); }
		const FARString& operator=(const wchar_t *lpwszData) { return Copy(lpwszData); }
		const FARString& operator=(wchar_t chData) { return Copy(chData); }
		const FARString& operator=(const std::string &strSrc) { return Copy(strSrc.c_str(), CP_UTF8); }
		const FARString& operator=(const std::wstring &strSrc) { return Copy(strSrc.c_str()); }

		const FARString& operator+=(const FARString &strAdd) { return Append(strAdd); }
		const FARString& operator+=(const char *lpszAdd) { return Append(lpszAdd); }
		const FARString& operator+=(const wchar_t *lpwszAdd) { return Append(lpwszAdd); }
		const FARString& operator+=(wchar_t chAdd) { return Append(chAdd); }

		friend const FARString operator+(const FARString &strSrc1, const FARString &strSrc2);
		friend const FARString operator+(const FARString &strSrc1, const char *lpszSrc2);
		friend const FARString operator+(const FARString &strSrc1, const wchar_t *lpwszSrc2);

		bool Equal(size_t Pos, size_t Len, const wchar_t* Data, size_t DataLen) const;
		bool Equal(size_t Pos, const wchar_t* Str, size_t StrLen) const { return Equal(Pos, StrLen, Str, StrLen); }
		bool Equal(size_t Pos, const wchar_t* Str) const { return Equal(Pos, StrLength(Str), Str, StrLength(Str)); }
		bool Equal(size_t Pos, const FARString& Str) const { return Equal(Pos, Str.GetLength(), Str.CPtr(), Str.GetLength()); }
		bool Equal(size_t Pos, wchar_t Ch) const { return Equal(Pos, 1, &Ch, 1); }
		bool operator==(const FARString& Str) const { return Equal(0, GetLength(), Str.CPtr(), Str.GetLength()); }
		bool operator==(const wchar_t* Str) const { return Equal(0, GetLength(), Str, StrLength(Str)); }
		bool operator==(wchar_t Ch) const { return Equal(0, GetLength(), &Ch, 1); }

		bool operator!=(const FARString& Str) const { return !Equal(0, GetLength(), Str.CPtr(), Str.GetLength()); }
		bool operator!=(const wchar_t* Str) const { return !Equal(0, GetLength(), Str, StrLength(Str)); }
		bool operator!=(wchar_t Ch) const { return !Equal(0, GetLength(), &Ch, 1); }

		bool operator<(const FARString& Str) const;

		FARString& Lower(size_t nStartPos=0, size_t nLength=(size_t)-1);
		FARString& Upper(size_t nStartPos=0, size_t nLength=(size_t)-1);

		bool Pos(size_t &nPos, wchar_t Ch, size_t nStartPos=0) const;
		bool Pos(size_t &nPos, const wchar_t *lpwszFind, size_t nStartPos=0) const;
		bool PosI(size_t &nPos, const wchar_t *lpwszFind, size_t nStartPos=0) const;
		bool RPos(size_t &nPos, wchar_t Ch, size_t nStartPos=0) const;

		bool Contains(wchar_t Ch, size_t nStartPos=0) const { return !wcschr(m_pData->GetData()+nStartPos,Ch) ? false : true; }
		bool Contains(const wchar_t *lpwszFind, size_t nStartPos=0) const { return !wcsstr(m_pData->GetData()+nStartPos,lpwszFind) ? false : true; }

		bool Begins(wchar_t Ch) const { return m_pData->GetLength() > 0 && *m_pData->GetData() == Ch; }
		bool Begins(const wchar_t *lpwszFind) const { return m_pData->GetLength() > 0 && wcsncmp(m_pData->GetData(), lpwszFind, wcslen(lpwszFind)) == 0; }
};

//typedef FARString FARString;