using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    public class usdiImportWindow : EditorWindow
    {
        public string m_path;
        public float m_scale = 1.0f;
        public bool m_swapHandedness = true;
        public bool m_swapFaces = true;

        public static void Open(string path)
        {
            usdiImportWindow window = (usdiImportWindow)EditorWindow.GetWindow(typeof(usdiImportWindow));
            window.titleContent = new GUIContent("Import Settings");
            window.m_path = path;
            window.Show();
        }

        void OnGUI()
        {
            m_scale = EditorGUILayout.FloatField("Scale", m_scale);
            m_swapHandedness = EditorGUILayout.Toggle("Swap Handedness", m_swapHandedness);
            m_swapFaces = EditorGUILayout.Toggle("Swap Faces", m_swapFaces);

            GUILayout.Space(10.0f);

            if (GUILayout.Button("Import"))
            {
                Import();
                Close();
            }
        }

        usdiStream Import()
        {
            var go = new GameObject();
            go.name = Path.GetFileNameWithoutExtension(m_path);

            var usd = go.AddComponent<usdiStream>();
            usd.m_scale = m_scale;
            usd.m_swapHandedness = m_swapHandedness;
            usd.m_swapFaces = m_swapFaces;
            usd.usdiLoad(m_path);
            return usd;
        }

    }
}
