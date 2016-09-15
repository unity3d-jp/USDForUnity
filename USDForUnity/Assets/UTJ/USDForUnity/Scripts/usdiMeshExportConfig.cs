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
        #region fields
        public bool m_captureNormals = true;
        public bool m_captureUVs = true;
        public bool m_captureEveryFrame = false;
        public bool m_captureEveryFrameUV = false;
        public bool m_captureEveryFrameIndices = false;
        #endregion


        #region callbacks
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
        #endregion

    }

}
