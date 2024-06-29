#include "stdafx.h"
#include "LevelChanger.h"

#include "Kirby.h"

IMPLEMENT_SINGLETON(CLevelChanger)

CLevelChanger::CLevelChanger()
{
}

//JSON 데이터 생성
void CLevelChanger::Save(LEVEL_DATA _LevelData)
{
    json data;
    data["name"] = "Kirby";    
    data["Coin"] = _LevelData.fKirbyCoin;
    data["HP"]   = _LevelData.fKirbyHP;

    string file_path = "LevelChangerData.json";
    ofstream output_file(file_path); // JSON 파일 쓰기
    if (!output_file.is_open()) 
    {
        MSG_BOX(TEXT("파일을 열 수 없습니다: "));
        return;
    }

    // JSON 데이터를 파일에 저장
    output_file << data.dump(4); // .dump(4) == JSON을 예쁘게 출력하기 위한 들여쓰기 설정
    output_file.close();
    // MSG_BOX(TEXT("JSON 파일이 생성되었습니다: "));
}

CLevelChanger::LEVEL_DATA CLevelChanger::Load()
{
    LEVEL_DATA tLevelData{};
    string file_path = "LevelChangerData.json";

    // JSON 파일 열기
    ifstream input_file(file_path);
    if (!input_file.is_open()) 
    {
        MSG_BOX(TEXT("파일을 열 수 없습니다: "));
        return tLevelData;
    }

    // JSON 데이터 읽기
    json data;
    input_file >> data;
    tLevelData.strObjectName = data["name"];
    tLevelData.fKirbyHP = stof(to_string(data["HP"]));
    tLevelData.fKirbyCoin = stof(to_string(data["Coin"]));

    input_file.close();
    return tLevelData;
}

void CLevelChanger::Free()
{
	__super::Free();
}

