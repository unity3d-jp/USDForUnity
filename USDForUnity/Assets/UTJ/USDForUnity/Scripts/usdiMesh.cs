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

        bool m_needsAllocateMeshData;
        bool m_needsUploadMeshData;

        int m_prevVertexCount;
        int m_prevIndexCount;
        int m_frame;
        double m_timeRead;

        List<usdiSubmesh> m_submeshes = new List<usdiSubmesh>();
        usdi.Task m_asyncRead;

        // for Unity 5.5 or later
        bool m_directVBUpdate;
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
            if (!m_mesh)
            {
                Debug.LogWarning("schema is not Mesh!");
                return;
            }

            usdi.usdiMeshGetSummary(m_mesh, ref m_meshSummary);
            usdiAddSubmesh();
        }

        public override void usdiOnUnload()
        {
            base.usdiOnUnload();

            usdiSync();
            m_asyncRead = null;
            m_mesh = default(usdi.Mesh);
        }


        // async
        void usdiAllocateMeshData(double t)
        {
            usdi.MeshData md = default(usdi.MeshData);
            usdi.usdiMeshReadSample(m_mesh, ref md, t, true);

            // skip if already allocated
            if (m_prevVertexCount == md.num_points &&
                m_prevIndexCount == md.num_indices_triangulated)
            {
                return;
            }

            m_meshData = md;
            if (m_meshData.num_submeshes == 0)
            {
                m_submeshes[0].usdiAllocateMeshData();
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
                    m_submeshes[i].usdiAllocateMeshData();
                }
            }
        }

        // async
        public override void usdiAsyncUpdate(double time)
        {
            base.usdiAsyncUpdate(time);
            if (!m_needsUpdate) {
                m_needsAllocateMeshData = false;
                m_needsUploadMeshData = false;
                return;
            }

            m_timeRead = time;

            m_needsAllocateMeshData =
                m_prevVertexCount == 0 ||
                m_meshSummary.topology_variance == usdi.TopologyVariance.Heterogenous;

            m_needsUploadMeshData =
                m_needsAllocateMeshData ||
                m_meshSummary.topology_variance != usdi.TopologyVariance.Constant;

            // todo: update heterogenous mesh when possible
            m_directVBUpdate =
#if UNITY_5_5_OR_NEWER
                m_prevVertexCount != 0 &&
                m_stream.directVBUpdate && !m_needsAllocateMeshData &&
                m_meshSummary.topology_variance == usdi.TopologyVariance.Homogenous;
#else
                false;
#endif

            if (m_needsAllocateMeshData)
            {
                usdiAllocateMeshData(m_timeRead);
            }

            if (m_needsUploadMeshData)
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
            if (m_directVBUpdate) { return; }

            int num_submeshes = m_meshData.num_submeshes == 0 ? 1 : m_meshData.num_submeshes;
            for (int i = 0; i < num_submeshes; ++i)
            {
                m_submeshes[i].usdiUploadMeshData(topology, close);
            }

            m_prevVertexCount = m_meshData.num_points;
            m_prevIndexCount = m_meshData.num_indices_triangulated;
        }

        // sync
        public override void usdiUpdate(double time)
        {
            if (!m_needsUpdate) { return; }
            base.usdiUpdate(time);

            usdiSync();

            int num_submeshes = m_meshData.num_submeshes == 0 ? 1 : m_meshData.num_submeshes;

            for (int i = 0; i < num_submeshes; ++i)
            {
                m_submeshes[i].usdiSetupMeshComponents();
            }

            if(num_submeshes > 1 && m_meshSummary.topology_variance == usdi.TopologyVariance.Heterogenous)
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

            if (m_needsUploadMeshData)
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

            if (m_needsAllocateMeshData && m_frame == 0)
            {
                bool close = m_meshSummary.topology_variance == usdi.TopologyVariance.Constant;
                usdiUploadMeshData(time, true, close);
            }
            else
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
                    if (m_needsUploadMeshData && m_frame > 1 && !m_directVBUpdate)
                    {
                        bool updateIndices = m_meshSummary.topology_variance == usdi.TopologyVariance.Heterogenous;
                        usdiUploadMeshData(m_timeRead, updateIndices, false);
                    }
                }
            }

            ++m_frame;
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
