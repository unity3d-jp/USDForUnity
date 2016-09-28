using System;
using System.Reflection;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{

    public class usdiSplitMesh : MonoBehaviour
    {
        #region fields
        usdiMesh m_parent;
        int m_nth;

        Mesh m_umesh;
        Vector3[] m_positions;
        Vector3[] m_normals;
        Vector2[] m_uvs;
        int[] m_indices;

        // for Unity 5.5 or later
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

        public void usdiOnLoad(usdiMesh parent, int nth)
        {
            m_parent = parent;
            m_nth = nth;
            m_umesh = usdiAddMeshComponents();
        }

        public void usdiOnUnload()
        {
        }


        public void usdiAllocateMeshData(ref usdi.SplitedMeshData data)
        {
            // skip if already allocated
            if (m_indices == null || m_indices.Length != data.num_points)
            {
                var sumamry = m_parent.meshSummary;
                m_indices = new int[data.num_points];
                m_positions = new Vector3[data.num_points];
                if (sumamry.has_normals) { m_normals = new Vector3[data.num_points]; }
                if (sumamry.has_uvs) { m_uvs = new Vector2[data.num_points]; }
            }

            data.points = usdi.GetArrayPtr(m_positions);
            data.indices = usdi.GetArrayPtr(m_indices);
            if (m_normals != null) { data.normals = usdi.GetArrayPtr(m_normals); }
            if (m_uvs != null) { data.uvs = usdi.GetArrayPtr(m_uvs); }
        }

        public void usdiFreeMeshData(ref usdi.SplitedMeshData data)
        {
            m_positions = null;
            m_normals = null;
            m_uvs = null;
            m_indices = null;

            data.points = IntPtr.Zero;
            data.normals = IntPtr.Zero;
            data.uvs = IntPtr.Zero;
            data.indices = IntPtr.Zero;
        }

        public void usdiUpdateMeshData(bool topology, bool close)
        {
            if (!m_parent.directVBUpdate)
            {
                m_umesh.vertices = m_positions;
                if (m_normals != null) { m_umesh.normals = m_normals; }
                if (m_uvs != null) { m_umesh.uv = m_uvs; }

                if (topology)
                {
                    m_umesh.SetIndices(m_indices, MeshTopology.Triangles, 0);
                    if (m_normals == null) { m_umesh.RecalculateNormals(); }
                }

                m_umesh.UploadMeshData(close);
#if UNITY_5_5_OR_NEWER
                if(m_parent.stream.directVBUpdate)
                {
                    m_ctxVB.resource = m_umesh.GetNativeVertexBufferPtr(0);
                    //m_ctxIB.resource = m_umesh.GetNativeIndexBufferPtr();
                }
#endif
            }
        }
    }
    #endregion

}
