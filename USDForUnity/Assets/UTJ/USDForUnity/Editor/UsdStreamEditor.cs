using System;
using System.IO;
using UnityEditor;
using UnityEngine;
using UTJ.USD;

namespace UTJ.USD
{
    [CustomEditor(typeof(UsdStream))]
    public class UsdStreamEditor : Editor
    {
#if UNITY_2017_1_OR_NEWER
        private bool m_ShowImportSettings = true;
        private bool m_ShowAdvOptions = false;

        public override void OnInspectorGUI()
        {
            var so = this.serializedObject;
            so.Update();

            EditorGUI.BeginChangeCheck();
            try
            {
                var stream = so.targetObject as UsdStream;

                AddSimpleProperty(serializedObject.FindProperty(() => stream.m_time), "Time", "");

                m_ShowImportSettings = EditorGUILayout.Foldout(m_ShowImportSettings, "Import Settings");
                if (m_ShowImportSettings)
                {
                    using (var enabled = new EditorGUI.DisabledScope(true))
                    {
                        AddEnumProperty(so.FindProperty(() => stream.m_path.m_root), "Root", "", stream.m_path.m_root.GetType());
                        AddSimpleProperty(so.FindProperty(() => stream.m_path.m_leaf), "SubPath", "");
                    }

                    AddEnumProperty(serializedObject.FindProperty(() => stream.m_importSettings.interpolation), "Interpolation", "", stream.m_importSettings.interpolation.GetType());
                    AddEnumProperty(serializedObject.FindProperty(() => stream.m_importSettings.normalCalculation), "Compute normals", "", stream.m_importSettings.normalCalculation.GetType());
                    AddEnumProperty(serializedObject.FindProperty(() => stream.m_importSettings.tangentCalculation), "Compute tangents", "", stream.m_importSettings.tangentCalculation.GetType());
                    AddSimpleProperty(serializedObject.FindProperty(() => stream.m_importSettings.scale), "Scale", "");
                    AddByteBoolProperty(serializedObject.FindProperty(() => stream.m_importSettings.swapHandedness.v), "Swap handedness", "");
                    AddByteBoolProperty(serializedObject.FindProperty(() => stream.m_importSettings.swapFaces.v), "Swap faces", "");
                    AddEnumProperty(serializedObject.FindProperty(() => stream.m_timeUnit.m_type), "Time unit", "", stream.m_timeUnit.type.GetType());
                }

                m_ShowAdvOptions = EditorGUILayout.Foldout(m_ShowAdvOptions, new GUIContent("Advanced options"));
                if (m_ShowAdvOptions)
                {
                    AddSimpleProperty(serializedObject.FindProperty(() => stream.m_forceSingleThread), "Single threaded", "");
                    AddSimpleProperty(serializedObject.FindProperty(() => stream.m_detailedLog), "Detailed log", "");
                    AddSimpleProperty(serializedObject.FindProperty(() => stream.m_deferredUpdate), "Defered udpate", "");
                }

            }
            finally 
            {
                so.ApplyModifiedProperties();
                EditorGUI.EndChangeCheck();
            }
            AddButtons();
        }

        private void AddSimpleProperty(SerializedProperty property, string text, string tooltip)
        {
            if (property == null)
                return;

            switch (property.propertyType)
            {
                case SerializedPropertyType.Integer:
                    {
                        property.intValue = EditorGUILayout.IntField(new GUIContent(text, tooltip), property.intValue);
                        break;
                    }
                case SerializedPropertyType.Boolean:
                    {
                        property.boolValue = EditorGUILayout.Toggle(new GUIContent(text, tooltip), property.boolValue);
                        break;
                    }
                case SerializedPropertyType.Float:
                    {
                        property.floatValue = EditorGUILayout.FloatField(new GUIContent(text, tooltip), property.floatValue);
                        break;
                    }
                case SerializedPropertyType.String:
                    {
                        property.stringValue = EditorGUILayout.TextField(new GUIContent(text, tooltip), property.stringValue);
                        break;
                    }

                case SerializedPropertyType.ObjectReference:
                default:
                    return;
            }
        }

        void AddEnumProperty(SerializedProperty property, string text, string tooltip, Type typeOfEnum)
        {
            if (property == null)
                return;
            Rect ourRect = EditorGUILayout.BeginHorizontal();
            EditorGUI.BeginProperty(ourRect, GUIContent.none, property);

            int selectionFromInspector = property.intValue;
            string[] enumNamesList = System.Enum.GetNames(typeOfEnum);
            var actualSelected = EditorGUILayout.Popup(text, selectionFromInspector, enumNamesList);
            property.intValue = actualSelected;
            EditorGUI.EndProperty();
            EditorGUILayout.EndHorizontal();
        }

        private void AddByteBoolProperty(SerializedProperty property, string text, string tooltip)
        {
            if (property == null)
                return;

            var orgValue = property.intValue != 0;
            var newValue = EditorGUILayout.Toggle(new GUIContent(text, tooltip), orgValue);
            property.intValue = newValue ? 1 : 0;
        }

#else
        public override void OnInspectorGUI()
        {
            DrawDefaultInspector();
            AddButtons();
        }

#endif

        private void AddButtons()
        {
            var t = target as UsdStream;

            EditorGUILayout.Space();
            if (GUILayout.Button("Rebuild Tree"))
            {
                t.UsdReload();
                EditorUtility.SetDirty(t);
            }

            EditorGUILayout.Space();
            if (GUILayout.Button("Detach USD Components"))
            {
                t.UsdDetachUsdComponents();
            }
        }
    }
}
