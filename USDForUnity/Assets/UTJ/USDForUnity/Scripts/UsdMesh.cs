using System;
using System.IO;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
using UnityEngine.Rendering;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ.USD
{
    public class UsdMesh : UsdXform
    {
        public class Submesh
        {
            public PinnedList<int> indices = new PinnedList<int>();
            public bool update = true;
        }

        public class Split
        {
            public PinnedList<int> indices = new PinnedList<int>();
            public PinnedList<Vector3> points = new PinnedList<Vector3>();
            public PinnedList<Vector3> velocities = new PinnedList<Vector3>();
            public PinnedList<Vector3> normals = new PinnedList<Vector3>();
            public PinnedList<Vector4> tangents = new PinnedList<Vector4>();
            public PinnedList<Vector2> uv0 = new PinnedList<Vector2>();
            public PinnedList<Vector2> uv1 = new PinnedList<Vector2>();
            public PinnedList<Color> colors = new PinnedList<Color>();

            public PinnedList<BoneWeight> weights = new PinnedList<BoneWeight>();
            public PinnedList<Matrix4x4> bindposes = new PinnedList<Matrix4x4>();
            public Transform[] bones;
            public Transform rootBone;

            public Mesh mesh;
            public GameObject host; // null if master nodes
            public bool clear = true;
            public bool active = true;

            public Vector3 center;
            public Vector3 size;
            int nth;

            #region impl
            Mesh ShareOrCreateMesh(UsdMesh parent)
            {
                var master = parent.master as UsdMesh;
                if (master != null)
                {
                    return master.submeshes[nth].mesh;
                }
                else
                {
                    return new Mesh() { name = "<dyn>" };
                }
            }

            public void SetupMesh(UsdMesh parent)
            {
                if (mesh == null)
                {
                    mesh = new Mesh();
#if UNITY_2017_3_OR_NEWER
                    mesh.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32;
#endif
                    mesh.MarkDynamic();
                }
            }

            public void SetupComponents(UsdMesh parent)
            {
                if (nth == 0)
                {
                    host = parent.gameObject;
                }
                else
                {
                    string name = "Submesh[" + nth + "]";
                    var ptrans = parent.GetComponent<Transform>();
                    var child = ptrans.Find(name);
                    if (child != null)
                    {
                        host = child.gameObject;
                    }
                    else
                    {
                        host = new GameObject(name);
                        host.GetComponent<Transform>().SetParent(parent.GetComponent<Transform>(), false);
                    }
                }

                var transform = host.GetComponent<Transform>();

                var meshSummary = parent.meshSummary;
                if (meshSummary.boneCount > 0)
                {
                    // setup SkinnedMeshRenderer

                    var renderer = usdi.GetOrAddComponent<SkinnedMeshRenderer>(parent.gameObject);

                    if (renderer.sharedMesh != null && parent.master == null && renderer.sharedMesh.name.IndexOf("<dyn>") == 0)
                    {
                        mesh = renderer.sharedMesh;
                    }
                    else
                    {
                        mesh = ShareOrCreateMesh(parent);
                        renderer.sharedMesh = mesh;
                    }
                    mesh.MarkDynamic();
                }
                else
                {
                    // setup MeshFilter and MeshRenderer

                    var meshFilter = usdi.GetOrAddComponent<MeshFilter>(go);
                    if (meshFilter.sharedMesh != null && parent.master == null && meshFilter.sharedMesh.name.IndexOf("<dyn>") == 0)
                    {
                        mesh = meshFilter.sharedMesh;
                    }
                    else
                    {
                        mesh = ShareOrCreateMesh(parent);
                        meshFilter.sharedMesh = mesh;
                    }
                    mesh.MarkDynamic();

                    renderer = usdi.GetOrAddComponent<MeshRenderer>(host, ref assignDefaultMaterial);
                }
            }


            public void AllocateMeshData(ref usdi.MeshSummary summary, ref usdi.MeshData meshData)
            {
                {
                    points.ResizeDiscard(meshData.vertexCount);
                    meshData.points = points;
                }
                {
                    normals.ResizeDiscard(meshData.vertexCount);
                    meshData.normals = normals;
                }
                if (summary.hasColors)
                {
                    colors.ResizeDiscard(meshData.vertexCount);
                    meshData.colors = colors;
                }
                if (summary.hasTangents)
                {
                    tangents.ResizeDiscard(meshData.vertexCount);
                    meshData.tangents = tangents;
                }
                if (summary.hasUV0)
                {
                    uv0.ResizeDiscard(meshData.vertexCount);
                    meshData.uv0 = uv0;
                    if (summary.hasTangents)
                    {
                        tangents.ResizeDiscard(meshData.vertexCount);
                        meshData.tangents = tangents;
                    }
                }
                if (summary.boneCount > 0)
                {
                    weights.ResizeDiscard(meshData.vertexCount);
                    meshData.weights = weights;
                }
                {
                    indices.ResizeDiscard(meshData.indexCount);
                    meshData.indices = indices;
                }
            }


            public void usdiUploadMeshData(bool topology, bool skinning)
            {
                if (topology)
                    mesh.Clear();

                mesh.SetVertices(points.List);
                if (normals.Count > 0) { mesh.SetNormals(normals.List); }
                if (colors.Count > 0) { mesh.SetColors(colors.List); }
                if (uv0.Count > 0) { mesh.SetUVs(0, uv0.List); }
                if (tangents.Count > 0) { mesh.SetTangents(tangents.List); }

                if (topology)
                    mesh.SetTriangles(indices.List, 0);

                if (weights != null && skinning)
                {
                    mesh.boneWeights = weights.Array;
                    mesh.bindposes = bindposes.Array;
                }

                mesh.UploadMeshData(false);
            }
            #endregion
        }



        #region fields
        usdi.Mesh m_usdMesh;
        usdi.MeshSummary m_summary;
        usdi.MeshSampleSummary m_sampleSummary;
        PinnedList<usdi.MeshSplitSummary> m_splitSummaries = new PinnedList<usdi.MeshSplitSummary>();
        PinnedList<usdi.SubmeshSummary> m_submeshSummaries = new PinnedList<usdi.SubmeshSummary>();
        PinnedList<usdi.MeshData> m_splitData = new PinnedList<usdi.MeshData>();
        PinnedList<usdi.SubmeshData> m_submeshData = new PinnedList<usdi.SubmeshData>();

        List<Split> m_splits = new List<Split>();
        List<Submesh> m_submeshes = new List<Submesh>();
        bool m_freshSetup = false;
        #endregion


        #region properties
        public usdi.Mesh usdMesh
        {
            get { return m_usdMesh; }
        }
        public usdi.MeshSummary meshSummary
        {
            get { return m_summary; }
        }
        public PinnedList<usdi.MeshData> splitData
        {
            get { return m_splitData; }
        }
        public List<Split> submeshes
        {
            get { return m_splits; }
        }
        #endregion


        #region impl
        void UpdateSplits(int numSplits)
        {
            Split split = null;

            if (m_summary.topologyVariance == usdi.TopologyVariance.Heterogenous || numSplits > 1)
            {
                for (int i = 0; i < numSplits; ++i)
                {
                    if (i >= m_splits.Count)
                        m_splits.Add(new Split());
                    else
                        m_splits[i].active = true;
                }
            }
            else
            {
                if (m_splits.Count == 0)
                {
                    split = new Split
                    {
                        host = m_linkedGameObj,
                    };
                    m_splits.Add(split);
                }
                else
                {
                    m_splits[0].active = true;
                }
            }

            for (int i = numSplits; i < m_splits.Count; ++i)
                m_splits[i].active = false;
        }


        protected override UsdIComponent UsdSetupSchemaComponent()
        {
            return GetOrAddComponent<UsdMeshComponent>();
        }

        public override void UsdOnLoad()
        {
            base.UsdOnLoad();

            m_freshSetup = true;
            m_usdMesh = usdi.usdiAsMesh(m_schema);
            usdi.usdiMeshGetSummary(m_usdMesh, ref m_summary);

            if (isInstance)
            {
                // 
            }
            else
            {
                if (m_summary.boneCount > 0 && m_stream.importSettings.swapHandedness)
                {
                    Debug.LogWarning("Swap Handedness import option is enabled. This may cause broken skinning animation.");
                }
            }
        }

        public override void UsdOnUnload()
        {
            base.UsdOnUnload();

            int c = m_splits.Count;
            for (int i = 0; i < c; ++i) { m_splits[i].usdiOnUnload(); }

            m_splitData.Clear();
            m_summary = usdi.MeshSummary.defaultValue;
        }


        // async
        void usdiAllocateMeshData(double t)
        {
            usdi.MeshData md = usdi.MeshData.defaultValue;
            usdi.usdiMeshReadSample(m_usdMesh, ref md, t, true);

            m_meshData = md;
            if (m_meshData.submeshCount == 0)
            {
                m_splits[0].AllocateMeshData(ref m_summary, ref m_meshData);
            }
            else
            {
                m_splitData.ResizeDiscard(m_meshData.submeshCount);
                m_meshData.submeshes = m_splitData;
                usdi.usdiMeshReadSample(m_usdMesh, ref m_meshData, t, true);

                while (m_splits.Count < m_meshData.submeshCount)
                {
                    usdiAddSubmesh();
                }
                for (int i = 0; i < m_meshData.submeshCount; ++i)
                {
                    m_splits[i].AllocateMeshData(ref m_summary, m_splitData);
                }
            }
        }

        // async
        public override void UsdPrepareSample()
        {
            base.UsdPrepareSample();
            if (m_updateFlags.bits == 0) {
                return;
            }
            if(m_updateFlags.importConfigChanged || m_updateFlags.variantSetChanged)
            {
                usdi.usdiMeshGetSummary(m_usdMesh, ref m_summary);
            }

            if(isInstance)
            {
                usdi.usdiMeshReadSample(m_usdMesh, ref m_meshData, time, true);
                while (m_splits.Count < m_meshData.submeshCount)
                {
                    usdiAddSubmesh();
                }
            }
            else
            {
            }

            usdiAllocateMeshData(m_timeRead);

            if (m_updateVerticesRequired)
            {
                usdi.usdiMeshReadSample(m_usdMesh, ref m_meshData, m_timeRead, true);
            }
        }



        // sync
        public override void UsdSyncDataBegin()
        {
            base.UsdSyncDataBegin();


            usdi.usdiMeshGetSampleSummary(m_usdMesh, ref m_sampleSummary);
            int splitCount = m_sampleSummary.splitCount;
            int submeshCount = m_sampleSummary.submeshCount;

            m_splitSummaries.ResizeDiscard(splitCount);
            m_splitData.ResizeDiscard(splitCount);
            m_submeshSummaries.ResizeDiscard(submeshCount);
            m_submeshData.ResizeDiscard(submeshCount);

            sample.GetSplitSummaries(m_splitSummaries);
            sample.GetSubmeshSummaries(m_submeshSummaries);

            UpdateSplits(m_sampleSummary.splitCount);


            bool topologyChanged = m_sampleSummary.topologyChanged;

            // setup buffers
            var vertexData = default(usdi.MeshData);
            for (int spi = 0; spi < splitCount; ++spi)
            {
                var split = m_splits[spi];

                split.clear = topologyChanged;
                split.active = true;

                int vertexCount = m_splitSummaries[spi].vertexCount;

                if (!m_summary.constantPoints || topologyChanged)
                    split.points.ResizeDiscard(vertexCount);
                else
                    split.points.ResizeDiscard(0);
                vertexData.points = split.points;

                if (m_summary.hasVelocities && (!m_summary.constantVelocities || topologyChanged))
                    split.velocities.ResizeDiscard(vertexCount);
                else
                    split.velocities.ResizeDiscard(0);
                vertexData.velocities = split.velocities;

                if (m_summary.hasNormals && (!m_summary.constantNormals || topologyChanged))
                    split.normals.ResizeDiscard(vertexCount);
                else
                    split.normals.ResizeDiscard(0);
                vertexData.normals = split.normals;

                if (m_summary.hasTangents && (!m_summary.constantTangents || topologyChanged))
                    split.tangents.ResizeDiscard(vertexCount);
                else
                    split.tangents.ResizeDiscard(0);
                vertexData.tangents = split.tangents;

                if (m_summary.hasUV0 && (!m_summary.constantUV0 || topologyChanged))
                    split.uv0.ResizeDiscard(vertexCount);
                else
                    split.uv0.ResizeDiscard(0);
                vertexData.uv0 = split.uv0;

                if (m_summary.hasUV1 && (!m_summary.constantUV1 || topologyChanged))
                    split.uv1.ResizeDiscard(vertexCount);
                else
                    split.uv1.ResizeDiscard(0);
                vertexData.uv1 = split.uv1;

                if (m_summary.hasColors && (!m_summary.constantColors || topologyChanged))
                    split.colors.ResizeDiscard(vertexCount);
                else
                    split.colors.ResizeDiscard(0);
                vertexData.colors = split.colors;

                m_splitData[spi] = vertexData;
            }

            {
                if (m_submeshes.Count > submeshCount)
                    m_submeshes.RemoveRange(submeshCount, m_submeshes.Count - submeshCount);
                while (m_submeshes.Count < submeshCount)
                    m_submeshes.Add(new Submesh());

                var submeshData = default(usdi.SubmeshData);
                for (int smi = 0; smi < submeshCount; ++smi)
                {
                    var submesh = m_submeshes[smi];
                    m_submeshes[smi].update = true;
                    submesh.indices.ResizeDiscard(m_submeshSummaries[smi].indexCount);
                    submeshData.indices = submesh.indices;
                    m_submeshData[smi] = submeshData;
                }
            }

            // kick async copy
            sample.FillVertexBuffer(m_splitData, m_submeshData);
        }

        public override void UsdSyncDataEnd()
        {
#if UNITY_EDITOR
            for (int s = 0; s < m_splits.Count; ++s)
            {
                var split = m_splits[s];
                var mf = split.host.GetComponent<MeshFilter>();
                if (mf != null)
                    mf.sharedMesh = split.mesh;
            }
#endif

            if (!m_usdMesh.schema.isDataUpdated)
                return;

            // wait async copy complete
            var sample = m_usdMesh.sample;
            sample.Sync();

            bool useSubObjects = (m_summary.topologyVariance == usdi.TopologyVariance.Heterogenous || m_sampleSummary.splitCount > 1);

            for (int s = 0; s < m_splits.Count; ++s)
            {
                var split = m_splits[s];
                if (split.active)
                {
                    // Feshly created splits may not have their host set yet
                    if (split.host == null)
                    {
                        if (useSubObjects)
                        {
                            string name = abcTreeNode.linkedGameObj.name + "_split_" + s;

                            Transform trans = abcTreeNode.linkedGameObj.transform.Find(name);

                            if (trans == null)
                            {
                                GameObject go = new GameObject();
                                go.name = name;

                                trans = go.GetComponent<Transform>();
                                trans.parent = abcTreeNode.linkedGameObj.transform;
                                trans.localPosition = Vector3.zero;
                                trans.localEulerAngles = Vector3.zero;
                                trans.localScale = Vector3.one;
                            }

                            split.host = trans.gameObject;
                        }
                        else
                        {
                            split.host = abcTreeNode.linkedGameObj;
                        }
                    }

                    // Feshly created splits may not have their mesh set yet
                    if (split.mesh == null)
                        split.mesh = AddMeshComponents(split.host);
                    if (split.clear)
                        split.mesh.Clear();

                    if (split.points.Count > 0)
                        split.mesh.SetVertices(split.points.List);
                    if (split.normals.Count > 0)
                        split.mesh.SetNormals(split.normals.List);
                    if (split.tangents.Count > 0)
                        split.mesh.SetTangents(split.tangents.List);
                    if (split.uv0.Count > 0)
                        split.mesh.SetUVs(0, split.uv0.List);
                    if (split.uv1.Count > 0)
                        split.mesh.SetUVs(1, split.uv1.List);
                    if (split.velocities.Count > 0)
                        split.mesh.SetUVs(3, split.velocities.List);
                    if (split.colors.Count > 0)
                        split.mesh.SetColors(split.colors.List);

                    // update the bounds
                    var data = m_splitData[s];
                    split.mesh.bounds = new Bounds(data.center, data.extents);

                    if (split.clear)
                    {
                        int submeshCount = m_splitSummaries[s].submeshCount;
                        split.mesh.subMeshCount = submeshCount;
                        MeshRenderer renderer = split.host.GetComponent<MeshRenderer>();
                        Material[] currentMaterials = renderer.sharedMaterials;
                        int nmat = currentMaterials.Length;
                        if (nmat != submeshCount)
                        {
                            Material[] materials = new Material[submeshCount];
                            int copyTo = (nmat < submeshCount ? nmat : submeshCount);
                            for (int i = 0; i < copyTo; ++i)
                            {
                                materials[i] = currentMaterials[i];
                            }
                            renderer.sharedMaterials = materials;
                        }
                    }

                    split.clear = false;
                    split.host.SetActive(true);
                }
                else
                {
                    split.host.SetActive(false);
                }
            }

            for (int smi = 0; smi < m_sampleSummary.submeshCount; ++smi)
            {
                var submesh = m_submeshes[smi];
                if (submesh.update)
                {
                    var sum = m_submeshSummaries[smi];
                    var split = m_splits[sum.splitIndex];
                    split.mesh.SetTriangles(submesh.indices.List, sum.submeshIndex);
                }
            }
        }

        Mesh AddMeshComponents(GameObject go)
        {
            Mesh mesh = null;
            MeshFilter meshFilter = go.GetComponent<MeshFilter>();
            bool hasMesh = meshFilter != null && meshFilter.sharedMesh != null && meshFilter.sharedMesh.name.IndexOf("dyn: ") == 0;

            if (!hasMesh)
            {
                mesh = new Mesh { name = "dyn: " + go.name };
#if UNITY_2017_3_OR_NEWER
                mesh.indexFormat = IndexFormat.UInt32;
#endif
                mesh.MarkDynamic();
                if (meshFilter == null)
                {
                    meshFilter = go.AddComponent<MeshFilter>();
                }
                meshFilter.sharedMesh = mesh;

                if (go.GetComponent<MeshRenderer>() == null)
                    go.AddComponent<MeshRenderer>();
            }
            else
            {
                mesh = UnityEngine.Object.Instantiate(meshFilter.sharedMesh);
                meshFilter.sharedMesh = mesh;
                mesh.name = "dyn: " + go.name;
            }

            return mesh;
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
