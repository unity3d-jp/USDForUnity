using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using UnityEngine;

#if usdiEnableComponentUpdator
namespace UTJ
{
    abstract class usdiIStreamUpdater
    {
        public struct Config
        {
            Bool forceSingleThread;
            Bool directVBUpdate;
        }

        abstract public void SetConfig(ref Config config);
        abstract public void ConstructScene();
        abstract public void OnLoad();
        abstract public void OnUnload();
        abstract public void AsyncUpdate(double time);
        abstract public void Update(double time);
    }


    class usdiStreamUpdater : usdiIStreamUpdater
    {
        IntPtr m_rep;

        public usdiStreamUpdater(usdi.Context usd, usdiStream stream) { m_rep = _Ctor(usd, stream); }
        ~usdiStreamUpdater() { _Dtor(m_rep); }

        override public void SetConfig(ref Config config) { _SetConfig(m_rep, ref config); }
        override public void ConstructScene() { _ConstructScene(m_rep); }
        override public void OnLoad() { _OnLoad(m_rep); }
        override public void OnUnload() { _OnUnload(m_rep); }
        override public void AsyncUpdate(double time) { _AsyncUpdate(m_rep, time); }
        override public void Update(double time) { _Update(m_rep, time); }

#region internal
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr _Ctor(usdi.Context usd, usdiStream stream);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void _Dtor(IntPtr rep);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void _SetConfig(IntPtr rep, ref Config config);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void _ConstructScene(IntPtr rep);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void _OnLoad(IntPtr rep);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void _OnUnload(IntPtr rep);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void _AsyncUpdate(IntPtr rep, double time);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void _Update(IntPtr rep, double time);
#endregion
    }


    class usdiStreamUpdaterM : usdiIStreamUpdater
    {
#region impl
        abstract class IUpdater
        {
            abstract public void OnLoad();
            abstract public void OnUnload();
            abstract public void AsyncUpdate(double time);
            abstract public void Update(double time);
        }

        class XformUpdater : IUpdater
        {
            usdi.Xform m_rep;
            usdi.XformData m_data;
            Transform m_trans;

            public XformUpdater(usdi.Xform rep, GameObject go)
            {
                m_rep = rep;
                m_trans = go.GetComponent<Transform>();
            }

            override public void OnLoad()
            {
            }

            override public void OnUnload()
            {
                m_rep = default(usdi.Xform);
            }

            override public void AsyncUpdate(double time)
            {
                usdi.usdiXformReadSample(m_rep, ref m_data, time);
            }

            override public void Update(double time)
            {
                usdi.usdiUniTransformAssign(m_trans, ref m_data);
            }
        }

        class CameraUpdater : XformUpdater
        {
            usdi.Camera m_rep;
            usdi.CameraData m_data;
            Camera m_cam;

            public CameraUpdater(usdi.Camera rep, GameObject go)
                : base(rep, go)
            {
                m_rep = rep;
                m_cam = usdi.GetOrAddComponent<Camera>(go);
            }

            override public void OnLoad()
            {
                base.OnLoad();
            }

            override public void OnUnload()
            {
                base.OnUnload();
                m_rep = default(usdi.Camera);
            }

            override public void AsyncUpdate(double time)
            {
                base.AsyncUpdate(time);
                usdi.usdiCameraReadSample(m_rep, ref m_data, time);
            }

            override public void Update(double time)
            {
                base.Update(time);
                usdi.usdiUniCameraAssign(m_cam, ref m_data);
            }
        }

        class MeshUpdater : XformUpdater
        {
            class Submesh
            {
                Vector3[] points;
                Vector3[] normals;
                Vector2[] uv;
                int[] indices;
                IntPtr vb;
                IntPtr ib;
            }


            usdi.Mesh m_rep;
            usdi.MeshData m_data;
            usdi.SubmeshData[] m_sdata;

            List<Submesh> m_submeshes;
            usdi.Task m_asyncRead;
            usdi.VertexUpdateCommand m_vuCmd;


            public MeshUpdater(usdi.Mesh rep, GameObject go)
                : base(rep, go)
            {
                m_rep = rep;
                m_submeshes = new List<Submesh>();
            }

            override public void OnLoad()
            {
                base.OnLoad();
            }

            override public void OnUnload()
            {
                base.OnUnload();
                m_rep = default(usdi.Mesh);
                m_submeshes = null;
            }

            override public void AsyncUpdate(double time)
            {
                base.AsyncUpdate(time);
            }

            override public void Update(double time)
            {
                base.Update(time);
            }
        }

        class PointsUpdater : XformUpdater
        {
            usdi.Points m_rep;
            usdi.PointsData m_data;

            public PointsUpdater(usdi.Points rep, GameObject go)
                : base(rep, go)
            {
                m_rep = rep;
            }

            override public void OnLoad()
            {
                base.OnLoad();
            }

            override public void OnUnload()
            {
                base.OnUnload();
                m_rep = default(usdi.Points);
            }

            override public void AsyncUpdate(double time)
            {
                base.AsyncUpdate(time);
                usdi.usdiPointsReadSample(m_rep, ref m_data, time, false);
            }

            override public void Update(double time)
            {
                base.Update(time);
            }
        }
#endregion impl


        List<IUpdater> m_children;
        Transform m_trans;


        public usdiStreamUpdaterM(usdi.Context usd, usdiStream stream)
        {
            m_trans = stream.GetComponent<Transform>();
        }

        ~usdiStreamUpdaterM()
        {
        }

        override public void SetConfig(ref Config config)
        {

        }

        override public void ConstructScene()
        {

        }

        override public void OnLoad()
        {
            int n = m_children.Count;
            for (int i = 0; i < n; ++i) { m_children[i].OnLoad(); }
        }

        override public void OnUnload()
        {
            int n = m_children.Count;
            for (int i = 0; i < n; ++i) { m_children[i].OnUnload(); }
        }

        override public void AsyncUpdate(double time)
        {
            int n = m_children.Count;
            for (int i = 0; i < n; ++i) { m_children[i].AsyncUpdate(time); }
        }

        override public void Update(double time)
        {
            int n = m_children.Count;
            for (int i = 0; i < n; ++i) { m_children[i].Update(time); }
            usdi.usdiUniTransformNotfyChange(m_trans);
        }
    }

}
#endif // usdiEnableComponentUpdator
