using UnityEngine;

namespace UTJ
{

    public abstract class usdiElement : MonoBehaviour
    {
        #region fields
        protected usdiStream m_stream;
        protected usdi.Schema m_schema;
        public usdi.VariantSet[] m_variantSets;
        #endregion


        #region properties
        public usdiStream stream
        {
            get { return m_stream; }
            set { m_stream = value; }
        }
        public usdi.Schema usdiObject { get { return m_schema; } }
        #endregion


        #region impl
        public virtual void usdiOnLoad(usdi.Schema schema)
        {
            m_schema = schema;
            m_variantSets = usdi.usdiPrimGetVariantSets(m_schema);
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
