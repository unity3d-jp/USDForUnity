using System;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{

    [ExecuteInEditMode]
    public class usdiMesh : usdiXform
    {
        #region fields
        usdi.Mesh m_mesh;
        usdi.MeshData m_meshData;
        usdi.SubmeshData[] m_submeshData;
        usdi.MeshSummary m_meshSummary;

        bool m_allocateMeshDataRequired;
        bool m_updateMeshDataRequired;
        bool m_directVBUpdate; // for Unity 5.5 or later
        double m_timeRead; // accessed from worker thread

        List<usdiSubmesh> m_submeshes = new List<usdiSubmesh>();
        usdi.Task m_asyncRead;
        #endregion


        #region properties
        public usdi.MeshSummary meshSummary { get { return m_meshSummary; } }
        public usdi.MeshData meshData {
            get { return m_meshData; }
            set { m_meshData = value; }
        }
        public usdi.SubmeshData[] submeshData { get { return m_submeshData; } }
        public List<usdiSubmesh> submeshes { get { return m_submeshes; } }
        public bool directVBUpdate { get { return m_directVBUpdate; } }
        #endregion


        #region impl
        usdiSubmesh usdiAddSubmesh()
        {
            var sm = new usdiSubmesh();
            sm.usdiOnLoad(this, m_submeshes.Count);
            m_submeshes.Add(sm);
            return sm;
        }

        public override void usdiOnLoad(usdi.Schema schema)
        {
            base.usdiOnLoad(schema);

            m_mesh = usdi.usdiAsMesh(schema);
            usdi.usdiMeshGetSummary(m_mesh, ref m_meshSummary);
            if(m_submeshes.Count == 0)
            {
                usdiAddSubmesh();
            }
            m_allocateMeshDataRequired = true;
            m_updateMeshDataRequired = true;
        }

        public override bool usdiOnReload()
        {
            if (!base.usdiOnReload()) { return false; }

            m_mesh = usdi.usdiAsMesh(m_schema);
            usdi.usdiMeshGetSummary(m_mesh, ref m_meshSummary);
            if (m_submeshes.Count == 0)
            {
                usdiAddSubmesh();
            }
            m_allocateMeshDataRequired = true;
            m_updateMeshDataRequired = true;
            return true;
        }

        public override void usdiOnUnload()
        {
            base.usdiOnUnload();

            int c = m_submeshes.Count;
            for (int i = 0; i < c; ++i) { m_submeshes[i].usdiOnUnload(); }

            m_meshData = usdi.MeshData.default_value;
            m_submeshData = null;
            m_meshSummary = usdi.MeshSummary.default_value;

            m_allocateMeshDataRequired = false;
            m_updateMeshDataRequired = false;
            m_timeRead = 0.0;

            m_asyncRead = null;
        }


        // async
        void usdiAllocateMeshData(double t)
        {
            usdi.MeshData md = default(usdi.MeshData);
            usdi.usdiMeshReadSample(m_mesh, ref md, t, true);

            m_meshData = md;
            if (m_meshData.num_submeshes == 0)
            {
                m_submeshes[0].usdiAllocateMeshData(ref m_meshSummary, ref m_meshData);
            }
            else
            {
                m_submeshData = new usdi.SubmeshData[m_meshData.num_submeshes];
                m_meshData.submeshes = usdi.GetArrayPtr(m_submeshData);
                usdi.usdiMeshReadSample(m_mesh, ref m_meshData, t, true);

                while (m_submeshes.Count < m_meshData.num_submeshes)
                {
                    usdiAddSubmesh();
                }
                for (int i = 0; i < m_meshData.num_submeshes; ++i)
                {
                    m_submeshes[i].usdiAllocateMeshData(ref m_meshSummary, ref m_submeshData);
                }
            }
        }

        // async
        public override void usdiAsyncUpdate(double time)
        {
            base.usdiAsyncUpdate(time);
            if (m_updateFlags.bits == 0 && !m_allocateMeshDataRequired && !m_updateMeshDataRequired) {
                return;
            }

            m_timeRead = time;
            if(!m_allocateMeshDataRequired)
            {
                m_allocateMeshDataRequired =
                    m_meshSummary.topology_variance == usdi.TopologyVariance.Heterogenous ||
                    m_updateFlags.variantSetChanged;
            }
            if(!m_updateMeshDataRequired)
            {
                m_updateMeshDataRequired =
                    m_allocateMeshDataRequired ||
                    m_meshSummary.topology_variance != usdi.TopologyVariance.Constant;
            }

            // todo: update heterogenous mesh when possible
            m_directVBUpdate =
#if UNITY_5_5_OR_NEWER
                m_stream.directVBUpdate && !m_allocateMeshDataRequired &&
                m_meshSummary.topology_variance == usdi.TopologyVariance.Homogenous;
#else
                false;
#endif

            if (m_allocateMeshDataRequired)
            {
                usdiAllocateMeshData(m_timeRead);
            }

            if (m_updateMeshDataRequired)
            {
                if (m_directVBUpdate)
                {
                    usdi.usdiMeshReadSample(m_mesh, ref m_meshData, m_timeRead, false);
                }
                else
                {
#if UNITY_EDITOR
                    if (m_stream.forceSingleThread)
                    {
                        usdi.usdiMeshReadSample(m_mesh, ref m_meshData, m_timeRead, true);
                    }
                    else
#endif
                    {
                        if (m_asyncRead == null)
                        {
                            m_asyncRead = new usdi.Task(usdi.usdiTaskCreateMeshReadSample(m_mesh, ref m_meshData, ref m_timeRead));
                        }
                        m_asyncRead.Run();
                    }
                }
            }
        }


        // sync
        void usdiUploadMeshData(double t, bool topology, bool close)
        {
            int num_submeshes = m_meshData.num_submeshes == 0 ? 1 : m_meshData.num_submeshes;
            for (int i = 0; i < num_submeshes; ++i)
            {
                m_submeshes[i].usdiUploadMeshData(topology, close);
            }
        }

        // sync
        public override void usdiUpdate(double time)
        {
            if (m_updateFlags.bits == 0 && !m_allocateMeshDataRequired && !m_updateMeshDataRequired)
            {
                return;
            }
            base.usdiUpdate(time);

            usdiSync();

            int num_submeshes = m_meshData.num_submeshes == 0 ? 1 : m_meshData.num_submeshes;

            for (int i = 0; i < num_submeshes; ++i)
            {
                m_submeshes[i].usdiSetupComponents();
            }

            if( num_submeshes > 1 &&
                m_meshSummary.topology_variance == usdi.TopologyVariance.Heterogenous)
            {
                // number of active submeshes may change over time if topology is dynamic.
                for (int i = 0; i < num_submeshes; ++i)
                {
                    m_submeshes[i].usdiSetActive(true);
                }
                for (int i = num_submeshes; i < m_submeshes.Count; ++i)
                {
                    m_submeshes[i].usdiSetActive(false);
                }
            }

            if (m_updateMeshDataRequired)
            {
                if(m_meshData.num_submeshes == 0)
                {
                    m_submeshes[0].usdiUpdateBounds(ref m_meshData);
                }
                else
                {
                    for (int i = 0; i < num_submeshes; ++i)
                    {
                        m_submeshes[i].usdiUpdateBounds(ref m_submeshData[i]);
                    }
                }
            }

            if (m_allocateMeshDataRequired)
            {
                bool close = m_meshSummary.topology_variance == usdi.TopologyVariance.Constant;
                usdiUploadMeshData(time, true, close);
            }
            else if(m_updateMeshDataRequired)
            {
                if (m_directVBUpdate)
                {
                    if (m_meshData.num_submeshes == 0)
                    {
                        m_submeshes[0].usdiKickVBUpdateTask(ref m_meshData);
                    }
                    else
                    {
                        for (int i = 0; i < num_submeshes; ++i)
                        {
                            m_submeshes[i].usdiKickVBUpdateTask(ref m_submeshData[i]);
                        }
                    }
                }
                else
                {
                    bool updateIndices = m_meshSummary.topology_variance == usdi.TopologyVariance.Heterogenous;
                    usdiUploadMeshData(m_timeRead, updateIndices, false);
                }
            }

            m_allocateMeshDataRequired = false;
            m_updateMeshDataRequired = false;
        }

        public override void usdiSync()
        {
            if (m_asyncRead != null)
            {
                m_asyncRead.Wait();
            }
        }

        #endregion


        #region callbacks
        //// bounds debug
        //void OnDrawGizmos()
        //{
        //    if (!enabled || m_umesh == null) return;
        //    var t = GetComponent<Transform>();
        //    var b = m_umesh.bounds;
        //    Gizmos.color = Color.cyan;
        //    Gizmos.matrix = t.localToWorldMatrix;
        //    Gizmos.DrawWireCube(b.center, b.extents * 2.0f);
        //    Gizmos.matrix = Matrix4x4.identity;
        //}
        #endregion
    }

}
