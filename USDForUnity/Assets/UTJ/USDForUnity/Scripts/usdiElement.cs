using System;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif


namespace UTJ
{

    public abstract class usdiIElement : MonoBehaviour
    {
        public abstract usdiSchema schema
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
                s.stream.usdiNotifyUpdateElementsList();
            }
        }
    }

    public class usdiElement : usdiIElement
    {
        [SerializeField] usdiSchema m_schema;

        public override usdiSchema schema
        {
            get { return m_schema; }
            set { m_schema = value; }
        }
    }
}