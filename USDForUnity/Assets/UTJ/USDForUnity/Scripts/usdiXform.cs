using System;
using UnityEngine;

namespace UTJ
{

    public class usdiXform : usdiElement
    {
        usdi.Xform      m_xf;
        usdi.XformData  m_xfData;
        Transform       m_trans;

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
            if (!m_xf) { return; }

            m_trans.localPosition = m_xfData.position;
            m_trans.localRotation = m_xfData.rotation;
            m_trans.localScale = m_xfData.scale;
        }
    }

}
