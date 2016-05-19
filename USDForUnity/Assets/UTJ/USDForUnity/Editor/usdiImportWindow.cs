using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    public class usdiImportWindow : EditorWindow
    {
        public string m_path;
        usdiImportOptions m_importOptions = new usdiImportOptions();

        public static void Open(string path)
        {
            usdiImportWindow window = (usdiImportWindow)EditorWindow.GetWindow(typeof(usdiImportWindow));
            window.titleContent = new GUIContent("Import Settings");
            window.m_path = path;
            window.Show();
        }

        void OnGUI()
        {
            m_importOptions.interpolation = (usdi.InterpolationType)EditorGUILayout.EnumPopup("Interpolation", (Enum)m_importOptions.interpolation);
            m_importOptions.scale = EditorGUILayout.FloatField("Scale", m_importOptions.scale);
            m_importOptions.swapHandedness = EditorGUILayout.Toggle("Swap Handedness", m_importOptions.swapHandedness);
            m_importOptions.swapFaces = EditorGUILayout.Toggle("Swap Faces", m_importOptions.swapFaces);

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
            usd.m_importOptions = m_importOptions;
            usd.usdiLoad(m_path);
            return usd;
        }

    }
}
