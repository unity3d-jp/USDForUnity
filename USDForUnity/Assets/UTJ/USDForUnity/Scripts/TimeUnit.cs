using System;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{
    [Serializable]
    public class TimeUnit
    {
        public enum Types
        {
            Frame_30FPS,
            Frame_60FPS,
            Seconds,
            FreeScale,
        }
        [SerializeField]
        Types m_type;
        [SerializeField]
        float m_scale;

        public Types type
        {
            get { return m_type; }
            set { m_type = value; m_scale = Adjust(m_type, m_scale); }
        }
        public float scale
        {
            get { return m_scale; }
            set { m_scale = value; m_type = Types.FreeScale; }
        }

        public TimeUnit(Types t = Types.Frame_30FPS)
        {
            type = t;
        }

        public static float Adjust(Types t, float s)
        {
            switch (t)
            {
                case Types.Frame_30FPS: return 30.0f;
                case Types.Frame_60FPS: return 60.0f;
                case Types.Seconds: return 1.0f;
                default: return s;
            }

        }
    }
}
