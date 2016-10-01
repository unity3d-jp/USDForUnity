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
        double m_initialTime = 0.0;
        bool m_forceSingleThread = false;

        public static void Open(string path)
        {
            usdiImportWindow window = (usdiImportWindow)EditorWindow.GetWindow(typeof(usdiImportWindow));
            window.titleContent = new GUIContent("Import Settings");
            window.m_path = path;
            window.Show();
        }

        public static usdiStream InstanciateUSD(string path, Action<usdiStream> modifier)
        {
            var go = new GameObject();
            go.name = Path.GetFileNameWithoutExtension(path);

            var usd = go.AddComponent<usdiStream>();
            modifier.Invoke(usd);
            usd.usdiLoad(path);
            return usd;
        }

        void OnGUI()
        {
            m_importOptions.interpolation = (usdi.InterpolationType)EditorGUILayout.EnumPopup("Interpolation", (Enum)m_importOptions.interpolation);
            m_importOptions.scale = EditorGUILayout.FloatField("Scale", m_importOptions.scale);
            m_importOptions.swapHandedness = EditorGUILayout.Toggle("Swap Handedness", m_importOptions.swapHandedness);
            m_importOptions.swapFaces = EditorGUILayout.Toggle("Swap Faces", m_importOptions.swapFaces);
            EditorGUILayout.Space();
            m_initialTime = EditorGUILayout.FloatField("Initial Time", (float)m_initialTime);
            m_forceSingleThread = EditorGUILayout.Toggle("Force Single Thread", m_forceSingleThread);

            GUILayout.Space(10.0f);

            if (GUILayout.Button("Import"))
            {
                var usd = InstanciateUSD(m_path, (stream) => {
                    stream.importOptions = m_importOptions;
                    stream.playTime = m_initialTime;
                    stream.forceSingleThread = m_forceSingleThread;
                });
                Selection.activeGameObject = usd.gameObject;
                Close();
            }
        }

    }
}
