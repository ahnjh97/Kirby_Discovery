import bpy
import numpy as np

# 이미지 데이터를 순회하며 RGB 반전
for img in bpy.data.images:
    if img.is_float:  # 이미지가 float 형식인지 확인
        pixels = np.array(img.pixels)
    else:
        pixels = np.array(img.pixels) * 255  # 0-255 사이의 값으로 변환
    
    # RGBA 형태로 재구성
    pixels = pixels.reshape((-1, 4))
    
    # Blue 채널 값이 전부 0인지 확인
    if np.all(pixels[:, 2] == 0):  # Blue 채널이 0인지 확인
        # 이미지의 색상 채널만 반전. 알파 채널은 그대로 유지
        pixels[:, :3] = 255 - pixels[:, :3]  # RGB 채널 반전
        
        if img.is_float:
            img.pixels = pixels.flatten().tolist()
        else:
            img.pixels = (pixels / 255).flatten().tolist()  # 원래 형태로 변환 후 저장
        
        img.update()
        print(f"Inverted colors for {img.name}")
    else:
        print(f"Skipped {img.name} as it does not meet the Blue channel condition")