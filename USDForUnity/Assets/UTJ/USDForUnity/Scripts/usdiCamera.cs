using System;
using UnityEngine;

namespace UTJ
{

    [Serializable]
    public class usdiCamera : usdiXform
    {
        public enum AspectRatioMode
        {
            Screen,
            USD,
        }

        #region fields
        [SerializeField] AspectRatioMode m_aspectRatioMode;
        [SerializeField] Camera m_ucam;

        usdi.Camera m_camera;
        usdi.CameraData m_cameraData = usdi.CameraData.default_value;
        #endregion


        #region properties
        public usdi.Camera nativeCameraPtr
        {
            get { return m_camera; }
        }
        public AspectRatioMode aspectRatioMode
        {
            get { return m_aspectRatioMode; }
            set { m_aspectRatioMode = value; }
        }
        #endregion


        #region impl
        protected override usdiIElement usdiSetupSchemaComponent()
        {
            return GetOrAddComponent<usdiCameraElement>();
        }

        public override void usdiOnLoad()
        {
            base.usdiOnLoad();
            m_camera = usdi.usdiAsCamera(m_schema);
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
            if (m_updateFlags.bits == 0) { return; }
            usdi.usdiCameraReadSample(m_camera, ref m_cameraData, time);
        }

        public override void usdiUpdate(double time)
        {
            if (m_updateFlags.bits == 0) { return; }
            base.usdiUpdate(time);

            if (m_goAssigned)
            {
                m_ucam.nearClipPlane = m_cameraData.near_clipping_plane;
                m_ucam.farClipPlane = m_cameraData.far_clipping_plane;
                m_ucam.fieldOfView = m_cameraData.field_of_view;

                if (m_aspectRatioMode == AspectRatioMode.USD)
                {
                    m_ucam.aspect = m_cameraData.aspect_ratio;
                }
                else
                {
                    m_ucam.ResetAspect();
                }
            }
        }
        #endregion
    }
}
