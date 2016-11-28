using System;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif


namespace UTJ
{
    public class usdiPointsElement : usdiIElement
    {
        [SerializeField]
        usdiPoints m_schema;

        public override usdiSchema schema
        {
            get { return m_schema; }
            set { m_schema = value as usdiPoints; }
        }
    }
}
