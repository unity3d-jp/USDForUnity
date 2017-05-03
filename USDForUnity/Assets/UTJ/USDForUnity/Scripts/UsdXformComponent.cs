using System;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif


namespace UTJ
{
    [ExecuteInEditMode]
    public class UsdXformComponent : UsdIComponent
    {
        [SerializeField]
        UsdXform m_schema;

        public override UsdSchema schema
        {
            get { return m_schema; }
            set { m_schema = value as UsdXform; }
        }
    }
}