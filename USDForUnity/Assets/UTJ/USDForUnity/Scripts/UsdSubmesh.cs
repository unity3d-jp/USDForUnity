using System;
using System.Reflection;
using System.IO;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{

    [Serializable]
    public class UsdSubmesh
    {
        #region fields
        int m_nth;
        Mesh m_umesh;

        Transform m_trans; // null in master nodes
        Renderer m_renderer; // null in master nodes

        usdi.Schema m_schema;
        bool m_setupRequierd = true;
        Vector3[] m_points;
        Vector3[] m_normals;
        Color[] m_colors;
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
        public Transform transform { get { return m_trans; } }
        public Renderer renderer { get { return m_renderer; } }
        public Mesh mesh { get { return m_umesh; } }
        public Matrix4x4[] bindposes { get { return m_bindposes; } }
        public Transform[] bones { get { return m_bones; } }
        public Transform rootBone { get { return m_rootBone; } }
        #endregion


        #region impl
        void usdiSetupBones(UsdMesh parent, ref usdi.MeshData meshData)
        {
            {
                var tmp = usdi.MeshData.default_value;
                m_bindposes = new Matrix4x4[parent.meshData.num_bones];
                tmp.bindposes = usdi.GetArrayPtr(m_bindposes);
                usdi.usdiMeshReadSample(parent.nativeMeshPtr, ref tmp, usdi.defaultTime, true);
            }

            var renderer = m_renderer as SkinnedMeshRenderer;
            if (renderer == null) { return; }

            if (m_nth > 0)
            {
                m_bindposes = parent.submeshes[0].bindposes;
                m_rootBone = parent.submeshes[0].rootBone;
                m_bones = parent.submeshes[0].bones;
            }
            else
            {
                var rootBoneName = usdi.S(meshData.root_bone);
                var boneNames = usdi.SA(meshData.bones);

                if (parent.isInstance)
                {
                    // remap bone names

                    var root = parent.nativeSchemaPtr;
                    for (;;) {
                        root = usdi.usdiPrimGetParent(root);
                        if(!usdi.usdiPrimGetMaster(root))
                        {
                            break;
                        }
                    }

                    var r = usdi.usdiPrimFindChild(root, Path.GetFileName(rootBoneName), true);
                    if (r)
                    {
                        rootBoneName = usdi.usdiPrimGetPathS(r);
                    }

                    for (int i = 0; i < boneNames.Length; ++i)
                    {
                        var c = usdi.usdiPrimFindChild(root, Path.GetFileName(boneNames[i]), true);
                        if (c)
                        {
                            boneNames[i] = usdi.usdiPrimGetPathS(c);
                        }
                    }
                }

                m_bones = new Transform[boneNames.Length];
                for (int i = 0; i < boneNames.Length; ++i)
                {
                    var schema = parent.stream.usdiFindSchema(boneNames[i]);
                    if (schema == null)
                    {
                        Debug.LogError("bone not found: " + boneNames[i]);
                        continue;
                    }
                    if (schema.gameObject == null)
                    {
                        Debug.LogError("bone don't have GameObject: " + boneNames[i]);
                        continue;
                    }
                    m_bones[i] = schema.gameObject.GetComponent<Transform>();
                }

                if (meshData.root_bone != IntPtr.Zero)
                {
                    var rootBone = parent.stream.usdiFindSchema(rootBoneName);
                    m_rootBone = rootBone.gameObject.GetComponent<Transform>();
                }
                else
                {
                    m_rootBone = m_bones[0]; // maybe incorrect
                }
            }

            renderer.bones = m_bones;
            renderer.rootBone = m_rootBone;
        }

        Mesh usdiShareOrCreateMesh(UsdMesh parent)
        {
            var master = parent.master as UsdMesh;
            if (master != null)
            {
                return master.submeshes[m_nth].mesh;
            }
            else
            {
                return new Mesh() {name = "<dyn>"};
            }
        }

        public void usdiSetupMesh(UsdMesh parent)
        {
            if (!m_setupRequierd) { return; }
            m_setupRequierd = false;

            if (m_umesh == null)
            {
                m_umesh = new Mesh();
                m_umesh.MarkDynamic();
            }

            var meshData = parent.meshData;
            usdiSetupBones(parent, ref meshData);
        }

        public void usdiSetupComponents(UsdMesh parent)
        {
            if (!m_setupRequierd) { return; }
            m_setupRequierd = false;

            GameObject go;
            if(m_nth == 0)
            {
                go = parent.gameObject;
            }
            else
            {
                string name = "Submesh[" + m_nth + "]";
                var ptrans = parent.GetComponent<Transform>();
                var child = ptrans.FindChild(name);
                if (child != null)
                {
                    go = child.gameObject;
                }
                else
                {
                    go = new GameObject(name);
                    go.GetComponent<Transform>().SetParent(parent.GetComponent<Transform>(), false);
                }
            }

            m_trans = go.GetComponent<Transform>();

            var meshSummary = parent.meshSummary;
            var meshData = parent.meshData;
            bool assignDefaultMaterial = false;

            if (meshSummary.num_bones > 0)
            {
                // setup SkinnedMeshRenderer

                var renderer = usdi.GetOrAddComponent<SkinnedMeshRenderer>(go, ref assignDefaultMaterial);
                m_renderer = renderer;
                usdiSetupBones(parent, ref meshData);

                if (renderer.sharedMesh != null && parent.master == null && renderer.sharedMesh.name.IndexOf("<dyn>") == 0)
                {
                    m_umesh = renderer.sharedMesh;
                }
                else
                {
                    m_umesh = usdiShareOrCreateMesh(parent);
                    renderer.sharedMesh = m_umesh;
                }
                m_umesh.MarkDynamic();
            }
            else
            {
                // setup MeshFilter and MeshRenderer

                var meshFilter = usdi.GetOrAddComponent<MeshFilter>(go);
                if (meshFilter.sharedMesh != null && parent.master == null && meshFilter.sharedMesh.name.IndexOf("<dyn>") == 0)
                {
                    m_umesh = meshFilter.sharedMesh;
                }
                else
                {
                    m_umesh = usdiShareOrCreateMesh(parent);
                    meshFilter.sharedMesh = m_umesh;
                }
                m_umesh.MarkDynamic();

                m_renderer = usdi.GetOrAddComponent<MeshRenderer>(go, ref assignDefaultMaterial);
            }

#if UNITY_EDITOR
            if (assignDefaultMaterial)
            {
                Material[] materials = null;
                if (m_nth != 0)
                {
                    var s = parent.submeshes[0];
                    if (s.renderer != null)
                    {
                        materials = s.renderer.sharedMaterials;
                    }
                }
                if(materials == null)
                {
                    materials = new Material[] { UnityEngine.Object.Instantiate(GetDefaultMaterial()) };
                    materials[0].name = "Material_0";
                }
                m_renderer.sharedMaterials = materials;
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
            if(summary.has_colors)
            {
                m_colors = new Color[meshData.num_points];
                meshData.colors = usdi.GetArrayPtr(m_colors);
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
                meshData.weights = usdi.GetArrayPtr(m_weights);
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
            if (summary.has_colors)
            {
                m_colors = new Color[data.num_points];
                data.colors = usdi.GetArrayPtr(m_colors);
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
            usdi.MeshAssignBounds(m_umesh, ref data.center, ref data.extents);
        }
        public void usdiUpdateBounds(ref usdi.SubmeshData data)
        {
            usdi.MeshAssignBounds(m_umesh, ref data.center, ref data.extents);
        }

        public void usdiSetActive(bool v)
        {
            if(m_trans != null)
            {
                m_trans.gameObject.SetActive(v);
            }
        }

        public void usdiOnLoad(UsdMesh parent, int nth)
        {
            m_schema = parent.nativeSchemaPtr;
            m_nth = nth;
        }

        public void usdiOnUnload()
        {
            usdiFreeMeshData();
        }


        void usdiUpdateSkinningData()
        {
            m_umesh.boneWeights = m_weights;
            m_umesh.bindposes = m_bindposes;
        }

        public void usdiUploadMeshData(bool directVBUpdate, bool topology, bool skinning, bool getVB)
        {
            if (directVBUpdate && m_VB != IntPtr.Zero)
            {
                if (m_weights != null && skinning)
                {
                    usdiUpdateSkinningData();
                    m_umesh.UploadMeshData(false);
                }
            }
            else
            {
                if (topology)
                {
                    m_umesh.Clear();
                }

                m_umesh.vertices = m_points;
                if (m_normals != null) { m_umesh.normals = m_normals; }
                if (m_colors != null) { m_umesh.colors = m_colors; }
                if (m_uvs != null) { m_umesh.uv = m_uvs; }
                if (m_tangents != null) { m_umesh.tangents = m_tangents; }

                if (topology)
                {
                    m_umesh.SetIndices(m_indices, MeshTopology.Triangles, 0);
                }

                if (m_weights != null && skinning)
                {
                    usdiUpdateSkinningData();
                }

                m_umesh.UploadMeshData(false);

#if UNITY_5_5_OR_NEWER
                if (getVB && usdi.usdiIsVtxCmdAvailable())
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
