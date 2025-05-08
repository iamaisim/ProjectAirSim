# Environment Objects Configuration Settings

{# include enable_internal_docs.tpp #}
Environment objects in Project AirSim are elements within the simulation that enhance visual realism without directly participating in the simulation's core tasks. These objects include various elements such as Niagara particle systems or Particle Components.

Environment objects: 
    - Particles: **[Particle configuration overview settings](#particle-configuration-overview)**

The Particles are only configured by a **link**.

## Particle configuration overview

Environment Particle Effects are dynamic visual effects used to enhance the realism and ambiance of a scene. Particularly in ProjectAirSim, these particles can simulate a wide range of natural and artificial phenomena, such as fire and smoke, contributing to the environment and immersion of the virtual world.
These environment objects are nor affected by gravity or physics in general.

`env_particle_effect_nfire_02.jsonc`
``` json
  {
    "visual": {...}
  }

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `visual` | **[Visual settings](#visual-settings-particles)** | Visual settings for the link's rendered mesh. |

### Visual settings particles

The link's visual settings consist of what visual element will be rendered for the link.

``` json
"visual": {
  "geometry": {
    "type": "unreal_mesh",
    "name": "/ProjectAirSim/FireSmokeAnims/M5VFXVOL2/Niagara/Fire/NFire_06",
    "scale": "3.0 3.0 3.0"
  }
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `geometry`: `type` | `unreal_mesh` | The type of mesh geometry to use. The mesh must be imported into the Unreal environment as a .uasset to be available at runtime. |
| `geometry`: `name` | string | For `unreal_mesh`, this should be the Unreal content path to the mesh in the environment. **Note:** Make sure this is a Niagara Particle Component or a Cascade Particle System uasset, otherwise it will not show on the screen. And that the Pivot is located at the base of the component, otherwise the BoundingBox won't set
correctly.|
| `geometry`: `scale` | string of 3 floats | Adjust the mesh's scale. Defaults to all 1.0 if not specified. |

---

Copyright (C) Microsoft Corporation.  All rights reserved