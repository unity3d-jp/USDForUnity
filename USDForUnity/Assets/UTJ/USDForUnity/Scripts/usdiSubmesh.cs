using System;
using System.Reflection;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{

    public class usdiSubmesh
    {
        #region fields
        usdiMesh m_parent;
        int m_nth;

        Transform m_trans;
        MeshFilter m_meshFilter;
        MeshRenderer m_renderer;
        Mesh m_umesh;

        Vector3[] m_points;
        Vector3[] m_normals;
        Vector2[] m_uvs;
        int[] m_indices;

        // for Unity 5.5 or later
        IntPtr m_VB, m_IB;
        usdi.VertexUpdateCommand m_vuCmd;
        #endregion


        #region impl
        public void usdiSetupMeshComponents()
        {
            if (m_umesh != null) { return; }

            GameObject go;
            if(m_nth == 0)
            {
                go = m_parent.gameObject;
            }
            else
            {
                string name = "Submesh[" + m_nth + "]";
                var parent = m_parent.GetComponent<Transform>();
                var child = parent.FindChild(name);
                if (child != null)
                {
                    go = child.gameObject;
                }
                else
                {
                    go = new GameObject(name);
                    go.GetComponent<Transform>().SetParent(m_parent.GetComponent<Transform>(), false);
                }
            }

            m_trans = go.GetComponent<Transform>();
            m_meshFilter = go.GetComponent<MeshFilter>();
            if (m_meshFilter == null || m_meshFilter.sharedMesh == null)
            {
                m_umesh = new Mesh();
                if (m_meshFilter == null)
                {
                    m_meshFilter = go.AddComponent<MeshFilter>();
                }
                m_meshFilter.sharedMesh = m_umesh;
            }
            else
            {
                m_umesh = m_meshFilter.sharedMesh;
            }
            m_umesh.MarkDynamic();

            m_renderer = go.GetComponent<MeshRenderer>();
            if (m_renderer == null)
            {
                m_renderer = go.AddComponent<MeshRenderer>();
#if UNITY_EDITOR
                Material material = UnityEngine.Object.Instantiate(GetDefaultMaterial());
                material.name = "Material_0";
                m_renderer.sharedMaterial = material;
#endif
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


        public void usdiAllocateMeshData()
        {
            var summary = m_parent.meshSummary;

            var meshData = m_parent.meshData;
            if (meshData.num_submeshes == 0)
            {
                var data = meshData;
                {
                    m_points = new Vector3[data.num_points];
                    data.points = usdi.GetArrayPtr(m_points);
                }
                if (summary.has_normals)
                {
                    m_normals = new Vector3[data.num_points];
                    data.normals = usdi.GetArrayPtr(m_normals);
                }
                if (summary.has_uvs)
                {
                    m_uvs = new Vector2[data.num_points];
                    data.uvs = usdi.GetArrayPtr(m_uvs);
                }
                {
                    m_indices = new int[data.num_indices_triangulated];
                    data.indices_triangulated = usdi.GetArrayPtr(m_indices);
                }
                m_parent.meshData = data;
            }
            else
            {
                var data = m_parent.submeshData[m_nth];
                {
                    m_points = new Vector3[data.num_points];
                    data.points = usdi.GetArrayPtr(m_points);
                }
                if (summary.has_normals)
                {
                    m_normals = new Vector3[data.num_points];
                    data.normals = usdi.GetArrayPtr(m_normals);
                }
                if (summary.has_uvs)
                {
                    m_uvs = new Vector2[data.num_points];
                    data.uvs = usdi.GetArrayPtr(m_uvs);
                }
                {
                    m_indices = new int[data.num_points];
                    data.indices = usdi.GetArrayPtr(m_indices);
                }
                m_parent.submeshData[m_nth] = data;
            }
        }

        public void usdiFreeMeshData()
        {
            m_points = null;
            m_normals = null;
            m_uvs = null;
            m_indices = null;
        }

        public void usdiKickVBUpdateTask()
        {
            bool active = m_trans.gameObject.activeInHierarchy && m_renderer.isVisible;

            if(active)
            {
                if (m_vuCmd == null)
                {
                    m_vuCmd = new usdi.VertexUpdateCommand(usdi.usdiGetNameS(m_parent.usdiObject));
                }

                var meshData = m_parent.meshData;
                if (meshData.num_submeshes == 0)
                {
                    m_vuCmd.Update(ref meshData, m_VB, IntPtr.Zero);
                }
                else
                {
                    var submeshData = m_parent.submeshData;
                    m_vuCmd.Update(ref submeshData[m_nth], m_VB, IntPtr.Zero);
                }
            }
        }

        public void usdiUpdateBounds()
        {
            var meshData = m_parent.meshData;
            if (meshData.num_submeshes == 0)
            {
                usdi.usdiUniMeshAssignBounds(m_umesh, ref meshData.center, ref meshData.extents);
            }
            else
            {
                var submeshData = m_parent.submeshData;
                usdi.usdiUniMeshAssignBounds(m_umesh, ref submeshData[m_nth].center, ref submeshData[m_nth].extents);
            }
        }

        public void usdiSetActive(bool v)
        {
            if(m_trans != null)
            {
                m_trans.gameObject.SetActive(v);
            }
        }

        public void usdiOnLoad(usdiMesh parent, int nth)
        {
            m_parent = parent;
            m_nth = nth;
        }

        public void usdiOnUnload()
        {
        }


        public void usdiUploadMeshData(bool topology, bool close)
        {
            bool directVBUpdate = m_parent.directVBUpdate && m_VB != null;
            if (directVBUpdate)
            {
                // nothing to do here
            }
            else
            {
                m_umesh.vertices = m_points;
                if (m_normals != null) { m_umesh.normals = m_normals; }
                if (m_uvs != null) { m_umesh.uv = m_uvs; }

                if (topology)
                {
                    m_umesh.SetIndices(m_indices, MeshTopology.Triangles, 0);
                    if (m_normals == null) { m_umesh.RecalculateNormals(); }
                }

                //m_umesh.UploadMeshData(close);
                m_umesh.UploadMeshData(false);
#if UNITY_5_5_OR_NEWER
                if (m_parent.stream.directVBUpdate)
                {
                    m_VB = m_umesh.GetNativeVertexBufferPtr(0);
                    m_IB = m_umesh.GetNativeIndexBufferPtr();
                }
#endif
            }
        }
    }
    #endregion

}
