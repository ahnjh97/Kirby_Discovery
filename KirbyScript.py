import bpy
import os
from difflib import SequenceMatcher
import shutil
import re

# 숨기기 기능
def hide_collision_objects():
    for obj in bpy.data.objects:
        if obj.type == 'MESH' and (obj.name.startswith("Collision") or "Transparent" in obj.name):
            obj.hide_set(True)

# 텍스처 매칭 기능
def similar(a, b):
    return SequenceMatcher(None, a, b).ratio()

def find_best_texture(material_name, directory, keyword):
    best_match = None
    best_match_ratio = 0.0

    for root, dirs, files in os.walk(directory):
        for file in files:
            if len(file) < 2 or len(material_name) < 2 or file[:2] != material_name[:2]:
                continue
            if keyword in file and material_name in file:
                match_ratio = similar(material_name, file)
                if match_ratio > best_match_ratio:
                    best_match_ratio = match_ratio
                    best_match = os.path.join(root, file)

    return best_match

def change_material_name(material_name):
    # material_name이 name_changes 사전의 키에 해당하는지 확인하고, 해당하는 경우 새로운 이름을 반환
    if material_name in name_replacements:
        return name_replacements[material_name]
    # 일치하는 이름이 없는 경우, 원래의 이름을 반환
    else:
        return material_name
            
def link_textures_to_materials(src_path):
    copied_files = []
    
    # 모든 메쉬의 모든 머티리얼 노드를 초기화
    for obj in bpy.data.objects:
        if obj.type == 'MESH':
            for mat_slot in obj.material_slots:
                mat = mat_slot.material
                if mat is not None and mat.use_nodes:
                    # 노드 링크 초기화
                    mat.node_tree.links.clear()
                    # 기본 BSDF 노드 제거
                    for node in mat.node_tree.nodes:
                        mat.node_tree.nodes.remove(node)
                        
    for obj in bpy.data.objects:
        if obj.type == 'MESH' and not obj.hide_get():
            for mat_slot in obj.material_slots:
                mat = mat_slot.material
                if mat is not None and mat.use_nodes:
                    node_tree = mat.node_tree
                    
                    bsdf = None
                    material_output = None
                    for node in node_tree.nodes:
                        if node.type == 'BSDF_PRINCIPLED':
                            bsdf = node
                        elif node.type == 'OUTPUT_MATERIAL':
                            material_output = node
                        
                    if bsdf is None:
                        bsdf = node_tree.nodes.new('ShaderNodeBsdfPrincipled')
                    if material_output is None:
                        material_output = node_tree.nodes.new('ShaderNodeOutputMaterial')
                    node_tree.links.new(bsdf.outputs['BSDF'], material_output.inputs['Surface'])
                    
                    # 머티리얼 이름에서 _와 .를 제거하고 그 뒤의 숫자도 제거
                    material_name = re.sub(r'\.\d+$', '', mat.name)
                    material_name = change_material_name(material_name)
                    material_name = re.sub(r'[_\.\d]+', '', material_name)
                    material_name = change_material_name(material_name)
                    
                    base_color_texture_path = find_best_texture(material_name, src_path, "BaseColor")
                    mra_texture_path = find_best_texture(material_name, src_path, "MRA")
                    normal_texture_path = find_best_texture(material_name, src_path, "Normal")
                    textures_applied = False

                    if base_color_texture_path:
                        base_color_image = bpy.data.images.load(base_color_texture_path)
                        for node in mat.node_tree.nodes:
                            if node.type == 'TEX_IMAGE' and node.image and "Base_Color" in node.image.filepath:
                                node.image = base_color_image
                                textures_applied = True
                                break
                        else:
                            new_node = mat.node_tree.nodes.new('ShaderNodeTexImage')
                            new_node.image = base_color_image
                            mat.node_tree.links.new(bsdf.inputs['Base Color'], new_node.outputs['Color'])
                            textures_applied = True

                    if mra_texture_path:
                        mra_image = bpy.data.images.load(mra_texture_path)
                        for node in mat.node_tree.nodes:
                            if node.type == 'TEX_IMAGE' and node.image and "MRA" in node.image.filepath:
                                node.image = mra_image
                                textures_applied = True
                                break
                        else:
                            new_node = mat.node_tree.nodes.new('ShaderNodeTexImage')
                            new_node.image = mra_image
                            # MRA를 Metallic에 연결
                            mat.node_tree.links.new(bsdf.inputs['Metallic'], new_node.outputs['Color'])
                            textures_applied = True

                    if normal_texture_path:
                        normal_image = bpy.data.images.load(normal_texture_path)
                        for node in mat.node_tree.nodes:
                            if node.type == 'TEX_IMAGE' and node.image and "Normal" in node.image.filepath:
                                node.image = normal_image
                                textures_applied = True
                                break
                        else:
                            new_node = mat.node_tree.nodes.new('ShaderNodeTexImage')
                            new_node.image = normal_image
                            
                            # Normal Map 노드를 추가하여 연결
                            normal_map_node = mat.node_tree.nodes.new('ShaderNodeNormalMap')
                            mat.node_tree.links.new(normal_map_node.inputs['Color'], new_node.outputs['Color'])
                            mat.node_tree.links.new(bsdf.inputs['Normal'], normal_map_node.outputs['Normal'])
                            textures_applied = True
                            
                    if textures_applied:
                        # 텍스처가 성공적으로 적용된 경우 파일 복사
                        if not os.path.exists(dest_path):
                            os.makedirs(dest_path)
                        
                        for texture_path in [base_color_texture_path, mra_texture_path, normal_texture_path]:
                            if texture_path:
                                destination_path = os.path.join(dest_path, os.path.basename(texture_path))
                                if not os.path.exists(destination_path):
                                    shutil.copy2(texture_path, destination_path)
                                    copied_files.append(destination_path)
                                    
    return copied_files

# 실행
hide_collision_objects()
src_path = r"C:\Users\Chelsea0427\Desktop\output\KirbyTexture"  # 텍스쳐 경로
dest_path = r"C:\Users\Chelsea0427\Desktop\Kirby_Discovery\Resources\Models\NonAnim\Level1Stage1Step01" # 텍스쳐를 복사할 경로 (fbx를 저장할 경로)

name_replacements = { # 머티리얼 이름과 동일한 텍스쳐가 없을경우 다른 텍스쳐로 대체
    "GsAllBuildingTopC" : "GsAllBuildingTopTileC",
    "GsAllBuildingDirtPartsC": "GsAllBuildingCoverC",
    "GsAllBuildingSideC" : "GsAllBuildingSidePaintC",
    #"GsAllBuildingSideC_0000_0000_0000" : "GsAllBuildingSidePaintC",
    
    "GsBuildingCeilingC": "GsAllBuildingCeilingConcreteC",
    "GsBuildingMocoCoverC": "GsDefaultMocoC",
    "GsBuildingSideC" : "GsAllBuildingSidePaintC",
    "GsBuildingWallCoverC": "GsDefaultPartsC",
    "GsBuildingTopC" : "GsLandTop",
    "GsBuildingTopC_0000_0000_0000": "GsLandTop",
    "GsBuildingTopC_0100_0000_0000" : "GsAllBuildingTopTileC",
    
    "GsDefaultCeilingC": "GsDefaultSideRockC",
    "GsDefaultMocoCoverC": "GsDefaultMocoC",
    "GsDefaultSideC" : "GsDefaultSideRockC",
    "GsDefaultTopC" : "GsLandSoilC",
    "GsDefaultWallCoverC": "GsDefaultPartsC",
    "GsDefaultTopC_0000_0000_0000" : "GsLandTop"
}

copied_files = link_textures_to_materials(src_path)
print(f"Copied files: {copied_files}")