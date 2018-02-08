using System;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif


namespace UTJ.USD
{

    [ExecuteInEditMode]
    public abstract class UsdIComponent : MonoBehaviour
    {
        public abstract UsdSchema schema
        {
            get;
            set;
        }

#if UNITY_EDITOR
        public virtual void OnValidate()
        {
            if(!usdi.pluginInitialized) return;

            var s = schema;
            if (s != null)
            {
                s.usdiApplyImportSettings();
                s.usdiApplyVariantSets();
            }
        }
#endif

        void OnDestroy()
        {
            var s = schema;
            if (s != null)
            {
                s.UsdSync();
                s.gameObject = null;
            }
        }
    }

    [ExecuteInEditMode]
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