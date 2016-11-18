using System;
using UnityEngine;

namespace UTJ
{

    public class usdiXform : usdiElement
    {
        #region fields
        usdi.Xform      m_xf;
        usdi.XformData m_xfData = usdi.XformData.default_value;
        Transform m_trans;
        protected bool m_needsUpdate = true;
        #endregion


        #region impl
        public override void usdiOnLoad(usdi.Schema schema)
        {
            base.usdiOnLoad(schema);
            m_xf = usdi.usdiAsXform(schema);
            m_trans = GetComponent<Transform>();
        }

        public override void usdiOnUnload()
        {
            m_xf = default(usdi.Xform);
        }

        public override void usdiAsyncUpdate(double time)
        {
            m_needsUpdate = usdi.usdiPrimNeedsUpdate(m_xf);
            usdi.usdiXformReadSample(m_xf, ref m_xfData, time);
        }

        public override void usdiUpdate(double time)
        {
            usdi.usdiUniTransformAssign(m_trans, ref m_xfData);

            //// fall back
            //if ((m_xfData.flags & (int)usdi.XformData.Flags.UpdatedPosition) != 0)
            //{
            //    m_trans.localPosition = m_xfData.position;
            //}
            //if ((m_xfData.flags & (int)usdi.XformData.Flags.UpdatedRotation) != 0)
            //{
            //    m_trans.localRotation = m_xfData.rotation;
            //}
            //if ((m_xfData.flags & (int)usdi.XformData.Flags.UpdatedScale) != 0)
            //{
            //    m_trans.localScale = m_xfData.scale;
            //}
        }
        #endregion
    }

}
