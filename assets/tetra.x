xof 0303txt 0032

template XSkinMeshHeader {
  <3cf169ce-ff7c-44ab-93c0-f78f62d172e2>
  WORD nMaxSkinWeightsPerVertex;
  WORD nMaxSkinWeightsPerFace;
  WORD nBones;
}

template SkinWeights {
  <6f0d123b-bad2-4167-a0d0-80224f25fabb>
  STRING transformNodeName;
  DWORD nWeights;
  array DWORD vertexIndices[nWeights];
  array float weights[nWeights];
  Matrix4x4 matrixOffset;
}

Frame Root {
  FrameTransformMatrix {
     1.000000, 0.000000, 0.000000, 0.000000,
     0.000000, 1.000000, 0.000000, 0.000000,
     0.000000, 0.000000, 1.000000, 0.000000,
     0.000000, 0.000000, 0.000000, 1.000000;;
  }
  Frame Cone {
    FrameTransformMatrix {
       1.000000, 0.000000, 0.000000, 0.000000,
       0.000000, 1.000000, 0.000000, 0.000000,
       0.000000, 0.000000, 1.000000, 0.000000,
       0.000000, 0.000000, 0.000000, 1.000000;;
    }
    Mesh { // Cone mesh
      12;
       0.866025;-0.500000;-1.000000;,
       0.000000; 0.000000; 1.000000;,
      -0.866025;-0.500000;-1.000000;,
       0.000000; 1.000000;-1.000000;,
       0.000000; 0.000000; 1.000000;,
       0.866025;-0.500000;-1.000000;,
      -0.866025;-0.500000;-1.000000;,
       0.000000; 0.000000; 1.000000;,
       0.000000; 1.000000;-1.000000;,
       0.000000; 1.000000;-1.000000;,
       0.866025;-0.500000;-1.000000;,
      -0.866025;-0.500000;-1.000000;;
      4;
      3;0,1,2;,
      3;3,4,5;,
      3;6,7,8;,
      3;9,10,11;;
      MeshNormals { // Cone normals
        4;
         0.000000; 0.970143;-0.242536;,
        -0.840168;-0.485071;-0.242536;,
         0.840168;-0.485071;-0.242536;,
        -0.000000;-0.000000; 1.000000;;
        4;
        3;0,0,0;,
        3;1,1,1;,
        3;2,2,2;,
        3;3,3,3;;
      } // End of Cone normals
      MeshTextureCoords { // Cone UV coordinates
        12;
         0.000000; 1.000000;,
         1.000000; 1.000000;,
         1.000000; 0.000000;,
         0.000000; 1.000000;,
         1.000000; 1.000000;,
         1.000000; 0.000000;,
         0.000000; 1.000000;,
         1.000000; 1.000000;,
         1.000000; 0.000000;,
         0.000000; 1.000000;,
         1.000000; 1.000000;,
         1.000000; 0.000000;;
      } // End of Cone UV coordinates
    } // End of Cone mesh
  } // End of Cone
} // End of Root
