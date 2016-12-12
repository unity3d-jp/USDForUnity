using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using System.Threading;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif



namespace UTJ
{
    [ExecuteInEditMode]
    [AddComponentMenu("UTJ/USD/Exporter")]
    public class usdiExporter : MonoBehaviour
    {
        #region impl

        static string CreateName(UnityEngine.Object target)
        {
            return target.name + "_" + target.GetInstanceID().ToString("X8");
        }

        public abstract class ComponentCapturer
        {
            protected usdiExporter m_exporter;
            protected ComponentCapturer m_parent;
            protected usdi.Schema m_usd;

            public usdiExporter exporter { get { return m_exporter; } }
            public usdi.Context ctx { get { return m_exporter.m_ctx; } }
            public ComponentCapturer parent { get { return m_parent; } }
            public usdi.Schema usd { get { return m_usd; } }
            public string primPath { get { return usdi.usdiPrimGetPathS(m_usd); } }
            public abstract void Capture(double t); // called from main thread
            public abstract void Flush(double t); // called from worker thread

            protected ComponentCapturer(usdiExporter exporter, ComponentCapturer parent)
            {
                m_exporter = exporter;
                m_parent = parent;
            }
        }

        public class RootCapturer : ComponentCapturer
        {
            public RootCapturer(usdiExporter exporter, usdi.Schema usd)
                : base(exporter, null)
            {
                m_usd = usd;
            }

            public override void Capture(double t) // called from main thread
            {
                // do nothing
            }
            public override void Flush(double t) // called from worker thread
            {
                // do nothing
            }
        }

        public class TransformCapturer : ComponentCapturer
        {
            Transform m_target;
            usdi.XformData m_data = usdi.XformData.default_value;
            bool m_inherits = true;
            bool m_scale = true;
            bool m_captureEveryFrame = true;
            int m_count = 0;

            public bool inherits {
                get { return m_inherits; }
                set { m_inherits = value; }
            }
            public bool scale
            {
                get { return m_scale; }
                set { m_scale = value; }
            }

            public TransformCapturer(usdiExporter exporter, ComponentCapturer parent, Transform target, bool create_usd_node = true)
                : base(exporter, parent)
            {
                m_target = target;
                if(create_usd_node)
                {
                    m_usd = usdi.usdiCreateXform(ctx, parent.usd, CreateName(target));
                }

                if(m_target.gameObject.isStatic)
                {
                    m_captureEveryFrame = false;
                }

                var config = target.GetComponent<usdiTransformExportConfig>();
                if(config)
                {
                    m_captureEveryFrame = config.m_captureEveryFrame;
                }
            }

            public override void Capture(double t) // called from main thread
            {
                if (m_target == null) { return; }

                if(m_captureEveryFrame || m_count == 0)
                {
                    if (inherits)
                    {
                        m_data.position = m_target.localPosition;
                        m_data.rotation = m_target.localRotation;
                        m_data.scale = scale ? m_target.localScale : Vector3.one;
                    }
                    else
                    {
                        m_data.position = m_target.position;
                        m_data.rotation = m_target.rotation;
                        m_data.scale = scale ? m_target.lossyScale : Vector3.one;
                    }
                }
            }

            public override void Flush(double t) // called from worker thread
            {
                if (m_target == null) { return; }

                if (m_captureEveryFrame || m_count == 0)
                {
                    t = m_count == 0 ? usdi.defaultTime : t;
                    usdi.usdiXformWriteSample(usdi.usdiAsXform(m_usd), ref m_data, t);
                    ++m_count;
                }
            }
        }

        public class CameraCapturer : TransformCapturer
        {
            Camera m_target;
            usdi.CameraData m_data = usdi.CameraData.default_value;
            int m_count = 0;

            public CameraCapturer(usdiExporter exporter, ComponentCapturer parent, Camera target)
                : base(exporter, parent, target.GetComponent<Transform>(), false)
            {
                m_usd = usdi.usdiCreateCamera(ctx, parent.usd, CreateName(target));
                m_target = target;
                //target.GetComponent<usdiCameraExportConfig>();
            }

            public override void Capture(double t) // called from main thread
            {
                base.Capture(t);
                if (m_target == null) { return; }

                m_data.near_clipping_plane = m_target.nearClipPlane;
                m_data.far_clipping_plane = m_target.farClipPlane;
                m_data.field_of_view = m_target.fieldOfView;
                //m_data.focal_length = cparams.m_focalLength;
                //m_data.focus_distance = cparams.m_focusDistance;
                //m_data.aperture = cparams.m_aperture;
                //m_data.aspect_ratio = cparams.GetAspectRatio();
            }

            public override void Flush(double t) // called from worker thread
            {
                base.Flush(t);
                if (m_target == null) { return; }

                t = m_count == 0 ? usdi.defaultTime : t;
                usdi.usdiCameraWriteSample(usdi.usdiAsCamera(m_usd), ref m_data, t);
                ++m_count;
            }
        }

        public class MeshBuffer
        {
            public int[] indices;
            public Vector3[] vertices;
            public Vector3[] normals;
            public Vector4[] tangents;
            public Vector2[] uvs;

            public BoneWeight[] weights;
            public string rootBone;
            public string[] bones;
        }

        public static void CaptureMesh(
            usdi.Mesh usd, Mesh mesh, Cloth cloth, MeshBuffer dst_buf, ref usdi.MeshData data,
            bool capture_normals, bool capture_tangents, bool capture_uvs, bool capture_weights, bool capture_indices,
            Mesh srcMesh = null, string rootBone = null, string[] bones = null)
        {
            dst_buf.indices = capture_indices ? mesh.triangles : null;
            dst_buf.uvs = capture_uvs ? mesh.uv : null;
            if (cloth == null)
            {
                dst_buf.vertices = mesh.vertices;
                dst_buf.normals = capture_normals ? mesh.normals : null;
            }
            else
            {
                dst_buf.vertices = cloth.vertices;
                dst_buf.normals = capture_normals ? cloth.normals : null;
            }
            dst_buf.tangents = capture_tangents ? mesh.tangents : null;

            dst_buf.weights = capture_weights ? srcMesh.boneWeights : null;
            if (rootBone != null && bones != null)
            {
                dst_buf.rootBone = rootBone;
                dst_buf.bones = bones;
            }
            else
            {
                dst_buf.rootBone = null;
                dst_buf.bones = null;
            }

            data = usdi.MeshData.default_value;
            if (dst_buf.vertices != null)
            {
                data.points = usdi.GetArrayPtr(dst_buf.vertices);
                data.num_points = dst_buf.vertices.Length;
            }
            if(dst_buf.indices != null)
            {
                data.indices = usdi.GetArrayPtr(dst_buf.indices);
                data.num_indices = dst_buf.indices.Length;
            }
            if (dst_buf.normals != null)
            {
                data.normals = usdi.GetArrayPtr(dst_buf.normals);
            }
            if (dst_buf.tangents != null)
            {
                data.tangents = usdi.GetArrayPtr(dst_buf.tangents);
            }
            if (dst_buf.uvs != null)
            {
                data.uvs = usdi.GetArrayPtr(dst_buf.uvs);
            }
            if (dst_buf.weights != null && dst_buf.bones != null)
            {
                data.weights = usdi.GetArrayPtr(dst_buf.weights);
                data.num_bones = dst_buf.bones.Length;
                usdi.usdiMeshAssignBones(usd, ref data, dst_buf.bones, dst_buf.bones.Length);
                usdi.usdiMeshAssignRootBone(usd, ref data, dst_buf.rootBone);
            }
        }

        public class MeshCapturer : TransformCapturer
        {
            MeshRenderer m_target;
            MeshBuffer m_mesh_buffer;
            usdi.MeshData m_data = default(usdi.MeshData);
            bool m_captureNormals = true;
            bool m_captureTangents = true;
            bool m_captureUVs = true;
            bool m_captureEveryFrame = false;
            bool m_captureEveryFrameUV = false;
            bool m_captureEveryFrameIndices = false;
            int m_count = 0;

            public MeshCapturer(usdiExporter exporter, ComponentCapturer parent, MeshRenderer target)
                : base(exporter, parent, target.GetComponent<Transform>(), false)
            {
                m_usd = usdi.usdiCreateMesh(ctx, parent.usd, CreateName(target));
                m_target = target;
                m_mesh_buffer = new MeshBuffer();

                m_captureNormals = exporter.m_captureMeshNormals;
                m_captureTangents = exporter.m_captureMeshTangents;
                m_captureUVs = exporter.m_captureMeshUVs;

                var conf = target.GetComponent<usdiMeshExportConfig>();
                if(conf != null)
                {
                    m_captureNormals = conf.m_captureNormals;
                    m_captureTangents = conf.m_captureTangents;
                    m_captureUVs = conf.m_captureUVs;
                    m_captureEveryFrame = conf.m_captureEveryFrame;
                    m_captureEveryFrameUV = conf.m_captureEveryFrameUV;
                    m_captureEveryFrameIndices = conf.m_captureEveryFrameIndices;
                }
            }

            public override void Capture(double t) // called from main thread
            {
                base.Capture(t);
                if (m_target == null) { return; }

                if(m_captureEveryFrame || m_count == 0)
                {
                    bool captureUV = m_captureUVs && (m_count == 0 || m_captureEveryFrameUV);
                    bool captureIndices = m_count == 0 || m_captureEveryFrameIndices;
                    CaptureMesh(
                        usdi.usdiAsMesh(m_usd), m_target.GetComponent<MeshFilter>().sharedMesh, null, m_mesh_buffer, ref m_data,
                        m_captureNormals, m_captureTangents, captureUV, false, captureIndices);
                }
            }

            public override void Flush(double t) // called from worker thread
            {
                base.Flush(t);
                if (m_target == null) { return; }

                if (m_captureEveryFrame || m_count == 0)
                {
                    t = m_count == 0 ? usdi.defaultTime : t;
                    usdi.usdiMeshWriteSample(usdi.usdiAsMesh(m_usd), ref m_data, t);
                    ++m_count;
                }
            }
        }

        public class SkinnedMeshCapturer : TransformCapturer
        {
            SkinnedMeshRenderer m_target;
            Mesh m_mesh;
            MeshBuffer m_mesh_buffer;
            usdi.MeshData m_data = default(usdi.MeshData);
            bool m_captureNormals = true;
            bool m_captureTangents = true;
            bool m_captureUVs = true;
            bool m_captureBones = false;
            bool m_captureEveryFrame = true;
            bool m_captureEveryFrameUV = false;
            bool m_captureEveryFrameIndices = false;
            int m_count = 0;

            public SkinnedMeshCapturer(usdiExporter exporter, ComponentCapturer parent, SkinnedMeshRenderer target)
                : base(exporter, parent, target.GetComponent<Transform>(), false)
            {
                m_usd = usdi.usdiCreateMesh(ctx, parent.usd, CreateName(target));
                m_target = target;
                m_mesh_buffer = new MeshBuffer();

                if (m_target.GetComponent<Cloth>() != null)
                {
                    base.scale = false;
                }

                m_captureNormals = exporter.m_captureMeshNormals;
                m_captureTangents = exporter.m_captureMeshTangents;
                m_captureUVs = exporter.m_captureMeshUVs;
                m_captureBones = exporter.m_captureSkinnedMeshAs == SkinnedMeshCaptureMode.BoneAndWeights;

                var conf = target.GetComponent<usdiMeshExportConfig>();
                if (conf != null)
                {
                    m_captureNormals = conf.m_captureNormals;
                    m_captureTangents = conf.m_captureTangents;
                    m_captureUVs = conf.m_captureUVs;
                    m_captureEveryFrame = conf.m_captureEveryFrame;
                    m_captureEveryFrameUV = conf.m_captureEveryFrameUV;
                    m_captureEveryFrameIndices = conf.m_captureEveryFrameIndices;
                }
            }

            public override void Capture(double t) // called from main thread
            {
                base.Capture(t);
                if (m_target == null) { return; }

                if (m_captureEveryFrame || m_count == 0)
                {
                    if (m_mesh == null) { m_mesh = new Mesh(); }
                    m_target.BakeMesh(m_mesh);
                    var srcMesh = m_target.sharedMesh;
                    bool captureUV = m_captureUVs && (m_count == 0 || m_captureEveryFrameUV);
                    bool captureIndices = m_count == 0 || m_captureEveryFrameIndices;
                    bool captureBones = m_captureBones && m_count == 0;

                    string rootBoneName = null;
                    string[] boneNames = null;
                    if (captureBones)
                    {
                        var root = m_exporter.FindNode(m_target.rootBone);
                        if(root != null)
                        {
                            rootBoneName = root.capturer.primPath;
                        }

                        var bones = m_target.bones;
                        if(bones != null)
                        {
                            boneNames = new string[bones.Length];
                            for (int i = 0; i < bones.Length; ++i)
                            {
                                var bone = m_exporter.FindNode(bones[i]);
                                boneNames[i] = bone.capturer.primPath;
                            }
                        }
                    }
                    CaptureMesh(
                        usdi.usdiAsMesh(m_usd), m_mesh, m_target.GetComponent<Cloth>(), m_mesh_buffer, ref m_data,
                        m_captureNormals, m_captureTangents, captureUV, captureBones, captureIndices,
                        srcMesh, rootBoneName, boneNames);
                }
            }

            public override void Flush(double t) // called from worker thread
            {
                base.Flush(t);
                if (m_target == null) { return; }

                if (m_captureEveryFrame || m_count == 0)
                {
                    t = m_count == 0 ? usdi.defaultTime : t;
                    usdi.usdiMeshWriteSample(usdi.usdiAsMesh(m_usd), ref m_data, t);
                    ++m_count;
                }
            }
        }

        public class ParticleCapturer : TransformCapturer
        {
            ParticleSystem m_target;
            usdi.Attribute m_attr_rotatrions;
            usdi.PointsData m_data = usdi.PointsData.default_value;
            usdi.AttributeData m_dataRot;

            ParticleSystem.Particle[] m_buf_particles;
            Vector3[] m_buf_positions;
            Vector4[] m_buf_rotations;

            bool m_captureRotations = true;


            public ParticleCapturer(usdiExporter exporter, ComponentCapturer parent, ParticleSystem target)
                : base(exporter, parent, target.GetComponent<Transform>(), false)
            {
                m_usd = usdi.usdiCreatePoints(ctx, parent.usd, CreateName(target));
                m_target = target;

                var config = target.GetComponent<usdiParticleExportConfig>();
                if(config != null)
                {
                    m_captureRotations = config.m_captureRotations;
                }
                if (m_captureRotations)
                {
                    m_attr_rotatrions = usdi.usdiPrimCreateAttribute(m_usd, "rotations", usdi.AttributeType.Float4Array);
                }
            }

            public override void Capture(double t) // called from main thread
            {
                base.Capture(t);
                if (m_target == null) { return; }

                // create buffer
                int count_max =
#if UNITY_5_5_OR_NEWER
                    m_target.main.maxParticles;
#else
                    m_target.maxParticles;
#endif
                bool allocated = false;
                if (m_buf_particles == null)
                {
                    m_buf_particles = new ParticleSystem.Particle[count_max];
                    m_buf_positions = new Vector3[count_max];
                    m_buf_rotations = new Vector4[count_max];
                    allocated = true;
                }
                else if (m_buf_particles.Length != count_max)
                {
                    Array.Resize(ref m_buf_particles, count_max);
                    Array.Resize(ref m_buf_positions, count_max);
                    Array.Resize(ref m_buf_rotations, count_max);
                    allocated = true;
                }

                if (allocated)
                {
                    m_data.points = usdi.GetArrayPtr(m_buf_positions);
                    m_dataRot.data = usdi.GetArrayPtr(m_buf_rotations);
                }

                // copy particle positions & rotations to buffer
                int count = m_target.GetParticles(m_buf_particles);
                for (int i = 0; i < count; ++i)
                {
                    m_buf_positions[i] = m_buf_particles[i].position;
                }
                if (m_captureRotations)
                {
                    for (int i = 0; i < count; ++i)
                    {
                        m_buf_rotations[i] = m_buf_particles[i].axisOfRotation;
                        m_buf_rotations[i].w = m_buf_particles[i].rotation;
                    }
                }

                m_data.num_points = count;
                m_dataRot.num_elements = count;
            }

            public override void Flush(double t) // called from worker thread
            {
                base.Flush(t);
                if (m_target == null) { return; }

                usdi.usdiPointsWriteSample(usdi.usdiAsPoints(m_usd), ref m_data, t);
                if (m_captureRotations)
                {
                    usdi.usdiAttrWriteSample(m_attr_rotatrions, ref m_dataRot, t);
                }
            }
        }

        public class CustomCapturerHandler : TransformCapturer
        {
            usdiCustomComponentCapturer m_target;

            public CustomCapturerHandler(usdiExporter exporter, ComponentCapturer parent, usdiCustomComponentCapturer target)
                : base(exporter, parent, target.GetComponent<Transform>(), false)
            {
                m_target = target;
            }

            public override void Capture(double t)
            {
                base.Capture(t);
                if (m_target == null) { return; }
                m_target.Capture(t);
            }

            public override void Flush(double t) // called from worker thread
            {
                base.Flush(t);
                if (m_target == null) { return; }
                m_target.Flush(t);
            }
        }

#if UNITY_EDITOR
        void ForceDisableBatching()
        {
            var method = typeof(UnityEditor.PlayerSettings).GetMethod("SetBatchingForPlatform", BindingFlags.NonPublic | BindingFlags.Static);
            method.Invoke(null, new object[] { BuildTarget.StandaloneWindows, 0, 0 });
            method.Invoke(null, new object[] { BuildTarget.StandaloneWindows64, 0, 0 });
        }
#endif

#endregion


        public enum Scope
        {
            EntireScene,
            CurrentBranch,
        }

        public enum SkinnedMeshCaptureMode
        {
            VertexCache,
            BoneAndWeights,
        }

        #region fields
        [Header("USD")]

        public string m_outputPath;
        public float m_scale = 1.0f;
        public bool m_swapHandedness = true;
        public bool m_swapFaces = true;

        [Header("Capture Components")]

        public Scope m_scope = Scope.EntireScene;
        public bool m_ignoreDisabled = true;
        [Space(8)]
        public bool m_captureMeshRenderer = true;
        public bool m_captureSkinnedMeshRenderer = true;
        public bool m_captureParticleSystem = true;
        public bool m_captureCamera = true;
        public bool m_customCapturer = true;
        [Space(8)]
        public SkinnedMeshCaptureMode m_captureSkinnedMeshAs = SkinnedMeshCaptureMode.VertexCache;
        public bool m_captureMeshNormals = true;
        public bool m_captureMeshTangents = true;
        public bool m_captureMeshUVs = true;

        [Header("Capture Setting")]

        [Tooltip("Flush to file at every N frames. 0=never")]
        public int m_flushEveryNthFrames = 0;
        [Tooltip("Start capture on start.")]
        public bool m_captureOnStart = false;
        [Tooltip("Automatically end capture when reached Max Capture Frame. 0=Infinite")]
        public int m_maxCaptureFrame = 0;

        [Header("Debug")]
#if UNITY_EDITOR
        public bool m_forceSingleThread;
        public bool m_detailedLog;
#endif

        usdi.Context m_ctx;
        ComponentCapturer m_root;
        List<ComponentCapturer> m_capturers = new List<ComponentCapturer>();
        bool m_recording;
        float m_time;
        float m_elapsed;
        int m_frameCount;
        int m_prevFrame = -1;

        usdi.Task m_asyncFlush;
        float m_timeFlush;
        #endregion


        #region properties
        public bool isRecording { get { return m_recording; } }
        public float time { get { return m_time; } }
        public float elapsed { get { return m_elapsed; } }
        public float frameCount { get { return m_frameCount; } }
        #endregion


        #region impl
        void usdiLog(string message)
        {
#if UNITY_EDITOR
            if (m_detailedLog)
            {
                Debug.Log(message);
            }
#endif
        }

        T[] GetTargets<T>() where T : Component
        {
            if(m_scope == Scope.CurrentBranch)
            {
                return GetComponentsInChildren<T>();
            }
            else
            {
                return FindObjectsOfType<T>();
            }
        }



        bool ShouldBeIgnored(Behaviour target)
        {
            return m_ignoreDisabled && (!target.gameObject.activeInHierarchy || !target.enabled);
        }
        bool ShouldBeIgnored(ParticleSystem target)
        {
            return m_ignoreDisabled && (!target.gameObject.activeInHierarchy);
        }
        bool ShouldBeIgnored(MeshRenderer target)
        {
            if (m_ignoreDisabled && (!target.gameObject.activeInHierarchy || !target.enabled)) { return true; }
            var mesh = target.GetComponent<MeshFilter>().sharedMesh;
            if (mesh == null) { return true; }
            return false;
        }
        bool ShouldBeIgnored(SkinnedMeshRenderer target)
        {
            if (m_ignoreDisabled && (!target.gameObject.activeInHierarchy || !target.enabled)) { return true; }
            var mesh = target.sharedMesh;
            if (mesh == null) { return true; }
            return false;
        }


        // capture node tree for "Preserve Tree Structure" option.
        public class CaptureNode
        {
            public CaptureNode parent;
            public List<CaptureNode> children = new List<CaptureNode>();
            public Type componentType;

            public Transform trans;
            public ComponentCapturer capturer;
        }

        Dictionary<Transform, CaptureNode> m_captureNodes;
        List<CaptureNode> m_rootNodes;

        CaptureNode FindNode(Transform t)
        {
            CaptureNode ret;
            if(m_captureNodes.TryGetValue(t, out ret)) { return ret; }
            return null;
        }

        CaptureNode ConstructTree(Transform trans)
        {
            if(trans == null) { return null; }
            usdiLog("ConstructTree() : " + trans.name);

            // return existing one if found
            CaptureNode ret;
            if (m_captureNodes.TryGetValue(trans, out ret)) { return ret; }

            ret = new CaptureNode();
            ret.trans = trans;
            m_captureNodes.Add(trans, ret);

            var parent = ConstructTree(trans.parent);
            if (parent != null)
            {
                ret.parent = parent;
                parent.children.Add(ret);
            }
            else
            {
                m_rootNodes.Add(ret);
            }

            return ret;
        }


        void SetupComponentCapturer(CaptureNode parent, CaptureNode node)
        {
            usdiLog("SetupComponentCapturer() " + node.trans.name);

            node.parent = parent;
            var parent_capturer = parent == null ? m_root : parent.capturer;


            if (node.componentType == null)
            {
                node.capturer = new TransformCapturer(this, parent_capturer, node.trans);
            }
            else if (node.componentType == typeof(Transform))
            {
                node.capturer = new TransformCapturer(this, parent_capturer, node.trans.GetComponent<Transform>());
            }
            else if (node.componentType == typeof(Camera))
            {
                node.capturer = new CameraCapturer(this, parent_capturer, node.trans.GetComponent<Camera>());
            }
            else if (node.componentType == typeof(MeshRenderer))
            {
                node.capturer = new MeshCapturer(this, parent_capturer, node.trans.GetComponent<MeshRenderer>());
            }
            else if (node.componentType == typeof(SkinnedMeshRenderer))
            {
                node.capturer = new SkinnedMeshCapturer(this, parent_capturer, node.trans.GetComponent<SkinnedMeshRenderer>());
            }
            else if (node.componentType == typeof(ParticleSystem))
            {
                node.capturer = new ParticleCapturer(this, parent_capturer, node.trans.GetComponent<ParticleSystem>());
            }
            else if (node.componentType == typeof(usdiCustomComponentCapturer))
            {
                node.capturer = new CustomCapturerHandler(this, parent_capturer, node.trans.GetComponent<usdiCustomComponentCapturer>());
            }

            if(node.capturer != null)
            {
                m_capturers.Add(node.capturer);
            }

            foreach (var c in node.children)
            {
                SetupComponentCapturer(node, c);
            }
        }

        void ConstructCaptureTree()
        {
            m_root = new RootCapturer(this, usdi.usdiGetRoot(m_ctx));
            m_captureNodes = new Dictionary<Transform, CaptureNode>();
            m_rootNodes = new List<CaptureNode>();

            var bones = new HashSet<Transform>();

            // construct tree
            // (bottom-up)
            if (m_captureCamera)
            {
                foreach (var t in GetTargets<Camera>())
                {
                    if (ShouldBeIgnored(t)) { continue; }
                    var node = ConstructTree(t.GetComponent<Transform>());
                    node.componentType = t.GetType();
                }
            }
            if (m_captureMeshRenderer)
            {
                foreach (var t in GetTargets<MeshRenderer>())
                {
                    if (ShouldBeIgnored(t)) { continue; }
                    var node = ConstructTree(t.GetComponent<Transform>());
                    node.componentType = t.GetType();
                }
            }
            if (m_captureSkinnedMeshRenderer)
            {
                foreach (var t in GetTargets<SkinnedMeshRenderer>())
                {
                    if (ShouldBeIgnored(t)) { continue; }
                    var node = ConstructTree(t.GetComponent<Transform>());
                    node.componentType = t.GetType();

                    // capture bones as well
                    if(m_captureSkinnedMeshAs == SkinnedMeshCaptureMode.BoneAndWeights)
                    {
                        if(t.rootBone != null)
                        {
                            bones.Add(t.rootBone);
                        }
                        if(t.bones != null)
                        {
                            foreach (var bone in t.bones)
                            {
                                bones.Add(bone);
                            }
                        }
                    }
                }
            }
            if (m_captureParticleSystem)
            {
                foreach (var t in GetTargets<ParticleSystem>())
                {
                    if (ShouldBeIgnored(t)) { continue; }
                    var node = ConstructTree(t.GetComponent<Transform>());
                    node.componentType = t.GetType();
                }
            }
            if (m_customCapturer)
            {
                foreach (var t in GetTargets<usdiCustomComponentCapturer>())
                {
                    if (ShouldBeIgnored(t)) { continue; }
                    var node = ConstructTree(t.GetComponent<Transform>());
                    node.componentType = typeof(usdiCustomComponentCapturer);
                }
            }

            foreach(var t in bones)
            {
                var node = ConstructTree(t.GetComponent<Transform>());
                node.componentType = t.GetType();
            }

            // make component capturers (top-down)
            foreach (var c in m_rootNodes)
            {
                SetupComponentCapturer(null, c);
            }
        }

        void ApplyExportConfig()
        {
            usdi.ExportSettings conf = usdi.ExportSettings.default_value;
            conf.scale = m_scale;
            conf.swap_handedness = m_swapHandedness;
            conf.swap_faces = m_swapFaces;
            usdi.usdiSetExportSettings(m_ctx, ref conf);
        }


        public bool BeginCapture()
        {
            if(m_recording) {
                Debug.Log("usdiExporter: already started");
                return false;
            }

            // create context and open archive
            m_ctx = usdi.usdiCreateContext();
            if(!usdi.usdiCreateStage(m_ctx, m_outputPath))
            {
                Debug.Log("usdiExporter: failed to create " + m_outputPath);
                usdi.usdiDestroyContext(m_ctx);
                m_ctx = default(usdi.Context);
                return false;
            }
            ApplyExportConfig();

            // create capturers
            ConstructCaptureTree();

            m_recording = true;
            //m_time = m_conf.startTime;
            m_frameCount = 0;
    
            Debug.Log("usdiExporter: start " + m_outputPath);
            return true;
        }

        public void EndCapture()
        {
            if (!m_recording) { return; }

            FlushUSD();
            m_capturers.Clear();
            usdi.usdiDestroyContext(m_ctx); // flush archive
            m_ctx = default(usdi.Context);
            m_recording = false;
            m_time = 0.0f;
            m_frameCount = 0;

            Debug.Log("usdiExporter: end: " + m_outputPath);
        }

        public void OneShot()
        {
            if (BeginCapture())
            {
                ProcessCapture();
                EndCapture();
            }
        }

        void WaitFlush()
        {
            if (m_asyncFlush != null)
            {
                m_asyncFlush.Wait();
            }
        }

        void FlushUSD()
        {
            WaitFlush();
            usdi.usdiSave(m_ctx);
        }


        void ProcessCapture()
        {
            if (!m_recording) { return; }

            // for some reason, come here twice on first frame. skip second run.
            int frame = Time.frameCount;
            if (frame == m_prevFrame) { return; }
            m_prevFrame = frame;

            // wait for complete previous flush
            WaitFlush();

            float begin_time = Time.realtimeSinceStartup;

            // capture components
            foreach (var c in m_capturers)
            {
                c.Capture(m_time);
            }

            // kick flush task
#if UNITY_EDITOR
            if(m_forceSingleThread)
            {
                foreach (var c in m_capturers) { c.Flush(time); }
            }
            else
#endif
            {
                if(m_asyncFlush == null)
                {
                    m_asyncFlush = new usdi.DelegateTask((var) => {
                        try
                        {
                            foreach (var c in m_capturers) { c.Flush(m_timeFlush); }
                        }
                        finally
                        {
                        }
                    }, "usdiExporter: " + gameObject.name);
                }
                m_timeFlush = m_time;
                m_asyncFlush.Run();
            }

            m_time += Time.deltaTime;
            ++m_frameCount;

            // flush to file when needed
            if(m_flushEveryNthFrames > 0 && m_frameCount % m_flushEveryNthFrames == 0)
            {
                FlushUSD();
            }

            m_elapsed = Time.realtimeSinceStartup - begin_time;
            usdiLog("usdiExporter.ProcessCapture(): " + (m_elapsed * 1000.0f) + "ms");

            if(m_maxCaptureFrame > 0 && m_frameCount >= m_maxCaptureFrame)
            {
                EndCapture();
            }
        }

        IEnumerator ProcessRecording()
        {
            yield return new WaitForEndOfFrame();
            if(!m_recording) { yield break; }

            ProcessCapture();
        }

        void UpdateOutputPath()
        {
            if (m_outputPath == null || m_outputPath == "")
            {
                m_outputPath = "Assets/StreamingAssets/" + gameObject.name + ".usdc";
            }
        }
        #endregion



        #region callbacks
#if UNITY_EDITOR
        void Reset()
        {
            ForceDisableBatching();
            UpdateOutputPath();
        }
#endif

        void Awake()
        {
            usdi.InitializePluginPass1();
            usdi.InitializePluginPass2();
        }

        void OnEnable()
        {
            UpdateOutputPath();
        }

        void OnDisable()
        {
            EndCapture();
        }

        void Start()
        {
            if (m_captureOnStart
#if UNITY_EDITOR
                 && EditorApplication.isPlaying
#endif
                )
            {
                BeginCapture();
            }
        }

        void Update()
        {
            if(m_recording)
            {
                StartCoroutine(ProcessRecording());
            }
        }
        #endregion
    }
}
