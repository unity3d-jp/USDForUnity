using System;
using UnityEngine;

namespace UTJ
{

    public class usdiCamera : usdiElement
    {
        usdi.Camera     m_camera;
        usdi.CameraData m_cameraData = usdi.CameraData.default_value;
        Camera          m_ucam;

        public override void usdiOnLoad(usdi.Schema schema)
        {
            m_camera = usdi.usdiAsCamera(schema);
            if(!m_camera)
            {
                Debug.LogWarning("schema is not Xform!");
            }

            m_ucam = GetOrAddComponent<Camera>();
        }

        public override void usdiOnUnload()
        {
            m_camera = default(usdi.Camera);
        }

        public override void usdiUpdate(double time)
        {
            if (!m_camera) { return; }

            if(usdi.usdiCameraReadSample(m_camera, ref m_cameraData, time))
            {
                m_ucam.nearClipPlane = m_cameraData.near_clipping_plane;
                m_ucam.farClipPlane = m_cameraData.far_clipping_plane;
                m_ucam.fieldOfView = m_cameraData.field_of_view;
                m_ucam.aspect = m_cameraData.aspect_ratio;
            }
        }
    }

}
