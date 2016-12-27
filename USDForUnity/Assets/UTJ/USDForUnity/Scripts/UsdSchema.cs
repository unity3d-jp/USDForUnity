using System;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{
    [Serializable]
    public class UsdSchema
    {
        #region fields
        protected GameObject m_go;
        protected bool m_goAssigned = false;
        protected string m_primPath;
        protected string m_primTypeName;
        protected UsdSchema m_master;
        protected UsdStream m_stream;
        protected usdi.Schema m_schema;
        protected usdi.VariantSets m_variantSets;
        [SerializeField] protected int[] m_variantSelection;
        [SerializeField] bool m_overrideImportSettings;
        [SerializeField] usdi.ImportSettings m_importSettings = usdi.ImportSettings.default_value;
        #endregion


        #region properties
        public GameObject gameObject
        {
            get { return m_go; }
            set
            {
                m_go = value;
                m_goAssigned = m_go != null;
            }
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
        public bool overrideImportSettings
        {
            get { return m_overrideImportSettings; }
            set { m_overrideImportSettings = value; }
        }
        public usdi.ImportSettings importSettings
        {
            get { return m_importSettings; }
            set { m_importSettings = value; }
        }
        public string primPath
        {
            get { return m_primPath; }
        }
        public string primTypeName
        {
            get { return m_primTypeName; }
        }
        public bool isEditable
        {
            get { return usdi.usdiPrimIsEditable(m_schema); }
        }
        public bool isInstance
        {
            get { return m_master != null; }
        }
        public bool isMaster
        {
            get { return usdi.usdiPrimIsMaster(m_schema); }
        }
        public bool isInMaster
        {
            get { return usdi.usdiPrimIsInMaster(m_schema); }
        }
        public UsdSchema master
        {
            get { return m_master; }
        }
        #endregion


        #region impl
        public virtual void usdiSetVariantSelection(int iset, int ival)
        {
            m_variantSelection[iset] = ival;
            usdiApplyVariantSets();
        }

        public void usdiSyncVarinatSets()
        {
            m_variantSets = usdi.usdiPrimGetVariantSets(m_schema);
            m_variantSelection = new int[m_variantSets.Count];
            for (int i = 0; i < m_variantSets.Count; ++i)
            {
                m_variantSelection[i] = usdi.usdiPrimGetVariantSelection(m_schema, i);
            }
        }
        public void usdiApplyVariantSets()
        {
            for (int si = 0; si < m_variantSelection.Length; ++si)
            {
                if (usdi.usdiPrimSetVariantSelection(m_schema, si, m_variantSelection[si]))
                {
                    m_stream.usdiSetVariantSelection(m_primPath, m_variantSelection);
                }
            }
        }

        public void usdiSyncImportSettings()
        {
            m_overrideImportSettings = usdi.usdiPrimIsImportSettingsOverriden(m_schema);
            if (m_overrideImportSettings)
            {
                usdi.usdiPrimGetImportSettings(m_schema, ref m_importSettings);
            }
        }

        public void usdiApplyImportSettings()
        {
            if (!m_schema) return;

            usdi.usdiPrimSetOverrideImportSettings(m_schema, m_overrideImportSettings);
            if (m_overrideImportSettings)
            {
                usdi.usdiPrimSetImportSettings(m_schema, ref m_importSettings);
                m_stream.usdiSetImportSettings(primPath, ref m_importSettings);
            }
            else
            {
                m_stream.usdiDeleteImportSettings(primPath);
            }
        }

        protected virtual UsdIComponent usdiSetupSchemaComponent()
        {
            return GetOrAddComponent<UsdComponent>();
        }

        public virtual void usdiOnLoad()
        {
            m_primPath = usdi.usdiPrimGetPathS(m_schema);
            m_primTypeName = usdi.usdiPrimGetUsdTypeNameS(m_schema);
            m_master = m_stream.usdiFindSchema(usdi.usdiPrimGetMaster(m_schema));

            usdiSyncVarinatSets();
            usdiSyncImportSettings();
            if (m_goAssigned)
            {
                var c = usdiSetupSchemaComponent();
                c.schema = this;
            }
        }

        public virtual void usdiOnUnload()
        {
            usdiSync();
            m_schema = default(usdi.Xform);
        }

        public virtual void usdiAsyncUpdate(double time)
        {
        }

        public virtual void usdiUpdate(double time)
        {
        }

        // make sure all async operations are completed
        public virtual void usdiSync()
        {
        }


        public T GetComponent<T>() where T : Component
        {
            if(m_go == null) { return null; }
            return m_go.GetComponent<T>();
        }

        public T GetOrAddComponent<T>() where T : Component
        {
            if (m_go == null) { return null; }
            var c = m_go.GetComponent<T>();
            if(c == null)
            {
                c = m_go.AddComponent<T>();
            }
            return c;
        }
        #endregion
    }

}
