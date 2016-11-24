using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    [CustomEditor(typeof(usdiElement))]
    public class usdiElementEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            //DrawDefaultInspector();

            var t = target as usdiElement;
            var vsets = t.variantSets;
            if (vsets != null && vsets.Count > 0)
            {
                EditorGUILayout.LabelField("Variant Sets", EditorStyles.boldLabel);

                var selections = t.variantSelections;
                for (int i = 0; i < vsets.Count; ++i)
                {
                    var names = vsets.variantNames[i];
                    var selection = selections[i];
                    if(selection == -1) { selection = names.Length - 1; }

                    EditorGUI.BeginChangeCheck();
                    var ivar = EditorGUILayout.Popup(vsets.setNames[i], selection, names);
                    if (EditorGUI.EndChangeCheck())
                    {
                        t.stream.recordUndo = true;
                        var objects_to_recotd = new UnityEngine.Object[] { t, t.stream };
                        Undo.RecordObjects(objects_to_recotd, "Changed Variant Set");
                        t.usdiSetVariantSelection(i, ivar);
                        foreach(var o in objects_to_recotd)
                        {
                            EditorUtility.SetDirty(o);
                        }
                        t.stream.recordUndo = false;
                    }
                }
            }
        }

    }
}