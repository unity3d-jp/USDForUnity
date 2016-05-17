using System;
using UnityEngine;

namespace UTJ
{

    public class usdiPoints : usdiXform
    {
        usdi.Points     m_points;
        usdi.PointsData m_pointsData;

        Vector3[] m_positions;
        Vector3[] m_velocities;

        public override void usdiOnLoad(usdi.Schema schema)
        {
            base.usdiOnLoad(schema);

            m_points = usdi.usdiAsPoints(schema);
            if(!m_points)
            {
                Debug.LogWarning("schema is not Points!");
            }
        }

        public override void usdiOnUnload()
        {
            base.usdiOnUnload();
            m_points = default(usdi.Points);
        }

        public override void usdiUpdate(double time)
        {
            base.usdiUpdate(time);
            if (!m_points) { return; }

            if(usdi.usdiPointsReadSample(m_points, ref m_pointsData, time))
            {
            }
        }
    }

}
