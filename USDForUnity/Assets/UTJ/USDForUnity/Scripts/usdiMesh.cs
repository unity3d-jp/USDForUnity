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
        class MeshBuffer
        {
            public Vector3[] positions;
            public Vector3[] normals;
            public Vector2[] uvs;
            public int[] indices;

            public void Allocate(ref usdi.MeshSummary summary, ref usdi.MeshData md)
            {
                {
                    positions = new Vector3[md.num_points];
                    md.points = usdi.GetArrayPtr(positions);
                }
                if (summary.has_normals)
                {
                    normals = new Vector3[md.num_points];
                    md.normals = usdi.GetArrayPtr(normals);
                }
                if (summary.has_uvs)
                {
                    uvs = new Vector2[md.num_points];
                    md.uvs = usdi.GetArrayPtr(uvs);
                }
                {
                    indices = new int[md.num_indices_triangulated];
                    md.indices_triangulated = usdi.GetArrayPtr(indices);
                }
            }

            public void Clear()
            {
                positions = null;
                normals = null;
                uvs = null;
                indices = null;
            }
        }


        #region fields
        usdi.Mesh m_mesh;
        usdi.MeshData m_meshData;
        usdi.MeshSummary m_meshSummary;
        MeshBuffer m_buf = new MeshBuffer();

        Renderer m_renderer;
        Mesh m_umesh;
        Mesh[] m_childMeshes;
        bool m_needsAllocateMeshData;
        bool m_needsUploadMeshData;

        int m_meshVertexCount;
        int m_prevVertexCount;
        int m_prevIndexCount;
        int m_frame;
        double m_timeRead;

        List<usdiSplitMesh> m_children = new List<usdiSplitMesh>();
        usdi.SplitedMeshData[] m_splitedData;

        usdi.Task m_asyncRead;
        usdi.VertexUpdateCommand m_vuCmd;

        // for Unity 5.5 or later
        bool m_directVBUpdate;
        IntPtr m_VB, m_IB;
        #endregion



        #region properties
        public usdi.MeshSummary meshSummary { get { return m_meshSummary; } }
        public usdi.MeshData meshData { get { return m_meshData; } }
        public bool directVBUpdate { get { return m_directVBUpdate; } }
        #endregion



        #region impl
        Mesh usdiGetOrAddMeshComponents()
        {
            Mesh mesh = null;

            var go = gameObject;
            MeshFilter meshFilter = go.GetComponent<MeshFilter>();

            if (meshFilter == null || meshFilter.sharedMesh == null)
            {
                mesh = new Mesh();

                if (meshFilter == null)
                {
                    meshFilter = go.AddComponent<MeshFilter>();
                }

                meshFilter.sharedMesh = mesh;

                MeshRenderer renderer = go.GetComponent<MeshRenderer>();

                if (renderer == null)
                {
                    renderer = go.AddComponent<MeshRenderer>();
                }

#if UNITY_EDITOR
                Material material = UnityEngine.Object.Instantiate(GetDefaultMaterial());
                material.name = "Material_0";
                renderer.sharedMaterial = material;
#endif
            }
            else
            {
                mesh = meshFilter.sharedMesh;
            }

            mesh.MarkDynamic();
            return mesh;
        }

        usdiSplitMesh usdiAddSplit()
        {
            usdiSplitMesh split;
            if (m_children.Count == 0)
            {
                split = GetOrAddComponent<usdiSplitMesh>();
            }
            else
            {
                var child = new GameObject("Split[" + m_children.Count + "]");
                child.GetComponent<Transform>().SetParent(GetComponent<Transform>(), false);
                split = child.AddComponent<usdiSplitMesh>();
            }
            split.usdiOnLoad(this, m_children.Count);
            m_children.Add(split);
            return split;
        }

        void usdiAllocateChildMeshes(int n)
        {
            if(n <= m_children.Count + 1) { return; }

            while(m_children.Count + 1 < n)
            {
                usdiAddSplit();
            }
        }


        void usdiGatherExistingSplits()
        {
            {
                var child = GetComponent<usdiSplitMesh>();
                if(child != null)
                {
                    m_children.Add(child);
                }
            }

            var t = GetComponent<Transform>();
            for (int i=1; ; ++i)
            {
                var child = t.FindChild("Split[" + i + "]");
                if (child == null) { break; }

                var split = child.GetComponent<usdiSplitMesh>();
                if (child != null) { m_children.Add(split); }
            }
        }

#if UNITY_EDITOR
        static MethodInfo s_GetBuiltinExtraResourcesMethod;
        public static Material GetDefaultMaterial()
        {
            if (s_GetBuiltinExtraResourcesMethod == null)
            {
                BindingFlags bfs = BindingFlags.NonPublic | BindingFlags.Static;
                s_GetBuiltinExtraResourcesMethod = typeof(EditorGUIUtility).GetMethod("GetBuiltinExtraResource", bfs);
            }
            return (Material)s_GetBuiltinExtraResourcesMethod.Invoke(null, new object[] { typeof(Material), "Default-Material.mat" });
        }
#endif

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
            m_umesh = usdiGetOrAddMeshComponents();
            m_meshVertexCount = m_umesh.vertexCount;

            m_renderer = GetComponent<MeshRenderer>();

            usdiGatherExistingSplits();
            for (int i = 0; i < m_children.Count; ++i)
            {
                m_children[i].usdiOnLoad(this, m_children.Count);
            }
        }

        public override void usdiOnUnload()
        {
            base.usdiOnUnload();

            if(m_asyncRead != null)
            {
                m_asyncRead.Wait();
                m_asyncRead = null;
            }

            m_mesh = default(usdi.Mesh);
            m_buf.Clear();
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
            if (m_meshData.num_splits != 0)
            {
                m_splitedData = new usdi.SplitedMeshData[m_meshData.num_splits];
                m_meshData.splits = usdi.GetArrayPtr(m_splitedData);
                usdi.usdiMeshReadSample(m_mesh, ref m_meshData, t, true);

                while (m_children.Count < m_meshData.num_splits)
                {
                    usdiAddSplit();
                }

                for (int i = 0; i < m_children.Count; ++i)
                {
                    m_children[i].usdiAllocateMeshData(ref m_splitedData[i]);
                }
            }
            else
            {
                m_buf.Allocate(ref m_meshSummary, ref m_meshData);
            }
        }

        // async
        void usdiReadMeshData(double t)
        {
            if (m_directVBUpdate)
            {
                usdi.usdiMeshReadSample(m_mesh, ref m_meshData, t, false);
            }
            else
            {
#if UNITY_EDITOR
                if (m_stream.forceSingleThread)
                {
                    usdi.usdiMeshReadSample(m_mesh, ref m_meshData, t, true);
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

        // sync
        void usdiUploadMeshData(double t, bool topology, bool close)
        {
            if (m_directVBUpdate) { return; }

            if (m_meshData.num_splits != 0)
            {
                for (int i = 0; i < m_children.Count; ++i)
                {
                    m_children[i].usdiUploadMeshData(topology, close);
                }
            }
            else
            {
                m_umesh.vertices = m_buf.positions;
                if (m_meshSummary.has_normals)
                {
                    m_umesh.normals = m_buf.normals;
                }
                if (m_meshSummary.has_uvs)
                {
                    m_umesh.uv = m_buf.uvs;
                }

                if (topology)
                {
                    m_umesh.SetIndices(m_buf.indices, MeshTopology.Triangles, 0);
                }

                m_umesh.UploadMeshData(close);
                m_meshVertexCount = m_umesh.vertexCount;

#if UNITY_5_5_OR_NEWER
                if (m_stream.directVBUpdate)
                {
                    m_VB = m_umesh.GetNativeVertexBufferPtr(0);
                    //m_IB = m_umesh.GetNativeIndexBufferPtr();
                }
#endif
            }

            m_prevVertexCount = m_meshData.num_points;
            m_prevIndexCount = m_meshData.num_indices_triangulated;
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
                m_meshVertexCount == 0 ||
                (m_buf.positions == null && m_meshSummary.topology_variance != usdi.TopologyVariance.Constant) ||
                m_meshSummary.topology_variance == usdi.TopologyVariance.Heterogenous;

            m_needsUploadMeshData =
                m_needsAllocateMeshData ||
                m_meshSummary.topology_variance != usdi.TopologyVariance.Constant;

            // todo: update heterogenous mesh if possible
            m_directVBUpdate =
                m_stream.directVBUpdate && !m_needsAllocateMeshData && m_VB != IntPtr.Zero &&
                m_meshSummary.topology_variance == usdi.TopologyVariance.Homogenous;

            if (m_needsAllocateMeshData) { usdiAllocateMeshData(time); }
            if (m_needsUploadMeshData) { usdiReadMeshData(time); }
        }

        // sync
        public override void usdiUpdate(double time)
        {
            if (!m_needsUpdate) { return; }
            base.usdiUpdate(time);

            if (m_asyncRead != null)
            {
                m_asyncRead.Wait();
            }

            if(m_needsUploadMeshData)
            {
                usdi.usdiUniMeshAssignBounds(m_umesh, ref m_meshData.center, ref m_meshData.extents);

                //// fall back
                // m_umesh.bounds = new Bounds(m_meshData.center, m_meshData.extents);
            }

            if (m_needsAllocateMeshData && m_frame == 0)
            {
                bool close = m_meshSummary.topology_variance == usdi.TopologyVariance.Constant;
                usdiUploadMeshData(time, true, close);
            }

            bool active = isActiveAndEnabled && m_renderer.isVisible;
            if (m_directVBUpdate)
            {
                if (m_vuCmd == null)
                {
                    m_vuCmd = new usdi.VertexUpdateCommand(usdi.usdiGetNameS(m_mesh));
                }

                if (active)
                {
                    m_vuCmd.Update(ref m_meshData, m_VB, m_IB);
                }
            }
            else
            {
                if (active && m_needsUploadMeshData && m_frame > 1 && !m_directVBUpdate)
                {
                    bool updateIndices = m_meshSummary.topology_variance == usdi.TopologyVariance.Heterogenous;
                    usdiUploadMeshData(m_timeRead, updateIndices, false);
                }
            }

            ++m_frame;
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
