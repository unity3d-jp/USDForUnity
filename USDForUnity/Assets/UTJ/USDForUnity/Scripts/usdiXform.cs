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
        #endregion

        #region impl
        public override void usdiOnLoad(usdi.Schema schema)
        {
            m_xf = usdi.usdiAsXform(schema);
            if(!m_xf)
            {
                Debug.LogWarning("schema is not Xform!");
            }

            m_trans = GetComponent<Transform>();
        }

        public override void usdiOnUnload()
        {
            m_xf = default(usdi.Xform);
        }

        public override void usdiAsyncUpdate(double time)
        {
            usdi.usdiXformReadSample(m_xf, ref m_xfData, time);
        }

        public override void usdiUpdate(double time)
        {
            if((m_xfData.flags & (int)usdi.XformData.Flags.UpdatedPosition) != 0)
            {
                m_trans.localPosition = m_xfData.position;
            }
            if ((m_xfData.flags & (int)usdi.XformData.Flags.UpdatedRotation) != 0)
            {
                m_trans.localRotation = m_xfData.rotation;
            }
            if ((m_xfData.flags & (int)usdi.XformData.Flags.UpdatedScale) != 0)
            {
                m_trans.localScale = m_xfData.scale;
            }
        }
        #endregion
    }

}
