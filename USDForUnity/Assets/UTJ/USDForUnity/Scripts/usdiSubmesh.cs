using System;
using System.Reflection;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{

    [Serializable]
    public class usdiSubmesh
    {
        #region fields
        [SerializeField] usdiStream m_stream;
        [SerializeField] int m_nth;
        [SerializeField] Mesh m_umesh;

        [SerializeField] Transform m_trans; // null in master nodes
        [SerializeField] Renderer m_renderer; // null in master nodes

        usdi.Schema m_schema;
        bool m_setupRequierd = true;
        Vector3[] m_points;
        Vector3[] m_normals;
        Vector2[] m_uvs;
        Vector4[] m_tangents;
        BoneWeight[] m_weights;
        Matrix4x4[] m_bindposes;
        Transform[] m_bones;
        Transform m_rootBone;
        int[] m_indices;
        int m_count;

        // for Unity 5.5 or later
        IntPtr m_VB, m_IB;
        usdi.VertexUpdateCommand m_vuCmd;
        #endregion


        #region properties
        public Mesh mesh { get { return m_umesh; } }
        #endregion


        #region impl
        public void usdiSetupMesh()
        {
            if (!m_setupRequierd) { return; }
            m_setupRequierd = false;

            if (m_umesh == null)
            {
                m_umesh = new Mesh();
                m_umesh.MarkDynamic();
            }
        }

        public void usdiSetupComponents(usdiMesh parent_mesh)
        {
            if (!m_setupRequierd) { return; }
            m_setupRequierd = false;

            GameObject go;
            if(m_nth == 0)
            {
                go = parent_mesh.gameObject;
            }
            else
            {
                string name = "Submesh[" + m_nth + "]";
                var parent = parent_mesh.GetComponent<Transform>();
                var child = parent.FindChild(name);
                if (child != null)
                {
                    go = child.gameObject;
                }
                else
                {
                    go = new GameObject(name);
                    go.GetComponent<Transform>().SetParent(parent_mesh.GetComponent<Transform>(), false);
                }
            }

            m_trans = go.GetComponent<Transform>();

            var meshSummary = parent_mesh.meshSummary;
            var meshData = parent_mesh.meshData;
            bool assignDefaultMaterial = false;

            if (meshSummary.num_bones > 0)
            {
                // setup SkinnedMeshRenderer

                var renderer = go.GetComponent<SkinnedMeshRenderer>();
                if (renderer == null)
                {
                    renderer = go.AddComponent<SkinnedMeshRenderer>();
                    assignDefaultMaterial = true;

                }
                {
                    var boneNames = usdi.usdiMeshGetBoneNames(parent_mesh.nativeMeshPtr, ref meshData);
                    m_bones = new Transform[boneNames.Length];
                    for (int i = 0; i < boneNames.Length; ++i)
                    {
                        var schema = m_stream.usdiFindSchema(boneNames[i]);
                        if (schema == null)
                        {
                            Debug.LogError("bone not found: " + boneNames[i]);
                            continue;
                        }
                        if (schema.gameObject == null)
                        {
                            // todo: this will happen on instanced object. need to do something
                            Debug.LogError("bone don't have GameObject: " + boneNames[i]);
                            continue;
                        }
                        m_bones[i] = schema.gameObject.GetComponent<Transform>();
                    }

                    if(meshData.root_bone != IntPtr.Zero)
                    {
                        var rootBone = m_stream.usdiFindSchema(usdi.S(meshData.root_bone));
                        m_rootBone = rootBone.gameObject.GetComponent<Transform>();
                    }
                    else
                    {
                        m_rootBone = m_bones[0]; // maybe incorrect
                    }
                }
                m_renderer = renderer;

                m_umesh = renderer.sharedMesh;
                if (m_umesh == null)
                {
                    m_umesh = new Mesh();
                    renderer.sharedMesh = m_umesh;
                }
                m_umesh.MarkDynamic();
            }
            else
            {
                // setup MeshFilter and MeshRenderer

                var meshFilter = go.GetComponent<MeshFilter>();
                if (meshFilter == null || meshFilter.sharedMesh == null)
                {
                    m_umesh = new Mesh();
                    if (meshFilter == null)
                    {
                        meshFilter = go.AddComponent<MeshFilter>();
                    }
                    meshFilter.sharedMesh = m_umesh;
                }
                else
                {
                    m_umesh = meshFilter.sharedMesh;
                }
                m_umesh.MarkDynamic();

                m_renderer = go.GetComponent<MeshRenderer>();
                if (m_renderer == null)
                {
                    m_renderer = go.AddComponent<MeshRenderer>();
                    assignDefaultMaterial = true;
                }
            }

#if UNITY_EDITOR
            if (assignDefaultMaterial)
            {
                Material material = UnityEngine.Object.Instantiate(GetDefaultMaterial());
                material.name = "Material_0";
                m_renderer.sharedMaterial = material;
            }
#endif
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


        public void usdiAllocateMeshData(ref usdi.MeshSummary summary,  ref usdi.MeshData meshData)
        {
            {
                m_points = new Vector3[meshData.num_points];
                meshData.points = usdi.GetArrayPtr(m_points);
            }
            {
                m_normals = new Vector3[meshData.num_points];
                meshData.normals = usdi.GetArrayPtr(m_normals);
            }
            if (summary.has_tangents)
            {
                m_tangents = new Vector4[meshData.num_points];
                meshData.tangents = usdi.GetArrayPtr(m_tangents);
            }
            if (summary.has_uvs)
            {
                m_uvs = new Vector2[meshData.num_points];
                meshData.uvs = usdi.GetArrayPtr(m_uvs);
                if (summary.has_tangents)
                {
                    m_tangents = new Vector4[meshData.num_points];
                    meshData.tangents = usdi.GetArrayPtr(m_tangents);
                }
            }
            if (summary.num_bones > 0)
            {
                m_weights = new BoneWeight[meshData.num_points];
                m_bindposes = new Matrix4x4[summary.num_bones];
                meshData.weights = usdi.GetArrayPtr(m_weights);
                meshData.bindposes = usdi.GetArrayPtr(m_bindposes);
            }
            {
                m_indices = new int[meshData.num_indices_triangulated];
                meshData.indices_triangulated = usdi.GetArrayPtr(m_indices);
            }
        }
        public void usdiAllocateMeshData(ref usdi.MeshSummary summary, ref usdi.SubmeshData[] submeshData)
        {
            var data = submeshData[m_nth];
            {
                m_points = new Vector3[data.num_points];
                data.points = usdi.GetArrayPtr(m_points);
            }
            {
                m_normals = new Vector3[data.num_points];
                data.normals = usdi.GetArrayPtr(m_normals);
            }
            if (summary.has_tangents)
            {
                m_tangents = new Vector4[data.num_points];
                data.tangents = usdi.GetArrayPtr(m_tangents);
            }
            if (summary.has_uvs)
            {
                m_uvs = new Vector2[data.num_points];
                data.uvs = usdi.GetArrayPtr(m_uvs);
                if (summary.has_tangents)
                {
                    m_tangents = new Vector4[data.num_points];
                    data.tangents = usdi.GetArrayPtr(m_tangents);
                }
            }
            if (summary.num_bones > 0)
            {
                m_weights = new BoneWeight[data.num_points];
                data.weights = usdi.GetArrayPtr(m_weights);
            }
            {
                m_indices = new int[data.num_points];
                data.indices = usdi.GetArrayPtr(m_indices);
            }
            submeshData[m_nth] = data;
        }

        public void usdiFreeMeshData()
        {
            m_points = null;
            m_normals = null;
            m_uvs = null;
            m_indices = null;
        }

        public void usdiKickVBUpdateTask(ref usdi.MeshData data, bool updateIndices)
        {
            bool active = m_renderer.isVisible;
            if(active)
            {
                if (m_vuCmd == null)
                {
                    m_vuCmd = new usdi.VertexUpdateCommand(usdi.usdiPrimGetNameS(m_schema));
                }
                m_vuCmd.Update(ref data, m_VB, updateIndices ? m_IB : IntPtr.Zero);
            }
        }
        public void usdiKickVBUpdateTask(ref usdi.SubmeshData data, bool updateIndices)
        {
            bool active = m_renderer.isVisible;
            if (active)
            {
                if (m_vuCmd == null)
                {
                    m_vuCmd = new usdi.VertexUpdateCommand(usdi.usdiPrimGetNameS(m_schema));
                }
                m_vuCmd.Update(ref data, m_VB, updateIndices ? m_IB : IntPtr.Zero);
            }
        }

        public void usdiUpdateBounds(ref usdi.MeshData data)
        {
            usdi.usdiUniMeshAssignBounds(m_umesh, ref data.center, ref data.extents);
        }
        public void usdiUpdateBounds(ref usdi.SubmeshData data)
        {
            usdi.usdiUniMeshAssignBounds(m_umesh, ref data.center, ref data.extents);
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
            m_stream = parent.stream;
            m_schema = parent.nativeSchemaPtr;
            m_nth = nth;
        }

        public void usdiOnUnload()
        {
            usdiFreeMeshData();
        }


        public void usdiUploadMeshData(bool directVBUpdate, bool topology, bool close)
        {
            if (directVBUpdate && m_VB != IntPtr.Zero)
            {
                // nothing to do here
            }
            else
            {
                m_umesh.vertices = m_points;
                if (m_normals != null) { m_umesh.normals = m_normals; }
                if (m_uvs != null) { m_umesh.uv = m_uvs; }
                if (m_tangents != null) { m_umesh.tangents = m_tangents; }

                if (topology)
                {
                    m_umesh.SetIndices(m_indices, MeshTopology.Triangles, 0);
                    if (m_normals == null) { m_umesh.RecalculateNormals(); }
                }

                if(m_count == 0 && m_weights != null)
                {
                    if(m_stream.importSettings.swapHandedness)
                    {
                        Debug.LogWarning("Swap Handedness import option is enabled. This may cause broken skinning animation.");
                    }

                    m_umesh.boneWeights = m_weights;
                    m_umesh.bindposes = m_bindposes;
                    var renderer = m_renderer as SkinnedMeshRenderer;
                    if(renderer != null)
                    {
                        renderer.bones = m_bones;
                        renderer.rootBone = m_rootBone;
                    }
                }

                //m_umesh.UploadMeshData(close);
                m_umesh.UploadMeshData(false);

#if UNITY_5_5_OR_NEWER
                if (m_stream.directVBUpdate && usdi.usdiVtxCmdIsAvailable())
                {
                    m_VB = m_umesh.GetNativeVertexBufferPtr(0);
                    m_IB = m_umesh.GetNativeIndexBufferPtr();
                }
#endif
            }

            ++m_count;
        }
    }
    #endregion

}
