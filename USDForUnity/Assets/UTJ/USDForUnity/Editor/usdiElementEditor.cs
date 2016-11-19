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
            var vsets = t.usdiVariantSets;
            if (vsets.Count > 0)
            {
                EditorGUILayout.LabelField("Variant Sets", EditorStyles.boldLabel);

                var selections = t.usdiVariantSelections;
                for (int i = 0; i < vsets.Count; ++i)
                {
                    var names = vsets.variantNames[i];
                    var selection = selections[i];
                    if(selection == -1) { selection = names.Length - 1; }

                    var ivar = EditorGUILayout.Popup(vsets.setNames[i], selection, names);
                    if (t.usdiSetVariantSelection(i, ivar))
                    {
                        EditorUtility.SetDirty(target);
                    }
                }
            }
        }

    }
}