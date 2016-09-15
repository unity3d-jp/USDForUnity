using System;
using System.Runtime.InteropServices;
using System.Reflection;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{

    public class usdiMesh : usdiXform
    {
        #region fields
        usdi.Mesh m_mesh;
        usdi.MeshData m_meshData;
        usdi.MeshSummary m_meshSummary;

        Mesh m_umesh;
        Vector3[] m_positions;
        Vector3[] m_velocities;
        Vector3[] m_normals;
        Vector2[] m_uvs;
        int[] m_indices;
        bool m_umeshIsEmpty;
        int m_prevVertexCount;
        int m_prevIndexCount;
        int m_frame;

        // for Unity 5.5 or later
        bool m_directVBUpdate;
        usdi.MapContext m_ctxVB;
        usdi.MapContext m_ctxIB;
        #endregion


        #region impl
        Mesh usdiAddMeshComponents()
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
            m_umesh = usdiAddMeshComponents();
            m_umeshIsEmpty = m_umesh.vertexCount == 0;
        }

        public override void usdiOnUnload()
        {
            base.usdiOnUnload();
            m_mesh = default(usdi.Mesh);
        }


        void usdiAllocateMeshData(double t)
        {
            usdi.MeshData md = default(usdi.MeshData);
            usdi.usdiMeshReadSample(m_mesh, ref md, t);

            // skip if already allocated
            if (m_prevVertexCount == md.num_points &&
                m_prevIndexCount == md.num_indices_triangulated)
            {
                return;
            }

            m_meshData = md;
            {
                m_positions = new Vector3[m_meshData.num_points];
                m_meshData.points = usdi.GetArrayPtr(m_positions);
            }
            if (m_meshSummary.has_normals)
            {
                m_normals = new Vector3[m_meshData.num_points];
                m_meshData.normals = usdi.GetArrayPtr(m_normals);
            }
            if (m_meshSummary.has_uvs)
            {
                m_uvs = new Vector2[m_meshData.num_points];
                m_meshData.uvs = usdi.GetArrayPtr(m_uvs);
            }
            {
                m_indices = new int[m_meshData.num_indices_triangulated];
                m_meshData.indices_triangulated = usdi.GetArrayPtr(m_indices);
            }
        }

        void usdiFreeMeshData()
        {
            m_positions = null;
            m_normals = null;
            m_uvs = null;
            m_indices = null;
            m_meshData.points = IntPtr.Zero;
            m_meshData.normals = IntPtr.Zero;
            m_meshData.uvs = IntPtr.Zero;
            m_meshData.indices_triangulated = IntPtr.Zero;
        }

        void usdiReadMeshData(double t)
        {
            usdi.usdiMeshReadSample(m_mesh, ref m_meshData, t);

            // need to improve this..
            m_directVBUpdate = m_stream.usdDirectVBUpdate &&
                (m_prevVertexCount == m_meshData.num_points && m_prevIndexCount == m_meshData.num_indices_triangulated) &&
                (m_ctxVB.resource != IntPtr.Zero);

            if (m_directVBUpdate)
            {
                bool update_topology = m_meshSummary.topology_variance == usdi.TopologyVariance.Heterogenous;
                if(!update_topology)
                {
                    m_meshData.indices_triangulated = IntPtr.Zero;
                }
                usdi.usdiExtQueueVertexBufferUpdateTask(ref m_meshData, ref m_ctxVB, ref m_ctxIB);
            }
        }

        void usdiUpdateMeshData(double t, bool topology, bool close)
        {
            if (!m_directVBUpdate)
            {
                m_umesh.vertices = m_positions;
                if (m_meshSummary.has_normals)
                {
                    m_umesh.normals = m_normals;
                }
                if (m_meshSummary.has_uvs)
                {
                    m_umesh.uv = m_uvs;
                }

                if (topology)
                {
                    m_umesh.SetIndices(m_indices, MeshTopology.Triangles, 0);

                    if (!m_meshSummary.has_normals)
                    {
                        m_umesh.RecalculateNormals();
                    }
                }

                m_umesh.UploadMeshData(close);
#if UNITY_5_5_OR_NEWER
                if(m_stream.usdDirectVBUpdate)
                {
                    m_ctxVB.resource = m_umesh.GetNativeVertexBufferPtr(0);
                    m_ctxIB.resource = m_umesh.GetNativeIndexBufferPtr();
                }
#endif
            }
            m_umeshIsEmpty = m_umesh.vertexCount == 0;
            m_prevVertexCount = m_meshData.num_points;
            m_prevIndexCount = m_meshData.num_indices_triangulated;
        }

        public override void usdiAsyncUpdate(double time)
        {
            base.usdiAsyncUpdate(time);

            switch (m_meshSummary.topology_variance)
            {
                case usdi.TopologyVariance.Constant:
                    if (m_frame == 0 && m_umeshIsEmpty)
                    {
                        usdiAllocateMeshData(time);
                        usdiReadMeshData(time);
                    }
                    break;

                case usdi.TopologyVariance.Homogenous:
                    if (m_frame == 0)
                    {
                        usdiAllocateMeshData(time);
                    }
                    usdiReadMeshData(time);
                    break;

                case usdi.TopologyVariance.Heterogenous:
                    usdiAllocateMeshData(time);
                    usdiReadMeshData(time);
                    break;
            }
        }

        public override void usdiUpdate(double time)
        {
            base.usdiUpdate(time);

            if(!m_mesh) { return; }

            switch (m_meshSummary.topology_variance) {
                case usdi.TopologyVariance.Constant:
                    if(m_frame == 0 && m_umeshIsEmpty)
                    {
                        usdiUpdateMeshData(time, true, true);
                        usdiFreeMeshData();
                    }
                    break;

                case usdi.TopologyVariance.Homogenous:
                    if (m_frame == 0)
                    {
                        usdiUpdateMeshData(time, true, false);
                    }
                    else
                    {
                        usdiUpdateMeshData(time, false, false);
                    }
                    break;

                case usdi.TopologyVariance.Heterogenous:
                    usdiUpdateMeshData(time, true, false);
                    break;
            }

            ++m_frame;
        }
    }
    #endregion

}
