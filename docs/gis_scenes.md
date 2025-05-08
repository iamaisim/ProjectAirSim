# GIS Scenes
If you are looking to use one of our geo-specific scenes, provided that you have the needed glb/glTF tiles, you only have to make two changes.

In the **[Scene Configuration](config_scene.md)**, you will need to set the "scene-type" and "tiles-dir" parameters:
1. For GIS environments, **scene-type** should be set to **CustomGIS**.
2. [Unless you are running on Azure] Set **tiles-dir** to the local directory containing the GIS tiles.