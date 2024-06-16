import bpy

# 현재 블렌더 파일에 있는 모든 액션 순회
for action in bpy.data.actions:
    # 액션 이름을 '|'로 분리한 후, 마지막 부분만 가져오기
    new_name = action.name.split('|')[-1]
    
    # 새 이름을 액션에 할당
    action.name = new_name

print("모든 액션의 이름이 업데이트되었습니다.")
