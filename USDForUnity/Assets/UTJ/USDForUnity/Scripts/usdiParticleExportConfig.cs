using System;
using System.Runtime.InteropServices;
using System.Reflection;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{

    [AddComponentMenu("UTJ/USD/Particle Export Config")]
    public class usdiParticleExportConfig : MonoBehaviour
    {
        public bool m_captureRotations = true;
    }

}
