using System;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    [CustomEditor(typeof(UsdCameraComponent))]
    public class UsdCameraEditor : UsdIComponentEditor
    {
        public override void OnInspectorGUI()
        {
            base.OnInspectorGUI();

            var component = target as UsdCameraComponent;
            var schema = component.schema as UsdCamera;
            if (schema == null) { return; }

            // camera settings
            EditorGUILayout.LabelField("Camera Settings", EditorStyles.boldLabel);
            EditorGUI.BeginChangeCheck();
            var arm = (UsdCamera.AspectRatioMode)EditorGUILayout.EnumPopup("Aspect Ratio Mode", (Enum)schema.aspectRatioMode);
            if (EditorGUI.EndChangeCheck())
            {
                Undo.RecordObject(component, "Changed Aspect  Ratio Mode");
                schema.aspectRatioMode = arm;
                schema.stream.usdiRequestForceUpdate();
                EditorUtility.SetDirty(component);
            }
        }
    }
}