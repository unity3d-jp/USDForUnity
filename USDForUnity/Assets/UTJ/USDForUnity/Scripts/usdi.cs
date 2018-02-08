using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using UnityEngine;

namespace UTJ.USD
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
            Bool, Byte, Int, UInt, Int64, UInt64,
            Half, Half2, Half3, Half4, QuatH,
            Float, Float2, Float3, Float4, QuatF,
            Double, Double2, Double3, Double4, QuatD,
            Float2x2, Float3x3, Float4x4,
            Double2x2, Double3x3, Double4x4,
            String, Token, Asset,

            UnknownArray = 0x100,
            BoolArray, ByteArray, IntArray, UIntArray, Int64Array, UInt64Array,
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
            public InterpolationType interpolation;
            public NormalCalculationType normalCalculation;
            public TangentCalculationType tangentCalculation;
            [HideInInspector] public int splitUnit;
            [HideInInspector] public int maxBoneWeights;
            public float scaleFactor;
            [HideInInspector] public Bool loadAllPayloads;
            [HideInInspector] public Bool triangulate;
            public Bool swapHandedness;
            public Bool swapFaces;

            public static ImportSettings default_value
            {
                get
                {
                    return new ImportSettings
                    {
                        interpolation = InterpolationType.Linear,
                        normalCalculation = NormalCalculationType.WhenMissing,
                        tangentCalculation = TangentCalculationType.Never,
#if UNITY_2017_3_OR_NEWER
                        splitUnit = 0x7fffffff,
#else
                        splitUnit = 65000,
#endif
                        maxBoneWeights = 4,
                        scaleFactor = 1.0f,
                        loadAllPayloads = true,
                        triangulate = true,
                        swapHandedness = false,
                        swapFaces = false,
                    };
                }
            }
        };

        [Serializable]
        public struct ExportSettings
        {
            public float scale;
            public Bool swapHandedness;
            public Bool swapFaces;
            public Bool instanceableByDefault;

            public static ExportSettings default_value
            {
                get
                {
                    return new ExportSettings
                    {
                        scale = 1.0f,
                        swapHandedness = true,
                        swapFaces = true,
                        instanceableByDefault = false,
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

            [Serializable]
            public struct Flags
            {
                public uint bits;


                public bool updatedPosition { get { return (bits & 0x1) != 0; } }
                public bool updatedRotation { get { return (bits & 0x2) != 0; } }
                public bool updatedScale { get { return (bits & 0x4) != 0; } }
            }

            public Flags flags;
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
                        flags = new Flags(),
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
            public TopologyVariance topologyVariance;
            public int boneCount;
            public int maxBoneWeights;
            public Bool hasVelocities;
            public Bool hasNormals;
            public Bool hasColors;
            public Bool hasUV0;
            public Bool hasTangents;

            public static MeshSummary default_value { get { return default(MeshSummary); } }
        };

        public struct MeshSampleSummary
        {
            public int splitCount;
            public int submeshCount;
            public int vertexCount;
            public int indexCount;
            public Bool topologyChanged;
        }

        public struct MeshSplitSummary
        {
            public int submeshCount;
            public int submeshOffset;
            public int vertexCount;
            public int vertexOffset;
            public int indexCount;
            public int indexOffset;
        }


        public struct SplitData
        {
            public IntPtr   points;
            public IntPtr   normals;
            public IntPtr   colors;
            public IntPtr   uvs;
            public IntPtr   tangents;
            public IntPtr   velocities;
            public IntPtr   indices; // always triangulated
            public IntPtr   weights;
            public int      num_points; // == num_indices

            public Vector3  center;
            public Vector3  extents;

            public static SplitData default_value { get { return default(SplitData); } }
        };

        public struct SubmeshData
        {
            public IntPtr indices;
        }

        public struct MeshData
        {
            public IntPtr   points;
            public IntPtr   velocities;
            public IntPtr   normals;
            public IntPtr   tangents;
            public IntPtr   uv0;
            public IntPtr   uv1;
            public IntPtr   colors;
            public IntPtr   counts;
            public IntPtr   indices;

            public IntPtr   weights;
            public IntPtr   bindposes;
            public IntPtr   bones;
            public IntPtr   rootBone;

            public int      pointCount;
            public int      faceCount;
            public int      indexCount;
            public int      boneCount;
            public int      maxBoneWeights;

            public Vector3  center;
            public Vector3  extents;

            public IntPtr   submeshes; // pointer to array of SubmeshData
            public int      submeshCount;

            public static MeshData defaultValue
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
            public IntPtr widths;
            public IntPtr ids64;
            public IntPtr ids32;

            public int pointCount;

            public static PointsData defaultValue
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

        [DllImport ("usdiRT")] public static extern Platform GetPlatform();
        [DllImport ("usdiRT")] public static extern IntPtr GetModulePath();
        [DllImport ("usdiRT")] public static extern void AddDLLSearchPath(IntPtr path);
        [DllImport ("usdiRT")] public static extern void AddDLLSearchPath(string path);
        [DllImport ("usdiRT")] public static extern void usdiSetPluginPath(string path);


        [DllImport("usdi")] public static extern void usdiInitialize();
        [DllImport("usdi")] public static extern void usdiFinalize();
        [DllImport("usdi")] public static extern Bool usdiConvertUSDToAlembic(string usd_path, string abc_path);


        [DllImport ("usdi")] public static extern IntPtr        usdiGetRenderEventFunc();

        [DllImport ("usdi")] public static extern void          usdiAddAssetSearchPath(string path);
        [DllImport ("usdi")] public static extern void          usdiClearAssetSearchPath();

        // Context interface
        [DllImport("usdi")] public static extern void           usdiClearContextsWithPath(string path);
        [DllImport ("usdi")] public static extern Context       usdiCreateContext(int uid);
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
        [DllImport ("usdi")] public static extern int           usdiGetNumSchemas(Context ctx);
        [DllImport ("usdi")] public static extern Schema        usdiGetSchema(Context ctx, int i);
        [DllImport ("usdi")] public static extern int           usdiGetNumMasters(Context ctx);
        [DllImport ("usdi")] public static extern Schema        usdiGetMaster(Context ctx, int i);
        [DllImport ("usdi")] public static extern Schema        usdiFindSchema(Context ctx, string path_or_name);

        [DllImport ("usdi")] public static extern void          usdiNotifyForceUpdate(Context ctx);
        [DllImport ("usdi")] public static extern void          usdiUpdateAllSamples(Context ctx, double t);
        [DllImport ("usdi")] public static extern void          usdiRebuildSchemaTree(Context ctx);

        // Prim interface
        [DllImport ("usdi")] public static extern int           usdiPrimGetID(Schema schema);
        [DllImport ("usdi")] public static extern IntPtr        usdiPrimGetPath(Schema schema);
        [DllImport ("usdi")] public static extern IntPtr        usdiPrimGetName(Schema schema);
        [DllImport ("usdi")] public static extern IntPtr        usdiPrimGetUsdTypeName(Schema schema);

        [DllImport ("usdi")] public static extern Bool          usdiPrimIsEditable(Schema schema);
        [DllImport ("usdi")] public static extern Bool          usdiPrimIsInstance(Schema schema);
        [DllImport ("usdi")] public static extern Bool          usdiPrimIsMaster(Schema schema);
        [DllImport ("usdi")] public static extern Bool          usdiPrimIsInMaster(Schema schema);
        [DllImport ("usdi")] public static extern Schema        usdiPrimGetMaster(Schema schema);
        [DllImport ("usdi")] public static extern int           usdiPrimGetNumInstances(Schema schema);
        [DllImport ("usdi")] public static extern Schema        usdiPrimGetInstance(Schema schema, int i);
        [DllImport ("usdi")] public static extern void          usdiPrimSetInstanceable(Schema schema, Bool v);
        [DllImport ("usdi")] public static extern Bool          usdiPrimAddReference(Schema schema, string asset_path, string prim_path);

        [DllImport ("usdi")] public static extern Schema        usdiPrimGetParent(Schema schema);
        [DllImport ("usdi")] public static extern int           usdiPrimGetNumChildren(Schema schema);
        [DllImport ("usdi")] public static extern Schema        usdiPrimGetChild(Schema schema, int i);
        [DllImport ("usdi")] public static extern Schema        usdiPrimFindChild(Schema schema, string path_or_name, Bool recursive);

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

        public static void usdiEachSchemas(Context ctx, Action<Schema> act)
        {
            var n = usdiGetNumSchemas(ctx);
            for (int i = 0; i < n; ++i)
            {
                act.Invoke(usdiGetSchema(ctx, i));
            }
        }

        public static void usdiEachMasters(Context ctx, Action<Schema> act)
        {
            var n = usdiGetNumMasters(ctx);
            for (int i = 0; i < n; ++i)
            {
                act.Invoke(usdiGetMaster(ctx, i));
            }
        }

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
        [DllImport ("usdi")] public static extern Xform     usdiAsXform(Schema schema);
        [DllImport ("usdi")] public static extern Bool      usdiXformReadSample(Xform xf, ref XformData dst, double t);
        [DllImport ("usdi")] public static extern Bool      usdiXformWriteSample(Xform xf, ref XformData src, double t);
        public delegate void usdiXformSampleCallback(ref XformData data, double t);
        [DllImport ("usdi")] public static extern int       usdiXformEachSample(Xform xf, usdiXformSampleCallback cb);

        // Camera interface
        [DllImport ("usdi")] public static extern Camera    usdiAsCamera(Schema schema);
        [DllImport ("usdi")] public static extern Bool      usdiCameraReadSample(Camera cam, ref CameraData dst, double t);
        [DllImport ("usdi")] public static extern Bool      usdiCameraWriteSample(Camera cam, ref CameraData src, double t);
        public delegate void usdiCameraSampleCallback(ref CameraData data, double t);
        [DllImport ("usdi")] public static extern int       usdiCameraEachSample(Camera cam, usdiCameraSampleCallback cb);

        // Mesh interface
        [DllImport ("usdi")] public static extern Mesh      usdiAsMesh(Schema schema);
        [DllImport ("usdi")] public static extern void      usdiMeshGetSummary(Mesh mesh, ref MeshSummary dst);
        [DllImport ("usdi")] public static extern Bool      usdiMeshReadSample(Mesh mesh, ref MeshData dst, double t, Bool copy);
        [DllImport ("usdi")] public static extern Bool      usdiMeshWriteSample(Mesh mesh, ref MeshData src, double t);
        public delegate void usdiMeshSampleCallback(ref MeshData data, double t);
        [DllImport ("usdi")] public static extern int       usdiMeshEachSample(Mesh mesh, usdiMeshSampleCallback cb);
        [DllImport ("usdi")] public static extern Bool      usdiMeshPreComputeNormals(Mesh mesh, Bool gen_tangents, Bool overwrite);

        // Points interface
        [DllImport ("usdi")] public static extern Points    usdiAsPoints(Schema schema);
        [DllImport ("usdi")] public static extern void      usdiPointsGetSummary(Points points, ref PointsSummary dst);
        [DllImport ("usdi")] public static extern Bool      usdiPointsReadSample(Points points, ref PointsData dst, double t, Bool copy);
        [DllImport ("usdi")] public static extern Bool      usdiPointsWriteSample(Points points, ref PointsData src, double t);
        public delegate void usdiPointsSampleCallback(ref PointsData data, double t);
        [DllImport ("usdi")] public static extern int       usdiPointsEachSample(Points points, usdiPointsSampleCallback cb);

        // Attribute interface
        [DllImport ("usdi")] public static extern IntPtr    usdiAttrGetName(Attribute attr);
        [DllImport ("usdi")] public static extern IntPtr    usdiAttrGetTypeName(Attribute attr);
        [DllImport ("usdi")] public static extern void      usdiAttrGetSummary(Attribute attr, ref AttributeSummary dst);
        [DllImport ("usdi")] public static extern Bool      usdiAttrReadSample(Attribute attr, ref AttributeData dst, double t, Bool copy);
        [DllImport ("usdi")] public static extern Bool      usdiAttrWriteSample(Attribute attr, ref AttributeData src, double t);

        [DllImport ("usdi")] public static extern IntPtr    usdiIndexStringArray(IntPtr v, int i);
        [DllImport ("usdi")] public static extern void      usdiMeshAssignRootBone(Mesh mesh, ref MeshData dst, string v);
        [DllImport ("usdi")] public static extern void      usdiMeshAssignBones(Mesh mesh, ref MeshData dst, string[] v, int n);


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

                    var tmp = new PinnedList<IntPtr>(data.num_elements);
                    data.data = tmp;
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


        // utils

        public static T GetOrAddComponent<T>(GameObject go) where T : Component
        {
            var c = go.GetComponent<T>();
            if (c == null)
            {
                c = go.AddComponent<T>();
            }
            return c;
        }
        public static T GetOrAddComponent<T>(GameObject go, ref bool created) where T : Component
        {
            var c = go.GetComponent<T>();
            if (c == null)
            {
                c = go.AddComponent<T>();
                created = true;
            }
            else
            {
                created = false;
            }
            return c;
        }

        public static string S(IntPtr cstring)
        {
            return Marshal.PtrToStringAnsi(cstring);
        }
        public static string[] SA(IntPtr cstringarray)
        {
            var ret = new List<string>();
            for (int i = 0; ; ++i)
            {
                var s = usdi.usdiIndexStringArray(cstringarray, i);
                if(s == IntPtr.Zero) { break; }
                ret.Add(S(s));
            }
            return ret.ToArray();
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        public static void InitializePluginPass1()
        {
            string usdPluginDir = "";
            switch(usdi.GetPlatform())
            {
            case Platform.Windows_x86_64:
                usdPluginDir = Application.streamingAssetsPath + "/USDForUnity/plugins_win64";
                break;
            case Platform.Linux_x86_64:
                usdPluginDir = Application.streamingAssetsPath + "/USDForUnity/plugins_linux";
                break;
            case Platform.Mac_x86_64:
                usdPluginDir = usdi.S(GetModulePath()) + "/usdi.bundle/Contents/plugins_mac";
                break;
            }

            usdi.usdiSetPluginPath(usdPluginDir);
            usdi.AddDLLSearchPath(usdPluginDir + "/lib");
            usdi.AddDLLSearchPath(GetModulePath());
        }

        public static bool pluginInitialized = false;

        // separate pass because loading usdi.dll will fail in InitializePluginPass1()
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static void InitializePluginPass2()
        {
            usdi.usdiInitialize();
            pluginInitialized = true;
        }

        public static void FinalizePlugin()
        {
            usdi.usdiFinalize();
        }

        public static bool ConvertUSDToAlembic(string usd_path, string abc_path)
        {
            return usdi.usdiConvertUSDToAlembic(usd_path, abc_path);
    }
    }
}
