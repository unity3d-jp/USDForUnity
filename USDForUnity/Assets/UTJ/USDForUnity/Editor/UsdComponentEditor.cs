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

                var selections = schema.variantSelections;
                for (int i = 0; i < vsets.Count; ++i)
                {
                    var names = vsets.variantNames[i];
                    var selection = selections[i];
                    if(selection == -1) { selection = names.Length - 1; }

                    EditorGUI.BeginChangeCheck();
                    var ivar = EditorGUILayout.Popup(vsets.setNames[i], selection, names);
                    if (EditorGUI.EndChangeCheck())
                    {
                        schema.stream.recordUndo = true;
                        Undo.RecordObject(schema.stream, "Changed Variant Set");
                        schema.usdiSetVariantSelection(i, ivar);
                        EditorUtility.SetDirty(schema.stream);
                        schema.stream.recordUndo = false;
                    }
                }
            }

            // per-object import settings
            {
                schema.usdiSyncImportSettings();

                EditorGUILayout.Space();
                EditorGUILayout.LabelField("Per-object Import Settings", EditorStyles.boldLabel);

                bool changed = false;
                EditorGUI.BeginChangeCheck();
                schema.overrideImportSettings = EditorGUILayout.Toggle("Override Import Settings", schema.overrideImportSettings);
                if (EditorGUI.EndChangeCheck())
                {
                    changed = true;
                }

                if (schema.overrideImportSettings)
                {
                    EditorGUI.indentLevel = 1;
                    EditorGUI.BeginChangeCheck();

                    usdi.ImportSettings tmp = schema.importSettings;
                    tmp.interpolation = (usdi.InterpolationType)EditorGUILayout.EnumPopup("Interpolation", (Enum)tmp.interpolation);
                    tmp.normalCalculation = (usdi.NormalCalculationType)EditorGUILayout.EnumPopup("Normal Calculation", (Enum)tmp.normalCalculation);
                    tmp.tangentCalculation = (usdi.TangentCalculationType)EditorGUILayout.EnumPopup("Tangent Calculation", (Enum)tmp.tangentCalculation);
                    tmp.scale = EditorGUILayout.FloatField("Scale", tmp.scale);
                    tmp.swapHandedness = EditorGUILayout.Toggle("Swap Handedness", tmp.swapHandedness);
                    tmp.swapFaces = EditorGUILayout.Toggle("Swap Faces", tmp.swapFaces);

                    if (EditorGUI.EndChangeCheck())
                    {
                        schema.importSettings = tmp;
                        changed = true;
                    }
                    EditorGUI.indentLevel = 0;
                }

                if(changed)
                {
                    Undo.RecordObject(schema.stream, "Changed Import Settings");
                    schema.usdiApplyImportSettings();
                    EditorUtility.SetDirty(schema.stream);
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