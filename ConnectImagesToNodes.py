## 이미지 노드 자동 연결 스크립트
한 머티리얼에서 base color, mra, normal 키워드가 포함된 이미지 텍스쳐 노드를 자동으로 연결해 줌

import bpy

def link_texture_nodes(material):
    if not material.use_nodes:
        print(f"Material {material.name} does not use nodes.")
        return
    
    node_tree = material.node_tree
    nodes = node_tree.nodes
    links = node_tree.links

    # 필요한 노드들 찾기
    base_color_node = None
    mra_node = None
    normal_node = None
    bsdf_node = None
    bsdf_node = None

    print(f"Scanning nodes in material: {material.name}")

    for node in nodes:
        if isinstance(node, bpy.types.ShaderNodeTexImage):
            if node.image is not None:
                print(f"Found ShaderNodeTexImage: {node.name}, Image: {node.image.name}")
                if "BaseColor" in node.image.name:
                    base_color_node = node
                    print(f"Identified as BaseColor: {node.image.name}")
                elif "MRA" in node.image.name:
                    mra_node = node
                    print(f"Identified as MRA: {node.image.name}")
                elif "Normal" in node.image.name:
                    normal_node = node
                    print(f"Identified as Normal: {node.image.name}")
        elif isinstance(node, bpy.types.ShaderNodeBsdfPrincipled):
            bsdf_node = node
            print("Found Principled BSDF node")

    # 노드 연결
    if bsdf_node:
        if base_color_node:
            print(f"Linking BaseColor: {base_color_node.image.name}")
            links.new(bsdf_node.inputs['Base Color'], base_color_node.outputs['Color'])
        if mra_node:
            print(f"Linking MRA: {mra_node.image.name}")
            # Removing any existing links to prevent multiple links
            if bsdf_node.inputs['Metallic'].is_linked:
                links.remove(bsdf_node.inputs['Metallic'].links[0])
            if bsdf_node.inputs['Roughness'].is_linked:
                links.remove(bsdf_node.inputs['Roughness'].links[0])
            links.new(bsdf_node.inputs['Metallic'], mra_node.outputs['Color'])
            links.new(bsdf_node.inputs['Roughness'], mra_node.outputs['Color'])
        if normal_node:
            print(f"Linking Normal: {normal_node.image.name}")
            normal_map_node = nodes.new('ShaderNodeNormalMap')
            links.new(normal_map_node.inputs['Color'], normal_node.outputs['Color'])
            links.new(bsdf_node.inputs['Normal'], normal_map_node.outputs['Normal'])
    else:
        print(f"No Principled BSDF node found in material {material.name}.")

# 현재 활성화된 오브젝트
obj = bpy.context.object

if obj:
    print(f"Active object: {obj.name}")
    for mat_slot in obj.material_slots:
        if mat_slot.material:
            print(f"Processing material slot: {mat_slot.material.name}")
            link_texture_nodes(mat_slot.material)
        else:
            print("No material in slot.")
else:
    print("No active object with material slots found.")