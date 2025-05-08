// Copyright (C) Microsoft Corporation. All rights reserved.

// Based on builtin Internal-DepthNormalsTexture.shader
// EncodeDepthNormal() is replaced with custom Output() function

Shader "Hidden/CameraEffects" {
  Properties{_MainTex("", 2D) = "white" {} _Cutoff("", Float) =
                 0.5 _Color("", Color) = (1, 1, 1, 1)

                     _ObjectColor("Object Color", Color) =
                         (1, 1, 1, 1)_CategoryColor("Catergory Color", Color) =
                             (0, 1, 0, 1)}

  SubShader {
    CGINCLUDE

    fixed4 _ObjectColor;
    fixed4 _CategoryColor;
    int _OutputMode;

    float4 Output(float depth, float3 normal) {
      /* see ImageSynthesis.cs
      enum ReplacelementModes {
              ObjectId 			= 0,
              CatergoryId			= 1,
              DepthPerspective	= 2,
              DepthMultichannel	= 3,
              Normals				= 4
      };*/

      if (_OutputMode == 0)  // ObjectId
      {
        return _ObjectColor;
      } else if (_OutputMode == 1)  // CatergoryId
      {
        return _CategoryColor;
      } else if (_OutputMode == 2)  // DepthPerspective
      {
        return float4(depth, depth, depth, 1);
      } else if (_OutputMode == 3)  // DepthMultichannel
      {
        float lowBits = frac(depth * 256);
        float highBits = depth - lowBits / 256;
        return float4(lowBits, highBits, depth, 1);
      } else if (_OutputMode == 4)  // Normals
      {
        // [-1 .. 1] => [0 .. 1]
        float3 c = normal * 0.5 + 0.5;
        return float4(c, 1);
      }

      // unsupported _OutputMode
      return float4(1, 0.5, 0.5, 1);
    }
    ENDCG

    // Support for different RenderTypes
    // The following code is based on builtin
    // Internal-DepthNormalsTexture.shader

    Tags{"RenderType" = "Opaque"} Pass {
      CGPROGRAM
      #pragma vertex vert
      #pragma fragment frag
      #include "UnityCG.cginc"
      struct v2f {
        float4 pos : SV_POSITION;
        float4 nz : TEXCOORD0;
        UNITY_VERTEX_OUTPUT_STEREO
      };
      v2f vert(appdata_base v) {
        v2f o;
        UNITY_SETUP_INSTANCE_ID(v);
        UNITY_INITIALIZE_VERTEX_OUTPUT_STEREO(o);
        o.pos = UnityObjectToClipPos(v.vertex);
        o.nz.xyz = COMPUTE_VIEW_NORMAL;
        COMPUTE_EYEDEPTH(o.nz.w);
        return o;
      }
      float4 frag(v2f i) : SV_Target { return Output(i.nz.w, i.nz.xyz); }
      ENDCG
    }
  }

  SubShader {
    Tags{"RenderType" = "TransparentCutout"} Pass {
      CGPROGRAM
      #pragma vertex vert
      #pragma fragment frag
      #include "UnityCG.cginc"
      struct v2f {
        float4 pos : SV_POSITION;
        float2 uv : TEXCOORD0;
        float4 nz : TEXCOORD1;
        UNITY_VERTEX_OUTPUT_STEREO
      };
      uniform float4 _MainTex_ST;
      v2f vert(appdata_base v) {
        v2f o;
        UNITY_SETUP_INSTANCE_ID(v);
        UNITY_INITIALIZE_VERTEX_OUTPUT_STEREO(o);
        o.pos = UnityObjectToClipPos(v.vertex);
        o.uv = TRANSFORM_TEX(v.texcoord, _MainTex);
        o.nz.xyz = COMPUTE_VIEW_NORMAL;
        COMPUTE_EYEDEPTH(o.nz.w);
        return o;
      }
      uniform sampler2D _MainTex;
      uniform fixed _Cutoff;
      uniform fixed4 _Color;
      float4 frag(v2f i) : SV_Target {
        fixed4 texcol = tex2D(_MainTex, i.uv);
        clip(texcol.a * _Color.a - _Cutoff);
        return Output(i.nz.w, i.nz.xyz);
      }
      ENDCG
    }
  }

  Fallback Off
}
