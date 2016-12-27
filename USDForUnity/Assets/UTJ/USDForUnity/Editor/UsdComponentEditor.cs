using System;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    [CustomEditor(typeof(UsdIComponent))]
    public class UsdIComponentEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            //DrawDefaultInspector();

            var component = target as UsdIComponent;
            var schema = component.schema;
            if(schema == null) { return; }

            EditorGUILayout.LabelField("USD Information", EditorStyles.boldLabel);
            EditorGUILayout.LabelField("Prim Path", schema.primPath);
            EditorGUILayout.LabelField("Type Name", schema.primTypeName);

            var master = schema.master;
            if(master != null)
            {
                EditorGUILayout.LabelField("Instance of", master.primPath);
            }


            // variant set selection
            var vsets = schema.variantSets;
            if (vsets != null && vsets.Count > 0)
            {
                EditorGUILayout.Space();
                EditorGUILayout.LabelField("Variant Sets", EditorStyles.boldLabel);

                var selections = (int[])schema.variantSelections.Clone();
                for (int i = 0; i < vsets.Count; ++i)
                {
                    var names = vsets.variantNames[i];
                    var selection = selections[i];
                    if(selection == -1) { selection = names.Length - 1; }

                    EditorGUI.BeginChangeCheck();
                    var ivar = EditorGUILayout.Popup(vsets.setNames[i], selection, names);
                    if (EditorGUI.EndChangeCheck())
                    {
                        Undo.RecordObject(component, "Changed Variant Set");
                        schema.usdiSetVariantSelection(i, ivar);
                        EditorUtility.SetDirty(component);
                    }
                }
            }

            // per-object import settings
            {
                schema.usdiSyncImportSettings();

                EditorGUILayout.Space();
                EditorGUILayout.LabelField("Per-object Import Settings", EditorStyles.boldLabel);

                bool changed = false;
                var overrideImportSettings = schema.overrideImportSettings;
                var importSettings = schema.importSettings;

                EditorGUI.BeginChangeCheck();
                overrideImportSettings = EditorGUILayout.Toggle("Override Import Settings", schema.overrideImportSettings);
                if (EditorGUI.EndChangeCheck())
                {
                    changed = true;
                }

                if (schema.overrideImportSettings)
                {
                    EditorGUI.indentLevel = 1;
                    EditorGUI.BeginChangeCheck();

                    importSettings.interpolation = (usdi.InterpolationType)EditorGUILayout.EnumPopup("Interpolation", (Enum)importSettings.interpolation);
                    importSettings.normalCalculation = (usdi.NormalCalculationType)EditorGUILayout.EnumPopup("Normal Calculation", (Enum)importSettings.normalCalculation);
                    importSettings.tangentCalculation = (usdi.TangentCalculationType)EditorGUILayout.EnumPopup("Tangent Calculation", (Enum)importSettings.tangentCalculation);
                    importSettings.scale = EditorGUILayout.FloatField("Scale", importSettings.scale);
                    importSettings.swapHandedness = EditorGUILayout.Toggle("Swap Handedness", importSettings.swapHandedness);
                    importSettings.swapFaces = EditorGUILayout.Toggle("Swap Faces", importSettings.swapFaces);

                    if (EditorGUI.EndChangeCheck())
                    {
                        changed = true;
                    }
                    EditorGUI.indentLevel = 0;
                }

                if(changed)
                {
                    Undo.RecordObject(component, "Changed Import Settings");
                    schema.overrideImportSettings = overrideImportSettings;
                    schema.importSettings = importSettings;
                    schema.usdiApplyImportSettings();
                    EditorUtility.SetDirty(component);
                }
            }
        }
    }


    [CustomEditor(typeof(UsdComponent))]
    public class UsdComponentEditor : UsdIComponentEditor
    {
        public override void OnInspectorGUI()
        {
            base.OnInspectorGUI();
        }
    }
}