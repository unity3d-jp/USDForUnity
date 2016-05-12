using UnityEngine;
using System.Collections.Generic;

namespace UTJ
{

    public class usdiStream : MonoBehaviour
    {
        public string m_path;
        usdi.Context m_usdi;
        List<usdiElement> m_elements;

        void Update()
        {
            foreach(var e in m_elements)
            {
                e.usdiUpdate();
            }
        }
    }

}
