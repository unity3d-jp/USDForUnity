using System;
using UnityEngine;

namespace UTJ.USD
{

    [Serializable]
    public class UsdPoints : UsdXform
    {
        #region fields
        usdi.Points     m_usdPoints;
        usdi.Attribute  m_usdAttrRot;
        usdi.PointsSummary m_summary = default(usdi.PointsSummary);
        usdi.PointsData m_pointsData;
        usdi.AttributeData m_rotData;

        PinnedList<Vector3> m_positions = new PinnedList<Vector3>();
        PinnedList<Vector3> m_velocities = new PinnedList<Vector3>();
        PinnedList<Vector4> m_rotations = new PinnedList<Vector4>();
        PinnedList<int> m_ids = new PinnedList<int>();

        usdi.Task m_asyncRead;
        double m_timeRead;
        #endregion


        #region properties
        public usdi.Points usdPoints { get { return m_usdPoints; } }
        public PinnedList<Vector3> positions { get { return m_positions; } }
        public PinnedList<Vector3> velocities { get { return m_velocities; } }
        public PinnedList<Vector4> rotations { get { return m_rotations; } }
        public PinnedList<int> ids { get { return m_ids; } }
        #endregion


        #region impl
        protected override UsdIComponent usdiSetupSchemaComponent()
        {
            return GetOrAddComponent<UsdPointsComponent>();
        }

        public override void usdiOnLoad()
        {
            base.usdiOnLoad();

            m_usdPoints = usdi.usdiAsPoints(m_schema);
            usdi.usdiPointsGetSummary(m_usdPoints, ref m_summary);
            m_usdAttrRot = usdi.usdiPrimFindAttribute(m_usdPoints, "rotations");
        }

        public override void usdiOnUnload()
        {
            base.usdiOnUnload();

            m_asyncRead = null;

            m_usdPoints = default(usdi.Points);
            m_summary = default(usdi.PointsSummary);
            m_pointsData = default(usdi.PointsData);
            m_usdAttrRot = default(usdi.Attribute);
            m_rotData = default(usdi.AttributeData);
        }

        public override void usdiAsyncUpdate(double time)
        {
            base.usdiAsyncUpdate(time);
            if (m_updateFlags.bits == 0) { return; }

            usdi.PointsData tmp = usdi.PointsData.default_value;
            usdi.usdiPointsReadSample(m_usdPoints, ref tmp, time, true);

            // allocate points data
            if (m_pointsData.num_points == tmp.num_points)
            {
                // no need to allocate
            }
            else
            {
                m_pointsData.num_points = tmp.num_points;

                m_positions.ResizeDiscard(m_pointsData.num_points);
                m_pointsData.points = m_positions;

                if (m_summary.has_velocities)
                {
                    m_velocities.ResizeDiscard(m_pointsData.num_points);
                    m_pointsData.velocities = m_velocities;
                }
                if (m_usdAttrRot)
                {
                    m_rotations.ResizeDiscard(m_pointsData.num_points);
                    m_rotData.data = m_rotations;
                    m_rotData.num_elements = tmp.num_points;
                }
                // todo: ids
            }

            // read points data
            if (m_pointsData.num_points > 0)
            {

#if UNITY_EDITOR
                if (m_stream.forceSingleThread)
                {
                    usdi.usdiPointsReadSample(m_usdPoints, ref m_pointsData, m_timeRead, true);
                    if (m_usdAttrRot)
                    {
                        usdi.usdiAttrReadSample(m_usdAttrRot, ref m_rotData, m_timeRead, true);
                    }
                }
                else
#endif
                {
                    if (m_asyncRead == null)
                    {
                        if(m_usdAttrRot)
                        {
                            m_asyncRead = new usdi.CompositeTask(new IntPtr[] {
                                usdi.usdiTaskCreatePointsReadSample(m_usdPoints, ref m_pointsData, ref m_timeRead),
                                usdi.usdiTaskCreateAttrReadSample(m_usdAttrRot, ref m_rotData, ref m_timeRead)
                            });
                        }
                        else
                        {
                            m_asyncRead =  new usdi.Task(usdi.usdiTaskCreatePointsReadSample(m_usdPoints, ref m_pointsData, ref m_timeRead));
                        }
                    }
                    m_timeRead = time;
                    m_asyncRead.Run();
                }
            }
        }

        public override void usdiUpdate(double time)
        {
            if (m_updateFlags.bits == 0) { return; }
            base.usdiUpdate(time);

            usdiSync();
        }

        public override void usdiSync()
        {
            if (m_asyncRead != null)
            {
                m_asyncRead.Wait();
            }
        }
        #endregion


        #region callbacks
        #endregion
    }

}
