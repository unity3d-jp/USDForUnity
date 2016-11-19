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

                var iset = EditorGUILayout.Popup("Set", t.usdiVariantSetIndex, vsets.setNames);
                var ivar = EditorGUILayout.Popup("Variant", t.usdiVariantIndex, vsets.variantNames[t.usdiVariantSetIndex]);
                if(t.usdiSetVariantSelection(iset, ivar))
                {
                    EditorUtility.SetDirty(target);
                }
            }
        }

    }
}