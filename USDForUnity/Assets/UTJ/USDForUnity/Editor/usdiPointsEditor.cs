using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    [CustomEditor(typeof(usdiPoints))]
    public class usdiPointsEditor : usdiXformEditor
    {
        public override void OnInspectorGUI()
        {
            base.OnInspectorGUI();
        }
    }
}