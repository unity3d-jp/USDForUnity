using System;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif


namespace UTJ
{

    public abstract class UsdIComponent : MonoBehaviour
    {
        public abstract UsdSchema schema
        {
            get;
            set;
        }

#if UNITY_EDITOR
        void OnValidate()
        {
            var s = schema;
            if (s != null)
            {
                var stream = s.stream;
                if(stream != null)
                {
                    stream.usdiNotifyForceUpdate();
                }
            }
        }
#endif

        void OnDestroy()
        {
            var s = schema;
            if (s != null)
            {
                s.usdiSync();
                s.gameObject = null;
                if(s.stream != null)
                {
                    s.stream.usdiNotifyUpdateElementsList();
                }
            }
        }
    }

    public class UsdComponent : UsdIComponent
    {
        [SerializeField] UsdSchema m_schema;

        public override UsdSchema schema
        {
            get { return m_schema; }
            set { m_schema = value; }
        }
    }
}