using System;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    [CustomEditor(typeof(usdiIElement))]
    public class usdiElementEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            //DrawDefaultInspector();

            var component = target as usdiIElement;
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
                        var objects_to_recotd = new UnityEngine.Object[] { component, schema.stream };
                        Undo.RecordObjects(objects_to_recotd, "Changed Variant Set");
                        schema.usdiSetVariantSelection(i, ivar);
                        foreach(var o in objects_to_recotd)
                        {
                            EditorUtility.SetDirty(o);
                        }
                        schema.stream.recordUndo = false;
                    }
                }
            }
        }
    }


    [CustomEditor(typeof(usdiElement))]
    public class usdiSchemaEditor : usdiElementEditor
    {
        public override void OnInspectorGUI()
        {
            base.OnInspectorGUI();
        }
    }
}