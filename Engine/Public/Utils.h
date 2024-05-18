#pragma once
#include "Base.h"

BEGIN(Engine)

class ENGINE_DLL CUtils
{
public:
	static wstring  StrToWstr(const string& value);
	static string   WstrToStr(const wstring& value);

	static _int		Make_RandomInt(_int min, _int max);
	static _float	Make_RandomFloat(_float min, _float max);

};

END