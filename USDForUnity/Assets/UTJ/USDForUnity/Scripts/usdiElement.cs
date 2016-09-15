using UnityEngine;

namespace UTJ
{

    public abstract class usdiElement : MonoBehaviour
    {
        #region fields
        protected usdiStream m_stream;
        #endregion


        #region properties
        public usdiStream stream
        {
            get { return m_stream; }
            set { m_stream = value; }
        }
        #endregion


        #region impl
        public abstract void usdiOnLoad(usdi.Schema schema);
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
