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

        public override void UsdOnLoad()
        {
            base.UsdOnLoad();

            m_usdPoints = usdi.usdiAsPoints(m_schema);
            usdi.usdiPointsGetSummary(m_usdPoints, ref m_summary);
            m_usdAttrRot = usdi.usdiPrimFindAttribute(m_usdPoints, "rotations");
        }

        public override void UsdOnUnload()
        {
            base.UsdOnUnload();

            m_usdPoints = default(usdi.Points);
            m_summary = default(usdi.PointsSummary);
            m_pointsData = default(usdi.PointsData);
            m_usdAttrRot = default(usdi.Attribute);
            m_rotData = default(usdi.AttributeData);
        }

        public override void UsdAsyncUpdate(double time)
        {
            base.UsdAsyncUpdate(time);
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
                usdi.usdiPointsReadSample(m_usdPoints, ref m_pointsData, time, true);
                if (m_usdAttrRot)
                {
                    usdi.usdiAttrReadSample(m_usdAttrRot, ref m_rotData, time, true);
                }
            }
        }

        public override void UsdUpdate(double time)
        {
            if (m_updateFlags.bits == 0) { return; }
            base.UsdUpdate(time);

            UsdSync();
        }

        public override void UsdSync()
        {
        }
        #endregion


        #region callbacks
        #endregion
    }

}
