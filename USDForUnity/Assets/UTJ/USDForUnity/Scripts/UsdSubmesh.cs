using System;
using System.Reflection;
using System.IO;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ.USD
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
        PinnedList<int> m_indices = new PinnedList<int>();
        PinnedList<Vector3> m_points = new PinnedList<Vector3>();
        PinnedList<Vector3> m_normals = new PinnedList<Vector3>();
        PinnedList<Color> m_colors = new PinnedList<Color>();
        PinnedList<Vector2> m_uvs = new PinnedList<Vector2>();
        PinnedList<Vector4> m_tangents = new PinnedList<Vector4>();
        PinnedList<BoneWeight> m_weights = new PinnedList<BoneWeight>();
        PinnedList<Matrix4x4> m_bindposes = new PinnedList<Matrix4x4>();
        Transform[] m_bones;
        Transform m_rootBone;
        int m_count;
        #endregion


        #region properties
        public Transform transform { get { return m_trans; } }
        public Renderer renderer { get { return m_renderer; } }
        public Mesh mesh { get { return m_umesh; } }
        public PinnedList<Matrix4x4> bindposes { get { return m_bindposes; } }
        public Transform[] bones { get { return m_bones; } }
        public Transform rootBone { get { return m_rootBone; } }
        #endregion


        #region impl
        void usdiSetupBones(UsdMesh parent, ref usdi.MeshData meshData)
        {
            {
                var tmp = usdi.MeshData.default_value;
                m_bindposes.ResizeDiscard(parent.meshData.num_bones);
                tmp.bindposes = m_bindposes;
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
                var child = ptrans.Find(name);
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
                    materials = new Material[]{
                        new Material(Shader.Find("Standard")),
                    };
                    materials[0].name = "Material";
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
                m_points.ResizeDiscard(meshData.num_points);
                meshData.points = m_points;
            }
            {
                m_normals.ResizeDiscard(meshData.num_points);
                meshData.normals = m_normals;
            }
            if(summary.has_colors)
            {
                m_colors.ResizeDiscard(meshData.num_points);
                meshData.colors = m_colors;
            }
            if (summary.has_tangents)
            {
                m_tangents.ResizeDiscard(meshData.num_points);
                meshData.tangents = m_tangents;
            }
            if (summary.has_uvs)
            {
                m_uvs.ResizeDiscard(meshData.num_points);
                meshData.uvs = m_uvs;
                if (summary.has_tangents)
                {
                    m_tangents.ResizeDiscard(meshData.num_points);
                    meshData.tangents = m_tangents;
                }
            }
            if (summary.num_bones > 0)
            {
                m_weights.ResizeDiscard(meshData.num_points);
                meshData.weights = m_weights;
            }
            {
                m_indices.ResizeDiscard(meshData.num_indices_triangulated);
                meshData.indices_triangulated = m_indices;
            }
        }
        public void usdiAllocateMeshData(ref usdi.MeshSummary summary, PinnedList<usdi.SubmeshData> submeshData)
        {
            var data = submeshData[m_nth];
            {
                m_points.ResizeDiscard(data.num_points);
                data.points = m_points;
            }
            {
                m_normals.ResizeDiscard(data.num_points);
                data.normals = m_normals;
            }
            if (summary.has_colors)
            {
                m_colors.ResizeDiscard(data.num_points);
                data.colors = m_colors;
            }
            if (summary.has_tangents)
            {
                m_tangents.ResizeDiscard(data.num_points);
                data.tangents = m_tangents;
            }
            if (summary.has_uvs)
            {
                m_uvs.ResizeDiscard(data.num_points);
                data.uvs = m_uvs;
                if (summary.has_tangents)
                {
                    m_tangents.ResizeDiscard(data.num_points);
                    data.tangents = m_tangents;
                }
            }
            if (summary.num_bones > 0)
            {
                m_weights.ResizeDiscard(data.num_points);
                data.weights = m_weights;
            }
            {
                m_indices.ResizeDiscard(data.num_points); // todo: data.num_indices
                data.indices = m_indices;
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
            m_umesh.boneWeights = m_weights.Array;
            m_umesh.bindposes = m_bindposes.Array;
        }

        public void usdiUploadMeshData(bool topology, bool skinning, bool getVB)
        {
            if (topology)
                m_umesh.Clear();

            m_umesh.SetVertices(m_points.List);
            if (m_normals.Count > 0) { m_umesh.SetNormals(m_normals.List); }
            if (m_colors.Count > 0) { m_umesh.SetColors(m_colors.List); }
            if (m_uvs.Count > 0) { m_umesh.SetUVs(0, m_uvs.List); }
            if (m_tangents.Count > 0) { m_umesh.SetTangents(m_tangents.List); }

            if (topology)
                m_umesh.SetTriangles(m_indices.List, 0);

            if (m_weights != null && skinning)
                usdiUpdateSkinningData();

            m_umesh.UploadMeshData(false);

            ++m_count;
        }
    }
    #endregion

}
