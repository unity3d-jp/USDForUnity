using System;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ.USD
{
    [Serializable]
    public class UsdSchema : IDisposable
    {
        #region fields
        protected GameObject m_linkedGameObj;
        protected string m_primPath;
        protected string m_primName;
        protected string m_primTypeName;
        protected UsdSchema m_master;
        protected UsdStream m_stream;
        protected usdi.Schema m_schema;
        protected usdi.VariantSets m_variantSets;
        protected usdi.AssetRef[] m_referencingAssets;
        [SerializeField] protected int[] m_variantSelection;
        #endregion


        #region properties
        public GameObject gameObject
        {
            get { return m_linkedGameObj; }
            set { m_linkedGameObj = value; }
        }
        public UsdStream stream
        {
            get { return m_stream; }
            set { m_stream = value; }
        }
        public usdi.Schema nativeSchemaPtr
        {
            get { return m_schema; }
            set { m_schema = value; }
        }
        public usdi.VariantSets variantSets
        {
            get { return m_variantSets; }
        }
        public int[] variantSelections
        {
            get { return m_variantSelection; }
        }
        public string primPath
        {
            get { return m_primPath; }
        }
        public string primName
        {
            get { return m_primName; }
        }
        public string primTypeName
        {
            get { return m_primTypeName; }
        }
        public bool isEditable
        {
            get { return m_schema.IsEditable(); }
        }
        public bool isInstance
        {
            get { return m_master != null; }
        }
        public bool isMaster
        {
            get { return m_schema.IsMaster(); }
        }
        public bool isInMaster
        {
            get { return m_schema.IsInMaster(); }
        }
        public UsdSchema master
        {
            get { return m_master; }
        }
        public usdi.AssetRef[] referencingAssets
        {
            get { return m_referencingAssets; }
        }
        #endregion


        #region impl
        public void Dispose()
        {
            m_linkedGameObj = null;
        }

        public virtual void UsdSetVariantSelection(int iset, int ival)
        {
            m_variantSelection[iset] = ival;
            UsdApplyVariantSets();
        }

        public void UsdSyncVarinatSets()
        {
            m_variantSets = usdi.usdiPrimGetVariantSets(m_schema);
            m_variantSelection = new int[m_variantSets.Count];
            for (int i = 0; i < m_variantSets.Count; ++i)
            {
                m_variantSelection[i] = m_schema.GetVariantSelection(i);
            }
        }
        public void UsdApplyVariantSets()
        {
            for (int si = 0; si < m_variantSelection.Length; ++si)
            {
                if (m_schema.SetVariantSelection(si, m_variantSelection[si]))
                {
                    m_stream.UsdSetVariantSelection(m_primPath, m_variantSelection);
                }
            }
        }

        protected virtual UsdIComponent UsdSetupSchemaComponent()
        {
            return GetOrAddComponent<UsdComponent>();
        }

        public virtual void UsdOnLoad()
        {
            m_primPath = m_schema.GetPath();
            m_primName = m_schema.GetName();
            m_primTypeName = m_schema.GetTypeName();
            m_master = m_stream.UsdFindSchema(m_schema.GetMaster());

            UsdSyncVarinatSets();
            m_referencingAssets = usdi.usdiGetReferencingAssets(m_schema);
            if (m_linkedGameObj != null)
            {
                var c = UsdSetupSchemaComponent();
                c.schema = this;
            }
        }

        public virtual void UsdOnUnload()
        {
            UsdSyncDataEnd();
            m_schema = default(usdi.Xform);
        }

        public virtual void UsdPrepareSample()
        {
        }

        public virtual void UsdSyncDataBegin()
        {
        }

        public virtual void UsdSyncDataEnd()
        {
        }


        public T GetComponent<T>() where T : Component
        {
            if(m_linkedGameObj == null) { return null; }
            return m_linkedGameObj.GetComponent<T>();
        }

        public T GetOrAddComponent<T>() where T : Component
        {
            if (m_linkedGameObj == null) { return null; }
            var c = m_linkedGameObj.GetComponent<T>();
            if(c == null)
            {
                c = m_linkedGameObj.AddComponent<T>();
            }
            return c;
        }
        #endregion
    }

}
