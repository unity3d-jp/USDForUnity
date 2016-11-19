using UnityEngine;

namespace UTJ
{

    public abstract class usdiElement : MonoBehaviour
    {
        #region fields
        protected usdiStream m_stream;
        protected usdi.Schema m_schema;
        protected usdi.VariantSet m_variantSets;
        [SerializeField] protected int m_variantSetIndex = 0;
        [SerializeField] protected int m_variantIndex = 0;
        #endregion


        #region properties
        public usdiStream stream
        {
            get { return m_stream; }
            set { m_stream = value; }
        }
        public usdi.Schema usdiObject { get { return m_schema; } }
        public usdi.VariantSet usdiVariantSets { get { return m_variantSets; } }
        public int usdiVariantSetIndex { get { return m_variantSetIndex; } }
        public int usdiVariantIndex { get { return m_variantIndex; } }
        #endregion


        #region impl
        public bool usdiSetVariantSelection(int iset, int ival)
        {
            if(iset == m_variantSetIndex && ival == m_variantIndex)
            {
                // nothing todo
                return false;
            }
            if(usdi.usdiPrimSetVariantSelection(m_schema, iset, ival))
            {
                m_variantSetIndex = iset;
                m_variantIndex = ival;
                // todo: refresh
                return true;
            }
            return false;
        }

        public virtual void usdiOnLoad(usdi.Schema schema)
        {
            m_schema = schema;
            m_variantSets = usdi.usdiPrimGetVariantSets(m_schema);
            usdi.usdiPrimSetVariantSelection(m_schema, m_variantSetIndex, m_variantIndex);
        }
        public abstract void usdiOnUnload();
        public abstract void usdiAsyncUpdate(double time);
        public abstract void usdiUpdate(double time);


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
