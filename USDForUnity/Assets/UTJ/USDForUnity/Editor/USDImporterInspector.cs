#if UNITY_2017_1_OR_NEWER

using System;
using UnityEditor;
using UnityEditor.Experimental.AssetImporters;
using UnityEngine;

namespace UTJ.USD
{
    [CustomEditor(typeof(USDImporter))]
    public class USDImporterInspector : ScriptedImporterEditor
    {
        private bool m_ShowAdvOptions= false;


        public override void OnInspectorGUI()
        {
            var importer = serializedObject.targetObject as USDImporter;
            AddEnumProperty(serializedObject.FindProperty(() => importer.m_importMode), "Import Mode", "", importer.m_importMode.GetType() );
            AddEnumProperty(serializedObject.FindProperty(()=> importer.m_importSettings.interpolation ), "Interpolation", "", importer.m_importSettings.interpolation.GetType() );
            AddEnumProperty(serializedObject.FindProperty(() => importer.m_importSettings.normalCalculation), "Compute normals", "", importer.m_importSettings.normalCalculation.GetType());
            AddEnumProperty(serializedObject.FindProperty(() => importer.m_importSettings.tangentCalculation), "Compute tangents", "", importer.m_importSettings.tangentCalculation.GetType());
            AddFloatProperty(serializedObject.FindProperty(() => importer.m_importSettings.scale), "Scale", "" );
            AddByteBoolProperty(serializedObject.FindProperty(() => importer.m_importSettings.swapHandedness.v), "Swap handedness", "");
            AddByteBoolProperty(serializedObject.FindProperty(() => importer.m_importSettings.swapFaces.v), "Swap faces", "");
            var type = serializedObject.FindProperty(() => importer.m_timeUnit).FindPropertyRelative("m_type");
            AddEnumProperty(type, "Time unit", "", importer.m_timeUnit.type.GetType());

            // Adv. Settings
            m_ShowAdvOptions = EditorGUILayout.Foldout(m_ShowAdvOptions, new GUIContent("Advanced options"));
            if (m_ShowAdvOptions)
            {
                AddBoolProperty(serializedObject.FindProperty(() => importer.m_forceSingleThread), "Single threaded", "");
                AddBoolProperty(serializedObject.FindProperty(() => importer.m_deferredUpdate), "Defered udpate", "");
            }

            base.ApplyRevertGUI();
        }

        private void AddBoolProperty(SerializedProperty property, string text, string tooltip)
        {
            if (property == null)
                return;

            var orgValue = property.boolValue;
            var newValue = EditorGUILayout.Toggle(new GUIContent(text, tooltip), orgValue);
            property.boolValue = newValue;
        }

        private void AddByteBoolProperty(SerializedProperty property, string text, string tooltip)
        {
            if (property == null)
                return;

            var orgValue = property.intValue != 0;
            var newValue = EditorGUILayout.Toggle(new GUIContent(text, tooltip), orgValue);
            property.intValue = newValue ?  1 : 0;
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

        void AddIntProperty(SerializedProperty property, string text, string tooltip)
        {
            if (property == null)
                return;

            var orgValue = property.intValue;
            var newValue = EditorGUILayout.IntField(new GUIContent(text, tooltip), orgValue);
            property.intValue = newValue;
        }

        void AddFloatProperty(SerializedProperty property, string text, string tooltip)
        {
            if (property == null)
                return;

            var orgValue = property.floatValue;
            var newValue = EditorGUILayout.FloatField(new GUIContent(text, tooltip), orgValue);
            property.floatValue = newValue;
        }

    }
}

#endif
