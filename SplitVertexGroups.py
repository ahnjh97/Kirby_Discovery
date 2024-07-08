import bpy

def separate_by_vertex_groups():
    # 현재 활성 오브젝트를 가져옵니다.
    obj = bpy.context.active_object
    
    if obj is None or obj.type != 'MESH':
        print("선택된 메쉬가 없습니다.")
        return
    
    # 모든 버텍스 그룹을 가져옵니다.
    vertex_groups = obj.vertex_groups
    
    if not vertex_groups:
        print("버텍스 그룹이 없습니다.")
        return
    
    # 각 버텍스 그룹별로 메쉬를 분리합니다.
    for group in vertex_groups:
        # 버텍스 그룹을 활성화합니다.
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.select_all(action='DESELECT')
        bpy.ops.object.vertex_group_set_active(group=group.name)
        bpy.ops.object.vertex_group_select()
        
        # 새로운 오브젝트로 분리합니다.
        bpy.ops.mesh.separate(type='SELECTED')
        bpy.ops.object.mode_set(mode='OBJECT')
        
        # 생성된 새로운 오브젝트를 가져옵니다.
        new_obj = bpy.context.selected_objects[0]
        new_obj.name = group.name
    
    # 원래 메쉬 오브젝트를 숨깁니다.
    obj.hide_set(True)

# 스크립트를 실행합니다.
separate_by_vertex_groups()
