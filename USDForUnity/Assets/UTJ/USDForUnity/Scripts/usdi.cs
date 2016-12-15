using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using UnityEngine;

namespace UTJ
{
    public static class usdi
    {
        public struct Context
        {
            public IntPtr ptr;
            public static implicit operator bool(Context v) { return v.ptr != IntPtr.Zero; }
        }

        public struct Attribute
        {
            public IntPtr ptr;
            public static implicit operator bool(Attribute v) { return v.ptr != IntPtr.Zero; }
        }

        public struct Schema
        {
            public IntPtr ptr;
            public static implicit operator bool(Schema v) { return v.ptr != IntPtr.Zero; }
        }

        public struct Xform
        {
            public IntPtr ptr;
            public static implicit operator bool(Xform v) { return v.ptr != IntPtr.Zero; }
            public static implicit operator Schema(Xform v) { Schema r; r.ptr = v.ptr; return r; }
        }

        public struct Camera
        {
            public IntPtr ptr;
            public static implicit operator bool(Camera v) { return v.ptr != IntPtr.Zero; }
            public static implicit operator Schema(Camera v) { Schema r; r.ptr = v.ptr; return r; }
            public static implicit operator Xform(Camera v) { Xform r; r.ptr = v.ptr; return r; }
        }

        public struct Mesh
        {
            public IntPtr ptr;
            public static implicit operator bool(Mesh v) { return v.ptr != IntPtr.Zero; }
            public static implicit operator Schema(Mesh v) { Schema r; r.ptr = v.ptr; return r; }
            public static implicit operator Xform(Mesh v) { Xform r; r.ptr = v.ptr; return r; }
        }

        public struct Points
        {
            public IntPtr ptr;
            public static implicit operator bool(Points v) { return v.ptr != IntPtr.Zero; }
            public static implicit operator Schema(Points v) { Schema r; r.ptr = v.ptr; return r; }
            public static implicit operator Xform(Points v) { Xform r; r.ptr = v.ptr; return r; }
        }

        public enum AttributeType
        {
            Unknown,
            Bool, Byte, Int, UInt,
            Half, Half2, Half3, Half4, QuatH,
            Float, Float2, Float3, Float4, QuatF,
            Double, Double2, Double3, Double4, QuatD,
            Float2x2, Float3x3, Float4x4,
            Double2x2, Double3x3, Double4x4,
            String, Token, Asset,

            UnknownArray = 0x100,
            BoolArray, ByteArray, IntArray, UIntArray,
            HalfArray, Half2Array, Half3Array, Half4Array, QuatHArray,
            FloatArray, Float2Array, Float3Array, Float4Array, QuatFArray,
            DoubleArray, Double2Array, Double3Array, Double4Array, QuatDArray,
            Float2x2Array, Float3x3Array, Float4x4Array,
            Double2x2Array, Double3x3Array, Double4x4Array,
            StringArray, TokenArray, AssetArray,
        };

        public enum InterpolationType
        {
            None,
            Linear,
        };

        public enum NormalCalculationType
        {
            Never,
            WhenMissing,
            Always,
        };
        public enum TangentCalculationType
        {
            Never,
            WhenMissing,
            Always,
        };

        public enum TopologyVariance
        {
            Constant, // both vertices and topologies are constant
            Homogenous, // vertices are not constant (= animated). topologies are constant.
            Heterogenous, // both vertices and topologies are not constant
        };

        public static double defaultTime
        {
            get { return Double.NaN; }
        }

        [Serializable]
        public struct UpdateFlags
        {
            public uint bits;

            public bool sampleUpdated       { get { return (bits & 0x1) != 0; } }
            public bool importConfigChanged { get { return (bits & 0x2) != 0; } }
            public bool variantSetChanged   { get { return (bits & 0x4) != 0; } }
            public bool payloadLoaded { get { return (bits & 0x8) != 0; } }
            public bool payloadUnloaded { get { return (bits & 0x10) != 0; } }
        }

        [Serializable]
        public class VariantSets
        {
            public string[] setNames;
            public string[][] variantNames;

            public VariantSets(int nsets)
            {
                setNames = new string[nsets];
                variantNames = new string[nsets][];
            }
            public int Count { get { return setNames.Length; } }
        }

        [Serializable]
        public struct ImportSettings
        {
            public const int Size = 0x14;

            public InterpolationType interpolation;
            public NormalCalculationType normalCalculation;
            public TangentCalculationType tangentCalculation;
            public float scale;
            [HideInInspector] public Bool loadAllPayloads;
            [HideInInspector] public Bool triangulate;
            public Bool swapHandedness;
            public Bool swapFaces;
            [HideInInspector] public Bool splitMesh;
            [HideInInspector] public Bool doubleBuffering;

            public static ImportSettings default_value
            {
                get
                {
                    return new ImportSettings
                    {
                        interpolation = InterpolationType.Linear,
                        normalCalculation = NormalCalculationType.WhenMissing,
                        tangentCalculation = TangentCalculationType.Never,
                        scale = 1.0f,
                        loadAllPayloads = true,
                        triangulate = true,
                        swapHandedness = true,
                        swapFaces = true,
                        splitMesh = true,
                        doubleBuffering = true,
                    };
                }
            }
        };

        [Serializable]
        public struct ExportSettings
        {
            public float scale;
            public Bool swap_handedness;
            public Bool swap_faces;
            public Bool instanceable_by_default;

            public static ExportSettings default_value
            {
                get
                {
                    return new ExportSettings
                    {
                        scale = 1.0f,
                        swap_handedness = true,
                        swap_faces = true,
                        instanceable_by_default = false,
                    };
                }
            }
        };


        public struct XformSummary
        {
            public enum Type
            {
                Unknown,
                TRS,
                Matrix,
            }

            public double start, end;
            public Type type;
        };

        public struct XformData
        {
            public enum Flags
            {
                UpdatedMask     = 0xf,
                UpdatedPosition = 0x1,
                UpdatedRotation = 0x2,
                UpdatedScale    = 0x4,
            };

            public int flags;
            public Vector3     position;
            public Quaternion  rotation;
            public Vector3     scale;
            public Matrix4x4   transform;

            public static XformData default_value
            {
                get
                {
                    return new XformData
                    {
                        flags = 0,
                        position = Vector3.zero,
                        rotation = Quaternion.identity,
                        scale = Vector3.one,
                        transform = Matrix4x4.identity,
                    };
                }
            }
        }


        public struct CameraSummary
        {
            public double start, end;
        };

        public struct CameraData
        {
            public float near_clipping_plane;
            public float far_clipping_plane;
            public float field_of_view;    // in degree. vertical one
            public float aspect_ratio;

            public float focus_distance;   // in cm
            public float focal_length;     // in mm
            public float aperture;         // in mm. vertical one


            public static CameraData default_value
            {
                get
                {
                    return new CameraData
                    {
                        near_clipping_plane = 0.3f,
                        far_clipping_plane = 1000.0f,
                        field_of_view = 60.0f,
                        aspect_ratio = 16.0f / 9.0f,
                        focus_distance = 5.0f,
                        focal_length = 0.0f,
                        aperture = 35.0f,
                    };
                }
            }
        }

        public struct MeshSummary
        {
            public double start, end;
            public TopologyVariance topology_variance;
            public int num_bones;
            public int max_bone_weights;
            public Bool has_normals;
            public Bool has_tangents;
            public Bool has_uvs;
            public Bool has_velocities;

            public static MeshSummary default_value { get { return default(MeshSummary); } }
        };

        public struct SubmeshData
        {
            public IntPtr   points;
            public IntPtr   normals;
            public IntPtr   tangents;
            public IntPtr   uvs;
            public IntPtr   indices; // always triangulated
            public IntPtr   weights;
            public int      num_points; // == num_indices

            public Vector3  center;
            public Vector3  extents;

            public static SubmeshData default_value { get { return default(SubmeshData); } }
        };

        public struct MeshData
        {
            public IntPtr   points;
            public IntPtr   velocities;
            public IntPtr   normals;
            public IntPtr   tangents;
            public IntPtr   uvs;
            public IntPtr   counts;
            public IntPtr   indices;
            public IntPtr   indices_triangulated;

            public IntPtr   weights;
            public IntPtr   bindposes;
            public IntPtr   bones;
            public IntPtr   root_bone;

            public int      num_points;
            public int      num_counts;
            public int      num_indices;
            public int      num_indices_triangulated;
            public int      num_bones;
            public int      max_bone_weights;

            public Vector3  center;
            public Vector3  extents;

            public IntPtr   submeshes; // pointer to array of SubmeshData
            public int      num_submeshes;

            public static MeshData default_value
            {
                get
                {
                    return default(MeshData);
                }
            }
        }

        public struct PointsSummary
        {
            public double start, end;
            public Bool has_velocities;
        };

        public struct PointsData
        {
            public IntPtr points;
            public IntPtr velocities;

            public int num_points;

            public static PointsData default_value
            {
                get
                {
                    return default(PointsData);
                }
            }
        }

        public struct AttributeSummary
        {
            public double start, end;
            public AttributeType type;
            public int num_samples;
        };

        public struct AttributeData
        {
            public IntPtr data;
            public int num_elements;
        };


        public enum Platform
        {
            Unknown,
            // any 32bit architectures are treated as "Unknown" :)
            Windows_x86_64,
            Linux_x86_64,
            Mac_x86_64,
            Android_ARM64,
            iOS_ARM64,
            PS4,
        };

        [DllImport ("usdiHelper")] public static extern Platform GetPlatform();
        [DllImport ("usdiHelper")] public static extern IntPtr GetModulePath();
        [DllImport ("usdiHelper")] public static extern void AddDLLSearchPath(IntPtr path);
        [DllImport ("usdiHelper")] public static extern void AddDLLSearchPath(string path);
        [DllImport ("usdiHelper")] public static extern void usdiSetPluginPath(string path);


        [DllImport("usdi")] public static extern void usdiInitialize();
        [DllImport("usdi")] public static extern void usdiFinalize();


        [DllImport ("usdi")] public static extern IntPtr        usdiGetRenderEventFunc();

        // Context interface
        [DllImport ("usdi")] public static extern Context       usdiCreateContext();
        [DllImport ("usdi")] public static extern void          usdiDestroyContext(Context ctx);
        [DllImport ("usdi")] public static extern Bool          usdiOpen(Context ctx, string path);
        [DllImport ("usdi")] public static extern Bool          usdiCreateStage(Context ctx, string path);
        [DllImport ("usdi")] public static extern Bool          usdiSave(Context ctx);
        [DllImport ("usdi")] public static extern Bool          usdiSaveAs(Context ctx, string path);

        [DllImport ("usdi")] public static extern void          usdiSetImportSettings(Context ctx, ref ImportSettings v);
        [DllImport ("usdi")] public static extern void          usdiGetImportSettings(Context ctx, ref ImportSettings v);
        [DllImport ("usdi")] public static extern void          usdiSetExportSettings(Context ctx, ref ExportSettings v);
        [DllImport ("usdi")] public static extern void          usdiGetExportSettings(Context ctx, ref ExportSettings v);

        [DllImport ("usdi")] public static extern Schema        usdiCreateOverride(Context ctx, string prim_path);
        [DllImport ("usdi")] public static extern Xform         usdiCreateXform(Context ctx, Schema parent, string name);
        [DllImport ("usdi")] public static extern Camera        usdiCreateCamera(Context ctx, Schema parent, string name);
        [DllImport ("usdi")] public static extern Mesh          usdiCreateMesh(Context ctx, Schema parent, string name);
        [DllImport ("usdi")] public static extern Points        usdiCreatePoints(Context ctx, Schema parent, string name);

        [DllImport ("usdi")] public static extern Schema        usdiGetRoot(Context ctx);
        [DllImport ("usdi")] public static extern int           usdiGetNumMasters(Context ctx);
        [DllImport ("usdi")] public static extern Schema        usdiGetMaster(Context ctx, int i);
        [DllImport ("usdi")] public static extern Schema        usdiFindSchema(Context ctx, string path);

        [DllImport ("usdi")] public static extern void          usdiNotifyForceUpdate(Context ctx);
        [DllImport ("usdi")] public static extern void          usdiUpdateAllSamples(Context ctx, double t);
        [DllImport ("usdi")] public static extern void          usdiRebuildSchemaTree(Context ctx);

        // Prim interface
        [DllImport ("usdi")] public static extern int           usdiPrimGetID(Schema schema);
        [DllImport ("usdi")] public static extern IntPtr        usdiPrimGetPath(Schema schema);
        [DllImport ("usdi")] public static extern IntPtr        usdiPrimGetName(Schema schema);
        [DllImport ("usdi")] public static extern IntPtr        usdiPrimGetUsdTypeName(Schema schema);

        [DllImport ("usdi")] public static extern Schema        usdiPrimGetMaster(Schema schema);
        [DllImport ("usdi")] public static extern int           usdiPrimGetNumInstances(Schema schema);
        [DllImport ("usdi")] public static extern Schema        usdiPrimGetInstance(Schema schema, int i);
        [DllImport ("usdi")] public static extern void          usdiPrimSetInstanceable(Schema schema, Bool v);
        [DllImport ("usdi")] public static extern Bool          usdiPrimAddReference(Schema schema, string asset_path, string prim_path);

        [DllImport ("usdi")] public static extern Schema        usdiPrimGetParent(Schema schema);
        [DllImport ("usdi")] public static extern int           usdiPrimGetNumChildren(Schema schema);
        [DllImport ("usdi")] public static extern Schema        usdiPrimGetChild(Schema schema, int i);

        [DllImport ("usdi")] public static extern int           usdiPrimGetNumAttributes(Schema schema);
        [DllImport ("usdi")] public static extern Attribute     usdiPrimGetAttribute(Schema schema, int i);
        [DllImport ("usdi")] public static extern Attribute     usdiPrimFindAttribute(Schema schema, string name, AttributeType type = AttributeType.Unknown);
        [DllImport ("usdi")] public static extern Attribute     usdiPrimCreateAttribute(Schema schema, string name, AttributeType type, AttributeType internal_type = AttributeType.Unknown);
        
        [DllImport ("usdi")] public static extern int           usdiPrimGetNumVariantSets(Schema schema);
        [DllImport ("usdi")] public static extern IntPtr        usdiPrimGetVariantSetName(Schema schema, int iset);
        [DllImport ("usdi")] public static extern int           usdiPrimGetNumVariants(Schema schema, int iset);
        [DllImport ("usdi")] public static extern IntPtr        usdiPrimGetVariantName(Schema schema, int iset, int ival);
        [DllImport ("usdi")] public static extern int           usdiPrimGetVariantSelection(Schema schema, int iset);
        [DllImport ("usdi")] public static extern Bool          usdiPrimSetVariantSelection(Schema schema, int iset, int ival);

        [DllImport ("usdi")] public static extern UpdateFlags   usdiPrimGetUpdateFlags(Schema schema);
        [DllImport ("usdi")] public static extern UpdateFlags   usdiPrimGetUpdateFlagsPrev(Schema schema);
        [DllImport ("usdi")] public static extern void          usdiPrimUpdateSample(Schema schema, double t);
        [DllImport ("usdi")] public static extern IntPtr        usdiPrimGetUserData(Schema schema);
        [DllImport ("usdi")] public static extern void          usdiPrimSetUserData(Schema schema, IntPtr data);

        public static VariantSets usdiPrimGetVariantSets(Schema schema)
        {
            int nset = usdiPrimGetNumVariantSets(schema);
            var vsets = new VariantSets(nset);
            for (int iset = 0; iset < nset; ++iset)
            {
                var name = S(usdiPrimGetVariantSetName(schema, iset));
                int nval = usdiPrimGetNumVariants(schema, iset);
                var vals = new string[nval + 1];
                for (int ival = 0; ival < nval; ++ival)
                {
                    vals[ival] = S(usdiPrimGetVariantName(schema, iset, ival));
                }
                vals[nval] = " ";
                vsets.setNames[iset] = name;
                vsets.variantNames[iset] = vals;
            }
            return vsets;
        }

        public static string usdiPrimGetPathS(Schema schema) { return S(usdiPrimGetPath(schema)); }
        public static string usdiPrimGetNameS(Schema schema) { return S(usdiPrimGetName(schema)); }
        public static string usdiPrimGetUsdTypeNameS(Schema schema) { return S(usdiPrimGetUsdTypeName(schema)); }

        // Xform interface
        [DllImport ("usdi")] public static extern Xform         usdiAsXform(Schema schema);
        [DllImport ("usdi")] public static extern Bool          usdiXformReadSample(Xform xf, ref XformData dst, double t);
        [DllImport ("usdi")] public static extern Bool          usdiXformWriteSample(Xform xf, ref XformData src, double t);

        // Camera interface
        [DllImport ("usdi")] public static extern Camera        usdiAsCamera(Schema schema);
        [DllImport ("usdi")] public static extern Bool          usdiCameraReadSample(Camera cam, ref CameraData dst, double t);
        [DllImport ("usdi")] public static extern Bool          usdiCameraWriteSample(Camera cam, ref CameraData src, double t);

        // Mesh interface
        [DllImport ("usdi")] public static extern Mesh          usdiAsMesh(Schema schema);
        [DllImport ("usdi")] public static extern void          usdiMeshGetSummary(Mesh mesh, ref MeshSummary dst);
        [DllImport ("usdi")] public static extern Bool          usdiMeshReadSample(Mesh mesh, ref MeshData dst, double t, Bool copy);
        [DllImport ("usdi")] public static extern Bool          usdiMeshWriteSample(Mesh mesh, ref MeshData src, double t);

        // Points interface
        [DllImport ("usdi")] public static extern Points        usdiAsPoints(Schema schema);
        [DllImport ("usdi")] public static extern void          usdiPointsGetSummary(Points points, ref PointsSummary dst);
        [DllImport ("usdi")] public static extern Bool          usdiPointsReadSample(Points points, ref PointsData dst, double t, Bool copy);
        [DllImport ("usdi")] public static extern Bool          usdiPointsWriteSample(Points points, ref PointsData src, double t);

        // Attribute interface
        [DllImport ("usdi")] public static extern IntPtr        usdiAttrGetName(Attribute attr);
        [DllImport ("usdi")] public static extern IntPtr        usdiAttrGetTypeName(Attribute attr);
        [DllImport ("usdi")] public static extern void          usdiAttrGetSummary(Attribute attr, ref AttributeSummary dst);
        [DllImport ("usdi")] public static extern bool          usdiAttrReadSample(Attribute attr, ref AttributeData dst, double t, Bool copy);
        [DllImport ("usdi")] public static extern bool          usdiAttrWriteSample(Attribute attr, ref AttributeData src, double t);

        [DllImport ("usdi")] public static extern IntPtr        usdiIndexStringArray(IntPtr v, int i);
        [DllImport ("usdi")] public static extern void          usdiMeshAssignRootBone(Mesh mesh, ref MeshData dst, string v);
        [DllImport ("usdi")] public static extern void          usdiMeshAssignBones(Mesh mesh, ref MeshData dst, string[] v, int n);

        public static string[] usdiMeshGetBoneNames(Mesh mesh, ref MeshData src)
        {
            string[] ret = new string[src.num_bones];
            for (int i = 0; i < src.num_bones; ++i)
            {
                ret[i] = S(usdiIndexStringArray(src.bones, i));
            }
            return ret;
        }

        public class AssetRef
        {
            public string name;
            public string path;
            public string[] paths;
        }

        public static AssetRef[] usdiGetReferencingAssets(Schema schema)
        {
            var ret = new List<AssetRef>();
            var summary = new AttributeSummary();

            var n = usdiPrimGetNumAttributes(schema);
            for (int ai = 0; ai < n; ++ai)
            {
                var attr = usdiPrimGetAttribute(schema, ai);
                usdiAttrGetSummary(attr, ref summary);

                if(summary.type == AttributeType.Asset)
                {
                    var data = new AttributeData();
                    usdiAttrReadSample(attr, ref data, defaultTime, true);
                    ret.Add(new AssetRef
                    {
                        name = S(usdiAttrGetName(attr)),
                        path = S(data.data)
                    });
                }
                else if (summary.type == AttributeType.AssetArray)
                {
                    var data = new AttributeData();
                    usdiAttrReadSample(attr, ref data, defaultTime, true);

                    var tmp = new IntPtr[data.num_elements];
                    data.data = GetArrayPtr(tmp);
                    usdiAttrReadSample(attr, ref data, defaultTime, true);

                    var e = new AssetRef();
                    e.name = S(usdiAttrGetName(attr));
                    e.paths = new string[data.num_elements];
                    for(int ei = 0; ei < data.num_elements; ++ei)
                    {
                        e.paths[ei] = S(tmp[ei]);
                    }

                    ret.Add(e);
                }
            }

            return ret.ToArray();
        }


        // ext

        [DllImport("usdi")] public static extern Bool usdiVtxCmdIsAvailable();
        [DllImport("usdi")] public static extern IntPtr usdiVtxCmdCreate(string dbg_name);
        [DllImport("usdi")] public static extern void usdiVtxCmdDestroy(IntPtr h);
        [DllImport("usdi")] public static extern void usdiVtxCmdUpdate(IntPtr h, ref MeshData data, IntPtr vb, IntPtr ib);
        [DllImport("usdi")] public static extern void usdiVtxCmdUpdateSub(IntPtr h, ref SubmeshData data, IntPtr vb, IntPtr ib);
        [DllImport("usdi")] public static extern void usdiVtxCmdWait();


        public delegate void usdiMonoDelegate(IntPtr arg);
        [DllImport("usdi")] public static extern void usdiTaskDestroy(IntPtr task);
        [DllImport("usdi")] public static extern void usdiTaskRun(IntPtr task);
        [DllImport("usdi")] public static extern bool usdiTaskIsRunning(IntPtr task);
        [DllImport("usdi")] public static extern void usdiTaskWait(IntPtr task);

        [DllImport("usdi")] public static extern IntPtr usdiTaskCreateMonoDelegate(usdiMonoDelegate func, IntPtr arg, string dbg_name);
        [DllImport("usdi")] public static extern IntPtr usdiTaskCreateMeshReadSample(Mesh mesh, ref MeshData dst, ref double t);
        [DllImport("usdi")] public static extern IntPtr usdiTaskCreatePointsReadSample(Points points, ref PointsData dst, ref double t);
        [DllImport("usdi")] public static extern IntPtr usdiTaskCreateAttrReadSample(Attribute points, ref AttributeData dst, ref double t);
        [DllImport("usdi")] public static extern IntPtr usdiTaskCreateComposite(IntPtr tasks, int num);

#if UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void usdiUniTransformAssign(UnityEngine.Transform trans, ref XformData data);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void usdiUniTransformNotfyChange(UnityEngine.Transform trans);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void usdiUniMeshAssignBounds(UnityEngine.Mesh mesh, ref Vector3 center, ref Vector3 extents);
#else
        public static void usdiUniTransformAssign(UnityEngine.Transform trans, ref XformData data)
        {
            if ((data.flags & (int)usdi.XformData.Flags.UpdatedPosition) != 0)
            {
                trans.localPosition = data.position;
            }
            if ((data.flags & (int)usdi.XformData.Flags.UpdatedRotation) != 0)
            {
                trans.localRotation = data.rotation;
            }
            if ((data.flags & (int)usdi.XformData.Flags.UpdatedScale) != 0)
            {
                trans.localScale = data.scale;
            }
        }
        public static void usdiUniTransformNotfyChange(UnityEngine.Transform trans)
        {
            // nothing to do
        }
        public static void usdiUniMeshAssignBounds(UnityEngine.Mesh mesh, ref Vector3 center, ref Vector3 extents)
        {
            mesh.bounds = new Bounds(center, extents);
        }
#endif


        public class VertexUpdateCommand
        {
            IntPtr handle;

            public VertexUpdateCommand(string dbg_name)
            {
                handle = usdiVtxCmdCreate(dbg_name);
            }

            ~VertexUpdateCommand()
            {
                usdiVtxCmdDestroy(handle);
            }

            public void Update(ref MeshData data, IntPtr vb, IntPtr ib)
            {
                usdiVtxCmdUpdate(handle, ref data, vb, ib);
            }

            public void Update(ref SubmeshData data, IntPtr vb, IntPtr ib)
            {
                usdiVtxCmdUpdateSub(handle, ref data, vb, ib);
            }
        }


        public class Task
        {
            protected IntPtr m_handle;

            public Task()
            {
            }

            public Task(IntPtr handle)
            {
                m_handle = handle;
            }

            ~Task()
            {
                usdiTaskDestroy(m_handle);
            }

            public void Run()
            {
                usdiTaskRun(m_handle);
            }

            public bool IsRunning()
            {
                return usdiTaskIsRunning(m_handle);
            }

            public void Wait()
            {
                usdiTaskWait(m_handle);
            }
        }

        public class DelegateTask : Task
        {
            usdiMonoDelegate m_func;
            GCHandle m_arg;

            public DelegateTask(usdiMonoDelegate f, object arg, string dbg_name = "")
            {
                m_func = f;
                m_arg = GCHandle.Alloc(arg);
                m_handle = usdiTaskCreateMonoDelegate(m_func, (IntPtr)m_arg, dbg_name);
            }

            public DelegateTask(usdiMonoDelegate f, string dbg_name = "")
            {
                m_func = f;
                m_handle = usdiTaskCreateMonoDelegate(m_func, IntPtr.Zero, dbg_name);
            }
        }

        public class CompositeTask : Task
        {
            IntPtr[] m_handles;

            public CompositeTask(IntPtr[] handles)
            {
                m_handles = handles;
                m_handle = usdiTaskCreateComposite(GetArrayPtr(m_handles), m_handles.Length);
            }
        }


        public static T GetOrAddComponent<T>(GameObject go) where T : Component
        {
            var c = go.GetComponent<T>();
            if (c == null)
            {
                c = go.AddComponent<T>();
            }
            return c;
        }

        public static string S(IntPtr cstring) { return Marshal.PtrToStringAnsi(cstring); }
        public static IntPtr GetArrayPtr(Array v) { return v == null ? IntPtr.Zero : Marshal.UnsafeAddrOfPinnedArrayElement(v, 0); }

        [MethodImpl(MethodImplOptions.NoInlining)]
        public static void InitializePluginPass1()
        {
            usdi.AddDLLSearchPath(GetModulePath());

            string platform_suffix = "";
            switch(usdi.GetPlatform())
            {
                case Platform.Windows_x86_64: platform_suffix = "_win64"; break;
                case Platform.Linux_x86_64:   platform_suffix = "_linux"; break;
                case Platform.Mac_x86_64:     platform_suffix = "_mac"; break;
                case Platform.Android_ARM64:  platform_suffix = "_android"; break;
                case Platform.PS4:            platform_suffix = "_ps4"; break;
            }

            var usdPluginDir = Application.streamingAssetsPath + "/UTJ/USDForUnity/plugins" + platform_suffix;
            usdi.AddDLLSearchPath(usdPluginDir + "/lib");
            usdi.usdiSetPluginPath(usdPluginDir);
        }

        // separate pass because loading usdi.dll will fail in InitializePluginPass1()
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static void InitializePluginPass2()
        {
            usdi.usdiInitialize();
        }

        public static void FinalizePlugin()
        {
            usdi.usdiFinalize();
        }
    }

}