using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif


namespace UTJ
{
    public abstract class usdiCustomComponentCapturer : MonoBehaviour
    {
        public abstract void CreateAbcObject(usdi.Schema parent);
        public abstract void Capture(double t);
    }
}
