# Experimental Unity Host Reference

## Status

The Unity integration in this directory is experimental, unmaintained reference
code. IAMAI does not currently validate, package, release, or provide support
for it. It is not part of the Project AirSim supported-platform matrix, and
compatibility with the current Project AirSim `main` branch or current Unity
versions is not guaranteed.

The checked-in example project records Unity Editor version `2020.3.36f1`.

## Purpose

This code is retained because it demonstrates how the engine-independent
Project AirSim simulation libraries can be hosted by a 3D engine other than
Unreal Engine. It is useful as an architectural reference for evaluating or
developing additional simulation hosts.

## Contents

- `BlocksUnity/`: Unity example project, scenes, assets, and C# host code.
- `sim_unity_wrapper/`: native wrapper that exposes Project AirSim simulation
  functionality to the Unity host.

This directory should not be treated as a production-ready Unity package or as
evidence of feature parity with the maintained Unreal Engine host.

---

Copyright (C) 2026 IAMAI CONSULTING CORP

MIT License
