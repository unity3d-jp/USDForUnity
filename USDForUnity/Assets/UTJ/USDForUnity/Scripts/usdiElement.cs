using UnityEngine;

namespace UTJ
{

    public abstract class usdiElement : MonoBehaviour
    {
        public abstract void usdiInitialize(usdi.Schema schema);
        public abstract void usdiUpdate(double time);
    }

}
