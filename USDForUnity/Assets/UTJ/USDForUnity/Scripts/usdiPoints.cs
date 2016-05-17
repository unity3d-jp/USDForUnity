using System;
using UnityEngine;

namespace UTJ
{

    public class usdiPoints : usdiElement
    {
        usdi.Points     m_points;
        usdi.PointsData m_pointsData;

        Vector3[] m_positions;
        Vector3[] m_velocities;

        public override void usdiOnLoad(usdi.Schema schema)
        {
            m_points = usdi.usdiAsPoints(schema);
            if(!m_points)
            {
                Debug.LogWarning("schema is not Points!");
            }
        }

        public override void usdiOnUnload()
        {
            m_points = default(usdi.Points);
        }

        public override void usdiUpdate(double time)
        {
            if (!m_points) { return; }

            if(usdi.usdiPointsReadSample(m_points, ref m_pointsData, time))
            {
            }
        }
    }

}
