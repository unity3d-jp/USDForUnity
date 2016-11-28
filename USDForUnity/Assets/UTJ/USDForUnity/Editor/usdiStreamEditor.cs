using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    [CustomEditor(typeof(usdiStream))]
    public class usdiStreamEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            DrawDefaultInspector();

            var t = target as usdiStream;
            if (GUILayout.Button("Rebuild Tree"))
            {
                t.usdiReload();
                EditorUtility.SetDirty(t);
            }
            if (GUILayout.Button("Detach USD Components"))
            {
                t.recordUndo = true;
                t.usdiDetach();
            }
        }

    }
}