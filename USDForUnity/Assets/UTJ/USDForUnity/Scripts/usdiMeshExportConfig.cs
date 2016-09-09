using System;
using System.Runtime.InteropServices;
using System.Reflection;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{

    [AddComponentMenu("UTJ/USD/Mesh Export Config")]
    public class usdiMeshExportConfig : MonoBehaviour
    {
        public bool m_captureNormals = true;
        public bool m_captureUVs = true;
        public bool m_captureEveryFrame = false;

#if UNITY_EDITOR
        void Reset()
        {
            if(GetComponent<MeshRenderer>() != null)
            {
                m_captureEveryFrame = false;
            }
            if (GetComponent<SkinnedMeshRenderer>() != null)
            {
                m_captureEveryFrame = true;
            }
        }
#endif

    }

}
