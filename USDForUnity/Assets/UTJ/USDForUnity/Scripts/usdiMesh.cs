using System;
using UnityEngine;

namespace UTJ
{

    public class usdiMesh : usdiElement
    {
        public usdi.Mesh m_usdiMesh;

        Vector3[] m_points;
        Vector3[] m_normals;
        int[] m_counts;
        int[] m_indices;


        public override void usdiUpdate()
        {
            if(!m_usdiMesh) { return; }
        }
    }

}
