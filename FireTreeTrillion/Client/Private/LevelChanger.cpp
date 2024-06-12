#include "stdafx.h"
#include "LevelChanger.h"

IMPLEMENT_SINGLETON(CLevelChanger)

CLevelChanger::CLevelChanger()
{
}

void CLevelChanger::Save()
{
    //JSON 데이터 생성
    json data;
    data["name"] = "John";
    data["age"] = 30;
    data["city"] = "New York";
    data["skells"] = { "Python", "C++", "Java" };
    data["update"];
    // JSON 파일 경로
    std::string file_path = "data.json";

    // JSON 파일 쓰기
    std::ofstream output_file(file_path);
    if (!output_file.is_open()) {
        std::cerr << "파일을 열 수 없습니다: " << file_path << std::endl;
        return;
    }

    // JSON 데이터를 파일에 저장
    output_file << data.dump(4); // .dump(4)는 JSON을 예쁘게 출력하기 위한 들여쓰기 설정입니다.

    output_file.close();
    std::cout << "JSON 파일이 생성되었습니다: " << file_path << std::endl;
}

void CLevelChanger::Load()
{
    std::string file_path = "data.json";

    // JSON 파일 열기
    std::ifstream input_file(file_path);
    if (!input_file.is_open()) {
        std::cerr << "파일을 열 수 없습니다: " << file_path << std::endl;
        return;
    }

    // JSON 데이터 읽기
    json data;
    input_file >> data;

    if(data.contains("city"))
        string city = data["city"];

    int age = data["age"];
    // JSON 데이터 출력
    if (data.contains("skells"))
    {
        auto arr = data["skells"];
        for (auto it : arr)
        {
            cout << it << endl;
        }
    }
    input_file.close();
}

void CLevelChanger::Release_LevelChanger()
{
	CGameInstance::Get_Instance()->Free();
	Destroy_Instance();
}

void CLevelChanger::Free()
{
	__super::Free();
}

