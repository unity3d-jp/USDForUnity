using System;
using UnityEditor;
using UnityEngine;

namespace UTJ.USD
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