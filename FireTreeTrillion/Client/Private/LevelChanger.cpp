#include "stdafx.h"
#include "LevelChanger.h"

#include "Level_Loading.h"
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
    data["HP"]   = _LevelData.fKirbyHP;
    data["Coin"] = _LevelData.fKirbyCoin;

    data["KirbyState"]   = _LevelData.iKirbyState;
    data["LatestLevel"]  = _LevelData.iLatestLevel;
    data["LastPosition"] = { _LevelData.vLastPos.x, _LevelData.vLastPos.y, _LevelData.vLastPos.z };

    string file_path = "../Bin/Resources/Data/LevelChangerData.json";
    ofstream output_file(file_path); // JSON 파일 쓰기
    if (!output_file.is_open()) 
    {
        MSG_BOX(TEXT("파일을 열 수 없습니다: Save"));
        return;
    }
    
    // JSON 데이터를 파일에 저장
    output_file << data.dump(4); // .dump(4) == JSON을 예쁘게 출력하기 위한 들여쓰기 설정
    output_file.close();
}

CLevelChanger::LEVEL_DATA CLevelChanger::Load()
{
    LEVEL_DATA tLevelData{};
    string file_path = "../Bin/Resources/Data/LevelChangerData.json";

    // JSON 파일 열기
    ifstream input_file(file_path);
    if (!input_file.is_open()) 
    {
        MSG_BOX(TEXT("파일을 열 수 없습니다: Load"));
        return tLevelData;
    }

    // JSON 데이터 읽기
    json data;
    input_file >> data;

    // 기본값 설정
    tLevelData.strObjectName = data.value("name", "");
    tLevelData.fKirbyHP      = data.value("HP", 0.0f);
    tLevelData.fKirbyCoin    = data.value("Coin", 0.0f);

    // 커비 모델 및 이동값에 대한 처리
    tLevelData.iKirbyState   = data.value("KirbyState", 0);
    tLevelData.iLatestLevel  = data.value("LatestLevel", 0);
    tLevelData.vLastPos      = { data["LastPosition"][0], data["LastPosition"][1], data["LastPosition"][2] };

    input_file.close();
    return tLevelData;
}

void CLevelChanger::Free()
{
	__super::Free();
}

