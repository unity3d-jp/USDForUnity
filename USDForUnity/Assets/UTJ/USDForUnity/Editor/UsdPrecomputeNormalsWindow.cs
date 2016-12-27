using System;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    public class UsdPrecomputeNormalsWindow : EditorWindow
    {
        public UsdStream m_stream;
        public UsdMesh m_mesh;
        static bool s_genTangents = true;
        static bool s_overwrite = false;

        public static void Open(UsdStream stream)
        {
            var window = (UsdPrecomputeNormalsWindow)EditorWindow.GetWindow(typeof(UsdPrecomputeNormalsWindow));
            window.titleContent = new GUIContent("Precompute");
            window.m_stream = stream;
            window.m_mesh = null;
            window.Show();
        }

        public static void Open(UsdMesh stream)
        {
            var window = (UsdPrecomputeNormalsWindow)EditorWindow.GetWindow(typeof(UsdPrecomputeNormalsWindow));
            window.titleContent = new GUIContent("Precompute");
            window.m_stream = null;
            window.m_mesh = stream;
            window.Show();
        }

        void Generate()
        {
            UsdStream stream = null;

            int ndone = 0;

            if (m_stream != null)
            {
                stream = m_stream;
                var meshes = new List<usdi.Mesh>();
                var results = new List<bool>();
                usdi.usdiPreComputeNormalsAll(m_stream.usdiContext, s_genTangents, s_overwrite, (usdi.Mesh mesh, Bool done)=> {
                    meshes.Add(mesh);
                    results.Add(done);
                    if(done)
                    {
                        ++ndone;
                    }
                });
                for (int i = 0; i < meshes.Count; ++i)
                {
                    var mesh = meshes[i];
                    if(results[i])
                    {
                        Debug.Log("Precompute done: " + usdi.usdiPrimGetPathS(mesh));
                    }
                    else
                    {
                        Debug.Log("Precompute skipped: " + usdi.usdiPrimGetPathS(mesh));
                    }
                }
            }
            else if(m_mesh != null)
            {
                stream = m_mesh.stream;
                var mesh = m_mesh.nativeMeshPtr;
                var ret = usdi.usdiMeshPreComputeNormals(mesh, s_genTangents, s_overwrite);
                if (ret)
                {
                    Debug.Log("Precompute done: " + usdi.usdiPrimGetPathS(mesh));
                    ++ndone;
                }
                else
                {
                    Debug.Log("Precompute skipped: " + usdi.usdiPrimGetPathS(mesh));
                }
            }

            if (stream != null && ndone > 0)
            {
                stream.usdiSave();
                stream.usdiReload();
            }
        }

        void OnGUI()
        {
            if (m_stream != null)
            {
                EditorGUILayout.LabelField("Precompute normals / tangents of all meshes.");
            }
            else if (m_mesh != null)
            {
                EditorGUILayout.LabelField("Precompute Generate normals / tangents of " + m_mesh.primPath + ".");
            }
            EditorGUILayout.LabelField("Precompute tangents requires UVs and may take some time to complete.");

            EditorGUILayout.Space();

            s_genTangents = EditorGUILayout.Toggle("Generate Tangents", s_genTangents);
            s_overwrite = EditorGUILayout.Toggle("Overwrite Existing Data", s_overwrite);

            GUILayout.Space(10.0f);

            if (GUILayout.Button("Generate"))
            {
                Generate();
                Close();
            }
        }

    }
}
