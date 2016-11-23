using UnityEngine;

namespace UTJ
{

    public abstract class usdiElement : MonoBehaviour
    {
        #region fields
        protected usdiStream m_stream;
        protected usdi.Schema m_schema;
        protected usdi.VariantSet m_variantSets;
        [SerializeField] protected int[] m_variantSelections;
        [SerializeField] protected string m_primPath;
        [SerializeField] protected string m_primTypeName;
        #endregion


        #region properties
        public usdiStream stream
        {
            get { return m_stream; }
            set { m_stream = value; }
        }
        public usdi.Schema usdiSchema { get { return m_schema; } }
        public usdi.VariantSet variantSets { get { return m_variantSets; } }
        public int[] variantSelections { get { return m_variantSelections; } }
        public string primPath { get { return m_primPath; } }
        public string primTypeName { get { return m_primTypeName; } }
        #endregion


        #region impl
        public virtual bool usdiSetVariantSelection(int iset, int ival)
        {
            if (iset < 0 || iset >= m_variantSets.Count) { return false; }
            if (m_variantSelections[iset] == ival) { return false; }

            if (usdi.usdiPrimSetVariantSelection(m_schema, iset, ival))
            {
                m_variantSelections[iset] = ival;
                m_stream.usdiNotifyUpdateNeeded();
                return true;
            }
            return false;
        }

        protected void usdiSyncVarinatSets()
        {
            m_variantSets = usdi.usdiPrimGetVariantSets(m_schema);
            if (m_variantSelections != null && m_variantSelections.Length == m_variantSets.Count)
            {
                for (int i = 0; i < m_variantSets.Count; ++i)
                {
                    if (!usdi.usdiPrimSetVariantSelection(m_schema, i, m_variantSelections[i]))
                    {
                        m_variantSelections[i] = usdi.usdiPrimGetVariantSelection(m_schema, i);
                    }
                }
            }
            else
            {
                m_variantSelections = new int[m_variantSets.Count];
                for (int i = 0; i < m_variantSets.Count; ++i)
                {
                    m_variantSelections[i] = usdi.usdiPrimGetVariantSelection(m_schema, i);
                }
            }
        }

        public virtual void usdiOnLoad(usdi.Schema schema)
        {
            m_schema = schema;
            m_primPath = usdi.usdiPrimGetPathS(schema);
            m_primTypeName = usdi.usdiPrimGetUsdTypeNameS(schema);
            usdiSyncVarinatSets();
        }

        public virtual bool usdiOnReload()
        {
            m_schema = usdi.usdiFindSchema(m_stream.usdiContext, m_primPath);
            if (!m_schema) { return false; }
            if (m_primTypeName != usdi.usdiPrimGetUsdTypeNameS(m_schema))
            {
                return false;
            }
            usdiSyncVarinatSets();
            return true;
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

        public virtual void usdiSync()
        {
        }


        protected T GetOrAddComponent<T>() where T : Component
        {
            var c = GetComponent<T>();
            if(c == null)
            {
                c = gameObject.AddComponent<T>();
            }
            return c;
        }
        #endregion
    }

}
