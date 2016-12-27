using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    public class UsdImportWindow : EditorWindow
    {
        public string m_path;
        static usdi.ImportSettings s_importOptions = usdi.ImportSettings.default_value;
        static TimeUnit s_timeUnit = new TimeUnit();
        static double s_initialTime = 0.0;
        static bool s_forceSingleThread = false;
        static bool s_directVBUpdate = true;


        public static void Open(string path)
        {
            UsdImportWindow window = (UsdImportWindow)EditorWindow.GetWindow(typeof(UsdImportWindow));
            window.titleContent = new GUIContent("Import Settings");
            window.m_path = path;
            window.Show();
        }

        public static UsdStream InstanciateUSD(string path, Action<UsdStream> modifier)
        {
            var go = new GameObject();
            go.name = Path.GetFileNameWithoutExtension(path);

            var usd = go.AddComponent<UsdStream>();
            modifier.Invoke(usd);
            usd.Load(path);
            return usd;
        }

        void OnGUI()
        {
            s_importOptions.interpolation = (usdi.InterpolationType)EditorGUILayout.EnumPopup("Interpolation", (Enum)s_importOptions.interpolation);
            s_importOptions.normalCalculation = (usdi.NormalCalculationType)EditorGUILayout.EnumPopup("Normal Calculation", (Enum)s_importOptions.normalCalculation);
            s_importOptions.tangentCalculation = (usdi.TangentCalculationType)EditorGUILayout.EnumPopup("Tangent Calculation", (Enum)s_importOptions.tangentCalculation);
            s_importOptions.scale = EditorGUILayout.FloatField("Scale", s_importOptions.scale);
            s_importOptions.swapHandedness = EditorGUILayout.Toggle("Swap Handedness", s_importOptions.swapHandedness);
            s_importOptions.swapFaces = EditorGUILayout.Toggle("Swap Faces", s_importOptions.swapFaces);
            EditorGUILayout.Space();

            s_timeUnit.type = (TimeUnit.Types)EditorGUILayout.EnumPopup("Time Unit", (Enum)s_timeUnit.type);
            s_initialTime = EditorGUILayout.FloatField("Initial Time", (float)s_initialTime);
            //s_forceSingleThread = EditorGUILayout.Toggle("Force Single Thread", s_forceSingleThread);
            //s_directVBUpdate = EditorGUILayout.Toggle("Direct VB Update", s_directVBUpdate);

            GUILayout.Space(10.0f);

            if (GUILayout.Button("Import"))
            {
                var usd = InstanciateUSD(m_path, (stream) => {
                    stream.importSettings = s_importOptions;
                    stream.timeUnit = s_timeUnit;
                    stream.playTime = s_initialTime;
                    stream.forceSingleThread = s_forceSingleThread;
                    stream.directVBUpdate = s_directVBUpdate;
                });
                Selection.activeGameObject = usd.gameObject;
                Close();
            }
        }

    }
}
