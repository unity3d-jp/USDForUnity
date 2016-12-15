using System;
using System.Runtime.InteropServices;
using System.Reflection;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{

    [AddComponentMenu("USD/Transform Export Config")]
    public class usdiTransformExportConfig : MonoBehaviour
    {
        #region fields
        public bool m_captureEveryFrame = true;
        #endregion

        #region callbacks
        void Reset()
        {
            if (gameObject.isStatic)
            {
                m_captureEveryFrame = false;
            }
        }
        #endregion
    }

}
