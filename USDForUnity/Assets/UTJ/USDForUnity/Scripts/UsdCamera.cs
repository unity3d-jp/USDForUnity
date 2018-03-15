using System;
using UnityEngine;

namespace UTJ.USD
{

    [Serializable]
    public class UsdCamera : UsdXform
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
        usdi.CameraData m_cameraData = usdi.CameraData.defaultValue;
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
        protected override UsdIComponent UsdSetupSchemaComponent()
        {
            return GetOrAddComponent<UsdCameraComponent>();
        }

        public override void UsdOnLoad()
        {
            base.UsdOnLoad();
            m_camera = m_schema.AsCamera();
            m_ucam = GetOrAddComponent<Camera>();
        }

        public override void UsdOnUnload()
        {
            base.UsdOnUnload();
            m_camera = default(usdi.Camera);
        }

        public override void UsdPrepareSample()
        {
            base.UsdPrepareSample();
            m_camera.ReadSample(ref m_cameraData);
        }

        public override void UsdSyncDataEnd()
        {
            base.UsdSyncDataEnd();

            if (m_linkedGameObj != null)
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
