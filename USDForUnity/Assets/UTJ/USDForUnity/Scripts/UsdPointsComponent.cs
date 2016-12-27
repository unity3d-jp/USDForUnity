using System;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif


namespace UTJ
{
    public class UsdPointsComponent : UsdIComponent
    {
        [SerializeField]
        UsdPoints m_schema;

        public override UsdSchema schema
        {
            get { return m_schema; }
            set { m_schema = value as UsdPoints; }
        }
    }
}
