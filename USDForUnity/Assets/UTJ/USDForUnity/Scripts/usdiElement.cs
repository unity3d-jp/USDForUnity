using UnityEngine;

namespace UTJ
{

    public abstract class usdiElement : MonoBehaviour
    {
        public abstract void usdiOnLoad(usdi.Schema schema);
        public abstract void usdiOnUnload();
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
    }

}
