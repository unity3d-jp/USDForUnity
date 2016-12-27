using System;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    [CustomEditor(typeof(UsdExporter))]
    public class UsdExporterEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            DrawDefaultInspector();

            GUILayout.Space(10);
            EditorGUILayout.LabelField("Capture Control", EditorStyles.boldLabel);

            var t = target as UsdExporter;

            if (t.isRecording)
            {
                if (GUILayout.Button("End Capture"))
                {
                    t.EndCapture();
                }
            }
            else
            {
                if (GUILayout.Button("Begin Capture"))
                {
                    t.BeginCapture();
                }

                if (GUILayout.Button("One Shot"))
                {
                    t.OneShot();
                }
            }
        }
    }
}
