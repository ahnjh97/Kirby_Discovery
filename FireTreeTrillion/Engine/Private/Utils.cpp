#include "Utils.h"

// string --> wstring 변환
wstring CUtils::StrToWstr(const string& value)
{
	wstring tag;
	USES_CONVERSION;
	tag = std::wstring(A2W(value.c_str()));
	return tag;
}

// wstring --> string 변환
string CUtils::WstrToStr(const wstring& value)
{
	string stemp;
	USES_CONVERSION;
	stemp = std::string(W2A(value.c_str()));
	return stemp;
}

_int CUtils::Make_RandomInt(_int min, _int max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<_int> dis(min, max);
	return dis(gen);
}

_float CUtils::Make_RandomFloat(_float min, _float max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<_float> dis(min, max);

	return dis(gen);
}
