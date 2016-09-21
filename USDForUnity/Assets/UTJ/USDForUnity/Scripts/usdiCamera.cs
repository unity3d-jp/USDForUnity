using System;
using UnityEngine;

namespace UTJ
{

    public class usdiCamera : usdiXform
    {
        public enum AcpectRatioMode
        {
            Screen,
            USD,
        }

        #region fields
        public AcpectRatioMode m_acpectRatioMode;

        usdi.Camera m_camera;
        usdi.CameraData m_cameraData = usdi.CameraData.default_value;
        Camera m_ucam;
        #endregion

        #region impl
        public override void usdiOnLoad(usdi.Schema schema)
        {
            base.usdiOnLoad(schema);

            m_camera = usdi.usdiAsCamera(schema);
            if(!m_camera)
            {
                Debug.LogWarning("schema is not Xform!");
            }

            m_ucam = GetOrAddComponent<Camera>();
        }

        public override void usdiOnUnload()
        {
            base.usdiOnUnload();
            m_camera = default(usdi.Camera);
        }

        public override void usdiAsyncUpdate(double time)
        {
            base.usdiAsyncUpdate(time);
            if (!m_needsUpdate) { return; }
            usdi.usdiCameraReadSample(m_camera, ref m_cameraData, time);
        }

        public override void usdiUpdate(double time)
        {
            if (!m_needsUpdate) { return; }
            base.usdiUpdate(time);

            m_ucam.nearClipPlane = m_cameraData.near_clipping_plane;
            m_ucam.farClipPlane = m_cameraData.far_clipping_plane;
            m_ucam.fieldOfView = m_cameraData.field_of_view;

            if (m_acpectRatioMode == AcpectRatioMode.USD)
            {
                m_ucam.aspect = m_cameraData.aspect_ratio;
            }
            else
            {
                m_ucam.ResetAspect();
            }
        }
        #endregion
    }
}
