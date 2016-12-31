using System;
using System.Linq;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif


namespace UTJ
{
    [ExecuteInEditMode]
    public class UsdMeshComponent : UsdIComponent
    {
        [SerializeField] UsdMesh m_schema;
        Material[] m_materials;


        public override UsdSchema schema
        {
            get { return m_schema; }
            set { m_schema = value as UsdMesh; }
        }


        void SyncMaterial()
        {
            if(m_schema == null) { return; }

            bool do_assign = false;
            foreach (var s in m_schema.submeshes)
            {
                if (s.renderer != null)
                {
                    var materials = s.renderer.sharedMaterials;
                    if (materials != null && (m_materials == null || !Enumerable.SequenceEqual(materials, m_materials)))
                    {
                        m_materials = materials;
                        do_assign = true;
                        break;
                    }
                }
            }
            if (do_assign)
            {
                foreach (var s in m_schema.submeshes)
                {
                    if (s.renderer != null)
                    {
                        s.renderer.sharedMaterials = m_materials;
                    }
                }
            }
        }

//#if UNITY_EDITOR
//        public void Update()
//        {
//            if (!EditorApplication.isPlaying)
//            {
//                SyncMaterial();
//            }
//        }
//#endif
    }
}