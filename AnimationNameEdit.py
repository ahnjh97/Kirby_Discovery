import bpy
import os

# 블렌더 파일이 있는 디렉토리 경로
directory_path = "/path/to/your/blender/files"

# 디렉토리 내 블렌더 파일 순회
for filename in os.listdir(directory_path):
    if filename.endswith(".blend"):
        file_path = os.path.join(directory_path, filename)
        
        # 블렌더 파일 열기
        bpy.ops.wm.open_mainfile(filepath=file_path)
        
        # 애니메이션 데이터 순회
        for action in bpy.data.actions:
            for fcurve in action.fcurves:
                # fcurve가 연결된 데이터 경로
                data_path = fcurve.data_path
                # 데이터 경로에서 오브젝트 이름 추출
                obj_name = data_path.split('"')[1]
                obj = bpy.data.objects.get(obj_name)
                
                if obj is not None:
                    # 오브젝트 이름에서 '|'로 분리된 부분 중 마지막 단어만 남김
                    if '|' in obj.name:
                        new_name = obj.name.split('|')[-1]
                        obj.name = new_name
                        print(f"Renamed {filename}: {obj.name}")
        
        # 파일 저장
        bpy.ops.wm.save_mainfile(filepath=file_path)

print("Finished processing all Blender files.")
