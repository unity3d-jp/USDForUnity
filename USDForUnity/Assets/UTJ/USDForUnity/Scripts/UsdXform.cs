using System;
using UnityEngine;

namespace UTJ.USD
{

    [Serializable]
    public class UsdXform : UsdSchema
    {
        #region fields
        [SerializeField] protected Transform transform;

        usdi.Xform m_xf;
        usdi.XformData m_xfData = usdi.XformData.defaultValue;
        protected usdi.UpdateFlags m_updateFlags;
        #endregion

        #region properties
        public usdi.Xform usdXform
        {
            get { return m_xf; }
        }
        #endregion

        #region impl
        protected override UsdIComponent UsdSetupSchemaComponent()
        {
            return GetOrAddComponent<UsdXformComponent>();
        }

        public override void UsdOnLoad()
        {
            base.UsdOnLoad();
            m_xf = m_schema.AsXform();
            transform = GetComponent<Transform>();
        }

        public override void UsdOnUnload()
        {
            base.UsdOnUnload();
            m_xf = default(usdi.Xform);
        }

        public override void UsdPrepareSample()
        {
            m_updateFlags = m_xf.schema.GetUpdateFlags();
            m_xf.ReadSample(ref m_xfData);
        }

        public override void UsdSyncDataEnd()
        {
            base.UsdSyncDataEnd();
            if(m_linkedGameObj != null)
            {
                if (m_xfData.flags.updatedPosition)
                    transform.localPosition = m_xfData.position;
                if (m_xfData.flags.updatedRotation)
                    transform.localRotation = m_xfData.rotation;
                if (m_xfData.flags.updatedScale)
                    transform.localScale = m_xfData.scale;
            }
        }
        #endregion
    }

}
