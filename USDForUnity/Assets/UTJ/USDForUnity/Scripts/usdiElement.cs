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
        #endregion


        #region properties
        public usdiStream stream
        {
            get { return m_stream; }
            set { m_stream = value; }
        }
        public usdi.Schema usdiObject { get { return m_schema; } }
        public usdi.VariantSet usdiVariantSets { get { return m_variantSets; } }
        public int[] usdiVariantSelections { get { return m_variantSelections; } }
        #endregion


        #region impl
        public virtual bool usdiSetVariantSelection(int iset, int ival)
        {
            if (iset < 0 || iset >= m_variantSets.Count) { return false; }
            if (m_variantSelections[iset] == ival) { return false; }

            usdiSync();
            if (usdi.usdiPrimSetVariantSelection(m_schema, iset, ival))
            {
                m_variantSelections[iset] = ival;
                // todo: refresh
                return true;
            }
            return false;
        }

        public virtual void usdiOnLoad(usdi.Schema schema)
        {
            m_schema = schema;

            // sync variant info
            m_variantSets = usdi.usdiPrimGetVariantSets(m_schema);
            if(m_variantSelections != null && m_variantSelections.Length == m_variantSets.Count)
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
        public abstract void usdiOnUnload();
        public abstract void usdiAsyncUpdate(double time);
        public abstract void usdiUpdate(double time);
        public virtual void usdiSync() {}


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
