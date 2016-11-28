using System;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    [CustomEditor(typeof(usdiCameraElement))]
    public class usdiCameraEditor : usdiElementEditor
    {
        public override void OnInspectorGUI()
        {
            base.OnInspectorGUI();

            var component = target as usdiCameraElement;
            var schema = component.schema as usdiCamera;
            if (schema == null) { return; }

            // camera settings
            EditorGUILayout.LabelField("Camera Settings", EditorStyles.boldLabel);
            EditorGUI.BeginChangeCheck();
            var arm = (usdiCamera.AspectRatioMode)EditorGUILayout.EnumPopup("Aspect Ratio Mode", (Enum)schema.aspectRatioMode);
            if (EditorGUI.EndChangeCheck())
            {
                Undo.RecordObject(component, "Changed Aspect  Ratio Mode");
                schema.aspectRatioMode = arm;
                schema.stream.usdiNotifyForceUpdate();
                EditorUtility.SetDirty(component);
            }
        }
    }
}