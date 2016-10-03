using System;
using UnityEngine;

namespace UTJ
{

    public class usdiPoints : usdiXform
    {
        #region fields
        usdi.Points     m_points;
        usdi.Attribute  m_attrRot;
        usdi.PointsSummary m_summary = default(usdi.PointsSummary);
        usdi.PointsData m_pointsData;
        usdi.AttributeData m_rotData;

        Vector3[] m_positions;
        Vector3[] m_velocities;
        Vector4[] m_rotations;

        usdi.Task m_asyncRead;
        double m_timeRead;
        #endregion


        #region properties
        public Vector3[] positions { get { return m_positions; } }
        public Vector3[] velocities { get { return m_velocities; } }
        #endregion


        #region impl
        public override void usdiOnLoad(usdi.Schema schema)
        {
            base.usdiOnLoad(schema);

            m_points = usdi.usdiAsPoints(schema);
            if(!m_points)
            {
                Debug.LogWarning("schema is not Points!");
                return;
            }
            usdi.usdiPointsGetSummary(m_points, ref m_summary);
            m_attrRot = usdi.usdiFindAttribute(m_points, "rotations");
        }

        public override void usdiOnUnload()
        {
            base.usdiOnUnload();

            if (m_asyncRead != null)
            {
                m_asyncRead.Wait();
                m_asyncRead = null;
            }

            m_points = default(usdi.Points);
            m_summary = default(usdi.PointsSummary);
            m_pointsData = default(usdi.PointsData);
            m_attrRot = default(usdi.Attribute);
            m_rotData = default(usdi.AttributeData);

            m_positions = null;
            m_velocities = null;
            m_rotations = null;
        }

        public override void usdiAsyncUpdate(double time)
        {
            base.usdiAsyncUpdate(time);
            if (!m_needsUpdate) { return; }

            usdi.PointsData tmp = usdi.PointsData.default_value;
            usdi.usdiPointsReadSample(m_points, ref tmp, time, true);

            // allocate points data
            if (m_pointsData.num_points == tmp.num_points)
            {
                // no need to allocate
            }
            else
            {
                m_pointsData.num_points = tmp.num_points;

                m_positions = new Vector3[m_pointsData.num_points];
                m_pointsData.points = usdi.GetArrayPtr(m_positions);

                if (m_summary.has_velocities)
                {
                    m_velocities = new Vector3[m_pointsData.num_points];
                    m_pointsData.velocities = usdi.GetArrayPtr(m_velocities);
                }
                if (m_attrRot)
                {
                    m_rotations = new Vector4[m_pointsData.num_points];
                    m_rotData.data = usdi.GetArrayPtr(m_rotations);
                    m_rotData.num_elements = tmp.num_points;
                }
            }

            // read points data
            if (m_pointsData.num_points > 0)
            {

#if UNITY_EDITOR
                if (m_stream.forceSingleThread)
                {
                    usdi.usdiPointsReadSample(m_points, ref m_pointsData, m_timeRead, true);
                    if (m_attrRot)
                    {
                        usdi.usdiAttrReadSample(m_attrRot, ref m_rotData, m_timeRead, true);
                    }
                }
                else
#endif
                {
                    if (m_asyncRead == null)
                    {
                        if(m_attrRot)
                        {
                            m_asyncRead = new usdi.CompositeTask(new IntPtr[] {
                                usdi.usdiTaskCreatePointsReadSample(m_points, ref m_pointsData, ref m_timeRead),
                                usdi.usdiTaskCreateAttrReadSample(m_attrRot, ref m_rotData, ref m_timeRead)
                            });
                        }
                        else
                        {
                            m_asyncRead =  new usdi.Task(usdi.usdiTaskCreatePointsReadSample(m_points, ref m_pointsData, ref m_timeRead));
                        }
                    }
                    m_timeRead = time;
                    m_asyncRead.Run();
                }
            }
        }

        public override void usdiUpdate(double time)
        {
            if (!m_needsUpdate) { return; }
            base.usdiUpdate(time);

            if(m_asyncRead != null)
            {
                m_asyncRead.Wait();
            }
        }
        #endregion


        #region callbacks
        #endregion
    }

}
