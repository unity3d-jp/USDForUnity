using System;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{
    [Serializable]
    public class usdiSchema
    {
        #region fields
        [SerializeField] protected GameObject m_go;
        [SerializeField] protected bool m_goAssigned = false;
        [SerializeField] protected string m_primPath;
        [SerializeField] protected string m_primTypeName;
        protected usdiSchema m_master;
        protected usdiStream m_stream;
        protected usdi.Schema m_schema;
        protected usdi.VariantSets m_variantSets;
        [SerializeField] protected int[] m_variantSelections;
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
        public usdiStream stream
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
            get { return m_variantSelections; }
        }
        public string primPath
        {
            get { return m_primPath; }
        }
        public string primTypeName
        {
            get { return m_primTypeName; }
        }
        public bool isInstance
        {
            get { return m_master != null; }
        }
        public usdiSchema master
        {
            get { return m_master; }
        }
        #endregion


        #region impl
        public virtual bool usdiSetVariantSelection(int iset, int ival)
        {
            if (iset < 0 || iset >= m_variantSets.Count) { return false; }
            if (m_variantSelections[iset] == ival) { return false; }

            if (usdi.usdiPrimSetVariantSelection(m_schema, iset, ival))
            {
                m_variantSelections[iset] = ival;
                m_stream.usdiSetVariantSelection(m_primPath, m_variantSelections);
                return true;
            }
            return false;
        }

        protected void usdiSyncVarinatSets()
        {
            m_variantSets = usdi.usdiPrimGetVariantSets(m_schema);
            m_variantSelections = new int[m_variantSets.Count];
            for (int i = 0; i < m_variantSets.Count; ++i)
            {
                m_variantSelections[i] = usdi.usdiPrimGetVariantSelection(m_schema, i);
            }
        }

        protected virtual usdiIElement usdiSetupSchemaComponent()
        {
            return GetOrAddComponent<usdiElement>();
        }

        public void usdiDestroy()
        {
#if UNITY_EDITOR
            if(m_stream.recordUndo)
            {
                Undo.DestroyObjectImmediate(m_go);
            }
            else
#endif
            {
                GameObject.DestroyImmediate(m_go);
            }
        }

        public virtual void usdiOnLoad()
        {
            m_primPath = usdi.usdiPrimGetPathS(m_schema);
            m_primTypeName = usdi.usdiPrimGetUsdTypeNameS(m_schema);
            m_master = m_stream.usdiFindSchema(usdi.usdiPrimGetMaster(m_schema));
            usdiSyncVarinatSets();
            if(m_goAssigned)
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
