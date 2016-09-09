using System;
using System.Runtime.InteropServices;
using System.Reflection;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{

    [AddComponentMenu("UTJ/USD/Transform Export Config")]
    public class usdiTransformExportConfig : MonoBehaviour
    {
        public bool m_captureEveryFrame = true;

        void Reset()
        {
            if (gameObject.isStatic)
            {
                m_captureEveryFrame = false;
            }
        }
    }

}
