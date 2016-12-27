using System;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif


namespace UTJ
{
    public class UsdMeshComponent : UsdIComponent
    {
        [SerializeField]
        UsdMesh m_schema;

        public override UsdSchema schema
        {
            get { return m_schema; }
            set { m_schema = value as UsdMesh; }
        }
    }
}