using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using UnityEngine;

namespace UTJ
{
    abstract class usdiIUpdator
    {
        protected IntPtr m_rep;

        public abstract void Update(double time);
    }

    class usdiTransformUpdator : usdiIUpdator
    {
        public usdiTransformUpdator(usdi.Xform usd, Transform trans) { m_rep = InternalCtor(usd, trans); }
        ~usdiTransformUpdator() { InternalDtor(m_rep); }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern IntPtr InternalCtor(usdi.Xform usd, Transform trans);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern void InternalDtor(IntPtr nobj);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern override void Update(double time);
    }

    class usdiCameraUpdator : usdiIUpdator
    {
        public usdiCameraUpdator(usdi.Camera usd, Camera cam) { m_rep = InternalCtor(usd, cam); }
        ~usdiCameraUpdator() { InternalDtor(m_rep); }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern IntPtr InternalCtor(usdi.Xform usd, Camera cam);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern void InternalDtor(IntPtr nobj);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern override void Update(double time);
    }


    class usdiMeshUpdator : usdiIUpdator
    {
        public usdiMeshUpdator(usdi.Mesh usd) { m_rep = InternalCtor(usd); }
        ~usdiMeshUpdator() { InternalDtor(m_rep); }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern IntPtr InternalCtor(usdi.Mesh usd);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern void InternalDtor(IntPtr nobj);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern override void Update(double time);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetMesh(int nth, Mesh mesh);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void RunCopyTask();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void WaitCopyTask();
    }

    class usdiParticleUpdator : usdiIUpdator
    {
        public usdiParticleUpdator(usdi.Camera usd) { m_rep = InternalCtor(usd); }
        ~usdiParticleUpdator() { InternalDtor(m_rep); }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern IntPtr InternalCtor(usdi.Xform usd);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern void InternalDtor(IntPtr nobj);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern override void Update(double time);
    }

}