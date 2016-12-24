using System;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    public class usdiPrecomputeNormalsWindow : EditorWindow
    {
        public usdiStream m_stream;
        public usdiMesh m_mesh;
        bool m_genTangents = true;
        bool m_overwrite = false;

        public static void Open(usdiStream stream)
        {
            var window = (usdiPrecomputeNormalsWindow)EditorWindow.GetWindow(typeof(usdiPrecomputeNormalsWindow));
            window.titleContent = new GUIContent("Precompute Normals");
            window.m_stream = stream;
            window.m_mesh = null;
            window.Show();
        }

        public static void Open(usdiMesh stream)
        {
            var window = (usdiPrecomputeNormalsWindow)EditorWindow.GetWindow(typeof(usdiPrecomputeNormalsWindow));
            window.titleContent = new GUIContent("Precompute Normals");
            window.m_stream = null;
            window.m_mesh = stream;
            window.Show();
        }

        void Generate()
        {
            usdiStream stream = null;

            int ndone = 0;
            Action<usdi.Mesh> body = (usdi.Mesh mesh) => {
                if (mesh && !usdi.usdiPrimIsMaster(mesh) && !usdi.usdiPrimIsInMaster(mesh) && !usdi.usdiPrimIsInstance(mesh))
                {
                    usdi.usdiMeshPreComputeNormals(mesh, m_genTangents, m_overwrite);
                    Debug.Log("Precompute done: " + usdi.usdiPrimGetPathS(mesh));
                    ++ndone;
                }
            };

            if (m_stream != null)
            {
                stream = m_stream;
                usdi.usdiEachSchemas(m_stream.usdiContext, (s) =>
                {
                    // todo: show progress
                    body.Invoke(usdi.usdiAsMesh(s));
                });
            }
            else if(m_mesh != null)
            {
                stream = m_mesh.stream;
                body.Invoke(m_mesh.nativeMeshPtr);
            }

            if(stream != null && ndone > 0)
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
            EditorGUILayout.LabelField("* Precompute tangents requires UVs and is very slow.");

            EditorGUILayout.Space();

            m_genTangents = EditorGUILayout.Toggle("Generate Tangents", m_genTangents);
            m_overwrite = EditorGUILayout.Toggle("Overwrite", m_overwrite);

            GUILayout.Space(10.0f);

            if (GUILayout.Button("Generate"))
            {
                Generate();
                Close();
            }
        }

    }
}
