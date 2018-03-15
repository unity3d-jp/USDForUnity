using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using UnityEngine;

namespace UTJ.USD
{
    public static class usdi
    {
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

        public enum Topology
        {
            Points,
            Lines,
            Triangles,
            Quads,
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

            public static ImportSettings defaultValue
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
            public float scaleFactor;
            public Bool swapHandedness;
            public Bool swapFaces;
            public Bool instanceableByDefault;

            public static ExportSettings defaultValue
            {
                get
                {
                    return new ExportSettings
                    {
                        scaleFactor = 1.0f,
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

            public static XformData defaultValue
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


            public static CameraData defaultValue
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
            public Bool hasTangents;
            public Bool hasUV0;
            public Bool hasUV1;
            public Bool hasColors;
            public Bool constantPoints;
            public Bool constantVelocities;
            public Bool constantNormals;
            public Bool constantTangents;
            public Bool constantUV0;
            public Bool constantUV1;
            public Bool constantColors;

            public static MeshSummary defaultValue
            {
                get
                {
                    return default(MeshSummary);
                }
            }
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

        public struct SubmeshSummary
        {
            public int splitIndex;
            public int submeshIndex;
            public int indexCount;
        }


        public struct SubmeshData
        {
            public IntPtr indices;
        }

        public struct MeshData
        {
            public IntPtr points;
            public IntPtr velocities;
            public IntPtr normals;
            public IntPtr tangents;
            public IntPtr uv0;
            public IntPtr uv1;
            public IntPtr colors;
            public IntPtr indices;
            public IntPtr faces;

            public int vertexCount;
            public int indexCount;
            public int faceCount;

            public Vector3  center;
            public Vector3  extents;

            public IntPtr weights;
            public IntPtr bindposes;
            public IntPtr bones;
            public IntPtr rootBone;
            public int boneCount;
            public int maxBoneWeights;

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


        // Prim interface


        public struct Context
        {
            public IntPtr self;
            public static implicit operator bool(Context v) { return v.self != IntPtr.Zero; }

            public static Context Create(int uid) { return usdiCreateContext(uid); }

            public void Destroy() { usdiDestroyContext(self); self = IntPtr.Zero; }
            public bool Open(string path) { return usdiOpen(self, path); }
            public bool CreateStage(string path) { return usdiCreateStage(self, path); }
            public bool Save() { return usdiSave(self); }
            public bool SaveAs(string path) { return usdiSaveAs(self, path); }

            public void SetImportSettings(ref ImportSettings v) { usdiSetImportSettings(self, ref v); }
            public void GetImportSettings(ref ImportSettings v) { usdiGetImportSettings(self, ref v); }
            public void SetExportSettings(ref ExportSettings v) { usdiSetExportSettings(self, ref v); }
            public void GetExportSettings(ref ExportSettings v) { usdiGetExportSettings(self, ref v); }

            public Schema CreateOverride(string prim_path) { return usdiCreateOverride(self, prim_path); }
            public Xform CreateXform(Schema parent, string name) { return usdiCreateXform(self, parent, name); }
            public Camera CreateCamera(Schema parent, string name) { return usdiCreateCamera(self, parent, name); }
            public Mesh CreateMesh(Schema parent, string name) { return usdiCreateMesh(self, parent, name); }
            public Points CreatePoints(Schema parent, string name) { return usdiCreatePoints(self, parent, name); }

            public Schema GetRoot() { return usdiGetRoot(self); }
            public int GetNumSchemas() { return usdiGetNumSchemas(self); }
            public Schema GetSchema(int i) { return usdiGetSchema(self, i); }
            public int GetNumMasters() { return usdiGetNumMasters(self); }
            public Schema GetMaster(int i) { return usdiGetMaster(self, i); }
            public Schema FindSchema(string path_or_name) { return usdiFindSchema(self, path_or_name); }

            public void NotifyForceUpdate() { usdiNotifyForceUpdate(self); }
            public void UpdateAllSamples(double t) { usdiUpdateAllSamples(self, t); }
            public void RebuildSchemaTree() { usdiRebuildSchemaTree(self); }

            #region internal
            [DllImport("usdi")] static extern void usdiClearContextsWithPath(string path);
            [DllImport("usdi")] static extern Context usdiCreateContext(int uid);
            [DllImport("usdi")] static extern void usdiDestroyContext(IntPtr self);
            [DllImport("usdi")] static extern Bool usdiOpen(IntPtr self, string path);
            [DllImport("usdi")] static extern Bool usdiCreateStage(IntPtr self, string path);
            [DllImport("usdi")] static extern Bool usdiSave(IntPtr self);
            [DllImport("usdi")] static extern Bool usdiSaveAs(IntPtr self, string path);

            [DllImport("usdi")] static extern void usdiSetImportSettings(IntPtr self, ref ImportSettings v);
            [DllImport("usdi")] static extern void usdiGetImportSettings(IntPtr self, ref ImportSettings v);
            [DllImport("usdi")] static extern void usdiSetExportSettings(IntPtr self, ref ExportSettings v);
            [DllImport("usdi")] static extern void usdiGetExportSettings(IntPtr self, ref ExportSettings v);

            [DllImport("usdi")] static extern Schema usdiCreateOverride(IntPtr self, string prim_path);
            [DllImport("usdi")] static extern Xform usdiCreateXform(IntPtr self, Schema parent, string name);
            [DllImport("usdi")] static extern Camera usdiCreateCamera(IntPtr self, Schema parent, string name);
            [DllImport("usdi")] static extern Mesh usdiCreateMesh(IntPtr self, Schema parent, string name);
            [DllImport("usdi")] static extern Points usdiCreatePoints(IntPtr self, Schema parent, string name);

            [DllImport("usdi")] static extern Schema usdiGetRoot(IntPtr self);
            [DllImport("usdi")] static extern int usdiGetNumSchemas(IntPtr self);
            [DllImport("usdi")] static extern Schema usdiGetSchema(IntPtr self, int i);
            [DllImport("usdi")] static extern int usdiGetNumMasters(IntPtr self);
            [DllImport("usdi")] static extern Schema usdiGetMaster(IntPtr self, int i);
            [DllImport("usdi")] static extern Schema usdiFindSchema(IntPtr self, string path_or_name);

            [DllImport("usdi")] static extern void usdiNotifyForceUpdate(IntPtr self);
            [DllImport("usdi")] static extern void usdiUpdateAllSamples(IntPtr self, double t);
            [DllImport("usdi")] static extern void usdiRebuildSchemaTree(IntPtr self);
            #endregion
        }

        public struct Attribute
        {
            public IntPtr self;
            public static implicit operator bool(Attribute v) { return v.self != IntPtr.Zero; }

            public string GetName() { return Marshal.PtrToStringAnsi(usdiAttrGetName(self)); }
            public string GetTypeName() { return Marshal.PtrToStringAnsi(usdiAttrGetTypeName(self)); }
            public void GetSummary(ref AttributeSummary dst) { usdiAttrGetSummary(self, ref dst); }
            public bool ReadSample(ref AttributeData dst, double t) { return usdiAttrReadSample(self, ref dst, t); }
            public bool WriteSample(ref AttributeData src, double t) { return usdiAttrWriteSample(self, ref src, t); }

            #region internal
            [DllImport("usdi")] static extern IntPtr usdiAttrGetName(IntPtr self);
            [DllImport("usdi")] static extern IntPtr usdiAttrGetTypeName(IntPtr self);
            [DllImport("usdi")] static extern void usdiAttrGetSummary(IntPtr self, ref AttributeSummary dst);
            [DllImport("usdi")] static extern Bool usdiAttrReadSample(IntPtr self, ref AttributeData dst, double t);
            [DllImport("usdi")] static extern Bool usdiAttrWriteSample(IntPtr self, ref AttributeData src, double t);
            #endregion
        }

        public struct Schema
        {
            public IntPtr self;
            public static implicit operator bool(Schema v) { return v.self != IntPtr.Zero; }

            public int GetID() { return usdiPrimGetID(self); }
            public string GetPath() { return Marshal.PtrToStringAnsi(usdiPrimGetPath(self)); }
            public string GetName() { return Marshal.PtrToStringAnsi(usdiPrimGetName(self)); }
            public string GetTypeName() { return Marshal.PtrToStringAnsi(usdiPrimGetUsdTypeName(self)); }

            public bool IsEditable() { return usdiPrimIsEditable(self); }
            public bool IsInstance() { return usdiPrimIsInstance(self); }
            public bool IsMaster() { return usdiPrimIsMaster(self); }
            public bool IsInMaster() { return usdiPrimIsInMaster(self); }
            public Schema GetMaster() { return usdiPrimGetMaster(self); }
            public int GetNumInstances() { return usdiPrimGetNumInstances(self); }
            public Schema GetInstance(int i) { return usdiPrimGetInstance(self, i); }
            public void SetInstanceable(bool v) { usdiPrimSetInstanceable(self, v); }
            public bool AddReference(string asset_path, string prim_path) { return usdiPrimAddReference(self, asset_path, prim_path); }

            public Schema GetParent() { return usdiPrimGetParent(self); }
            public int GetNumChildren() { return usdiPrimGetNumChildren(self); }
            public Schema GetChild(int i) { return usdiPrimGetChild(self, i); }
            public Schema FindChild(string path_or_name, bool recursive) { return usdiPrimFindChild(self, path_or_name, recursive); }

            public int GetNumAttributes() { return usdiPrimGetNumAttributes(self); }
            public Attribute GetAttribute(int i) { return usdiPrimGetAttribute(self, i); }
            public Attribute FindAttribute(string name, AttributeType type = AttributeType.Unknown) { return usdiPrimFindAttribute(self, name, type); }
            public Attribute CreateAttribute(string name, AttributeType type, AttributeType internal_type = AttributeType.Unknown) { return usdiPrimCreateAttribute(self, name, type, internal_type); }

            public int GetNumVariantSets() { return usdiPrimGetNumVariantSets(self); }
            public string GetVariantSetName(int iset) { return Marshal.PtrToStringAnsi(usdiPrimGetVariantSetName(self, iset)); }
            public int GetNumVariants(int iset) { return usdiPrimGetNumVariants(self, iset); }
            public string GetVariantName(int iset, int ival) { return Marshal.PtrToStringAnsi(usdiPrimGetVariantName(self, iset, ival)); }
            public int GetVariantSelection(int iset) { return usdiPrimGetVariantSelection(self, iset); }
            public bool SetVariantSelection(int iset, int ival) { return usdiPrimSetVariantSelection(self, iset, ival); }

            public UpdateFlags GetUpdateFlags() { return usdiPrimGetUpdateFlags(self); }
            public UpdateFlags GetUpdateFlagsPrev() { return usdiPrimGetUpdateFlagsPrev(self); }
            public void UpdateSample(double t) { usdiPrimUpdateSample(self, t); }
            public IntPtr GetUserData() { return usdiPrimGetUserData(self); }
            public void SetUserData(IntPtr data) { usdiPrimSetUserData(self, data); }

            public Xform AsXform() { return usdiAsXform(self); }
            public Camera AsCamera() { return usdiAsCamera(self); }
            public Mesh AsMesh() { return usdiAsMesh(self); }
            public Points AsPoints() { return usdiAsPoints(self); }

            #region internal
            [DllImport("usdi")] static extern int usdiPrimGetID(IntPtr self);
            [DllImport("usdi")] static extern IntPtr usdiPrimGetPath(IntPtr self);
            [DllImport("usdi")] static extern IntPtr usdiPrimGetName(IntPtr self);
            [DllImport("usdi")] static extern IntPtr usdiPrimGetUsdTypeName(IntPtr self);

            [DllImport("usdi")] static extern Bool usdiPrimIsEditable(IntPtr self);
            [DllImport("usdi")] static extern Bool usdiPrimIsInstance(IntPtr self);
            [DllImport("usdi")] static extern Bool usdiPrimIsMaster(IntPtr self);
            [DllImport("usdi")] static extern Bool usdiPrimIsInMaster(IntPtr self);
            [DllImport("usdi")] static extern Schema usdiPrimGetMaster(IntPtr self);
            [DllImport("usdi")] static extern int usdiPrimGetNumInstances(IntPtr self);
            [DllImport("usdi")] static extern Schema usdiPrimGetInstance(IntPtr self, int i);
            [DllImport("usdi")] static extern void usdiPrimSetInstanceable(IntPtr self, Bool v);
            [DllImport("usdi")] static extern Bool usdiPrimAddReference(IntPtr self, string asset_path, string prim_path);

            [DllImport("usdi")] static extern Schema usdiPrimGetParent(IntPtr self);
            [DllImport("usdi")] static extern int usdiPrimGetNumChildren(IntPtr self);
            [DllImport("usdi")] static extern Schema usdiPrimGetChild(IntPtr self, int i);
            [DllImport("usdi")] static extern Schema usdiPrimFindChild(IntPtr self, string path_or_name, Bool recursive);

            [DllImport("usdi")] static extern int usdiPrimGetNumAttributes(IntPtr self);
            [DllImport("usdi")] static extern Attribute usdiPrimGetAttribute(IntPtr self, int i);
            [DllImport("usdi")] static extern Attribute usdiPrimFindAttribute(IntPtr self, string name, AttributeType type = AttributeType.Unknown);
            [DllImport("usdi")] static extern Attribute usdiPrimCreateAttribute(IntPtr self, string name, AttributeType type, AttributeType internal_type = AttributeType.Unknown);

            [DllImport("usdi")] static extern int usdiPrimGetNumVariantSets(IntPtr self);
            [DllImport("usdi")] static extern IntPtr usdiPrimGetVariantSetName(IntPtr self, int iset);
            [DllImport("usdi")] static extern int usdiPrimGetNumVariants(IntPtr self, int iset);
            [DllImport("usdi")] static extern IntPtr usdiPrimGetVariantName(IntPtr self, int iset, int ival);
            [DllImport("usdi")] static extern int usdiPrimGetVariantSelection(IntPtr self, int iset);
            [DllImport("usdi")] static extern Bool usdiPrimSetVariantSelection(IntPtr self, int iset, int ival);

            [DllImport("usdi")] static extern UpdateFlags usdiPrimGetUpdateFlags(IntPtr self);
            [DllImport("usdi")] static extern UpdateFlags usdiPrimGetUpdateFlagsPrev(IntPtr self);
            [DllImport("usdi")] static extern void usdiPrimUpdateSample(IntPtr self, double t);
            [DllImport("usdi")] static extern IntPtr usdiPrimGetUserData(IntPtr self);
            [DllImport("usdi")] static extern void usdiPrimSetUserData(IntPtr self, IntPtr data);

            [DllImport("usdi")] static extern Xform usdiAsXform(IntPtr self);
            [DllImport("usdi")] static extern Camera usdiAsCamera(IntPtr self);
            [DllImport("usdi")] static extern Mesh usdiAsMesh(IntPtr self);
            [DllImport("usdi")] static extern Points usdiAsPoints(IntPtr self);
            #endregion
        }

        [StructLayout(LayoutKind.Explicit)]
        public struct Xform
        {
            [FieldOffset(0)] public IntPtr self;
            [FieldOffset(0)] public Schema schema;

            public static implicit operator bool(Xform v) { return v.self != IntPtr.Zero; }
            public static implicit operator Schema(Xform v) { Schema r; r.self = v.self; return r; }

            public bool ReadSample(ref XformData dst) { return usdiXformReadSample(self, ref dst); }
            public bool WriteSample(ref XformData src, double t) { return usdiXformWriteSample(self, ref src, t); }

            public delegate void usdiXformSampleCallback(ref XformData data, double t);
            public int EachSample(usdiXformSampleCallback cb) { return usdiXformEachSample(self, cb); }

            #region internal
            [DllImport("usdi")] static extern Bool usdiXformReadSample(IntPtr self, ref XformData dst);
            [DllImport("usdi")] static extern Bool usdiXformWriteSample(IntPtr self, ref XformData src, double t);
            [DllImport("usdi")] static extern int usdiXformEachSample(IntPtr self, usdiXformSampleCallback cb);
            #endregion

        }

        [StructLayout(LayoutKind.Explicit)]
        public struct Camera
        {
            [FieldOffset(0)] public IntPtr self;
            [FieldOffset(0)] public Schema schema;
            [FieldOffset(0)] public Xform xform;

            public static implicit operator bool(Camera v) { return v.self != IntPtr.Zero; }
            public static implicit operator Schema(Camera v) { return v.schema; }
            public static implicit operator Xform(Camera v) { return v.xform; }

            public bool ReadSample(ref CameraData dst) { return usdiCameraReadSample(self, ref dst); }
            public bool WriteSample(ref CameraData src, double t) { return usdiCameraWriteSample(self, ref src, t); }

            public delegate void usdiCameraSampleCallback(ref CameraData data, double t);
            public int EachSample(usdiCameraSampleCallback cb) { return usdiCameraEachSample(self, cb); }

            #region internal
            [DllImport("usdi")] static extern Bool usdiCameraReadSample(IntPtr self, ref CameraData dst);
            [DllImport("usdi")] static extern Bool usdiCameraWriteSample(IntPtr self, ref CameraData src, double t);
            [DllImport("usdi")] static extern int usdiCameraEachSample(IntPtr self, usdiCameraSampleCallback cb);
            #endregion
        }

        [StructLayout(LayoutKind.Explicit)]
        public struct Mesh
        {
            [FieldOffset(0)] public IntPtr self;
            [FieldOffset(0)] public Schema schema;
            [FieldOffset(0)] public Xform xform;

            public static implicit operator bool(Mesh v) { return v.self != IntPtr.Zero; }
            public static implicit operator Schema(Mesh v) { return v.schema; }
            public static implicit operator Xform(Mesh v) { return v.xform; }

            public void GetSummary(ref MeshSummary dst) { usdiMeshGetSummary(self, ref dst); }
            public void GetSampleSummary(ref MeshSampleSummary dst) { usdiMeshGetSampleSummary(self, ref dst); }
            public bool ReadSample(ref MeshData dst) { return usdiMeshReadSample(self, ref dst); }
            public bool WriteSample(ref MeshData src, double t) { return usdiMeshWriteSample(self, ref src, t); }

            public delegate void usdiMeshSampleCallback(ref MeshData data, double t);
            public int EachSample(usdiMeshSampleCallback cb) { return usdiMeshEachSample(self, cb); }

            public bool PreComputeNormals(bool gen_tangents, bool overwrite) { return usdiMeshPreComputeNormals(self, gen_tangents, overwrite); }

            #region internal
            [DllImport("usdi")] static extern void usdiMeshGetSummary(IntPtr self, ref MeshSummary dst);
            [DllImport("usdi")] static extern void usdiMeshGetSampleSummary(IntPtr self, ref MeshSampleSummary dst);
            [DllImport("usdi")] static extern Bool usdiMeshReadSample(IntPtr self, ref MeshData dst);
            [DllImport("usdi")] static extern Bool usdiMeshWriteSample(IntPtr self, ref MeshData src, double t);
            [DllImport("usdi")] static extern int usdiMeshEachSample(IntPtr self, usdiMeshSampleCallback cb);
            [DllImport("usdi")] static extern Bool usdiMeshPreComputeNormals(IntPtr self, Bool gen_tangents, Bool overwrite);
            #endregion
        }

        [StructLayout(LayoutKind.Explicit)]
        public struct Points
        {
            [FieldOffset(0)] public IntPtr self;
            [FieldOffset(0)] public Schema schema;
            [FieldOffset(0)] public Xform xform;

            public static implicit operator bool(Points v) { return v.self != IntPtr.Zero; }
            public static implicit operator Schema(Points v) { return v.schema; }
            public static implicit operator Xform(Points v) { return v.xform; }

            public void GetSummary(ref PointsSummary dst) { usdiPointsGetSummary(self, ref dst); }
            public bool ReadSample(ref PointsData dst) { return usdiPointsReadSample(self, ref dst); }
            public bool WriteSample(ref PointsData src, double t) { return usdiPointsWriteSample(self, ref src, t); }

            public delegate void usdiPointsSampleCallback(ref PointsData data, double t);
            public int EachSample(usdiPointsSampleCallback cb) { return usdiPointsEachSample(self, cb); }

            #region internal
            [DllImport("usdi")] static extern void usdiPointsGetSummary(IntPtr self, ref PointsSummary dst);
            [DllImport("usdi")] static extern Bool usdiPointsReadSample(IntPtr self, ref PointsData dst);
            [DllImport("usdi")] static extern Bool usdiPointsWriteSample(IntPtr self, ref PointsData src, double t);
            [DllImport("usdi")] static extern int usdiPointsEachSample(IntPtr self, usdiPointsSampleCallback cb);
            #endregion
        }



        public static void usdiEachSchemas(Context ctx, Action<Schema> act)
        {
            var n = ctx.GetNumSchemas();
            for (int i = 0; i < n; ++i)
                act.Invoke(ctx.GetSchema(i));
        }

        public static void usdiEachMasters(Context ctx, Action<Schema> act)
        {
            var n = ctx.GetNumMasters();
            for (int i = 0; i < n; ++i)
                act.Invoke(ctx.GetMaster(i));
        }

        public static VariantSets usdiPrimGetVariantSets(Schema schema)
        {
            int nset = schema.GetNumVariantSets();
            var vsets = new VariantSets(nset);
            for (int iset = 0; iset < nset; ++iset)
            {
                var name = schema.GetVariantSetName(iset);
                int nval = schema.GetNumVariants(iset);
                var vals = new string[nval + 1];
                for (int ival = 0; ival < nval; ++ival)
                {
                    vals[ival] = schema.GetVariantName(iset, ival);
                }
                vals[nval] = " ";
                vsets.setNames[iset] = name;
                vsets.variantNames[iset] = vals;
            }
            return vsets;
        }


        // Misc
        [DllImport("usdi")] static extern IntPtr usdiIndexStringArray(IntPtr v, int i);
        [DllImport("usdi")] static extern void usdiMeshAssignRootBone(Mesh mesh, ref MeshData dst, string v);
        [DllImport("usdi")] static extern void usdiMeshAssignBones(Mesh mesh, ref MeshData dst, string[] v, int n);


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

            var n = schema.GetNumAttributes();
            for (int ai = 0; ai < n; ++ai)
            {
                var attr = schema.GetAttribute(ai);
                attr.GetSummary(ref summary);

                if(summary.type == AttributeType.Asset)
                {
                    var data = new AttributeData();
                    attr.ReadSample(ref data, defaultTime);
                    ret.Add(new AssetRef
                    {
                        name = attr.GetName(),
                        path = S(data.data)
                    });
                }
                else if (summary.type == AttributeType.AssetArray)
                {
                    var data = new AttributeData();
                    attr.ReadSample(ref data, defaultTime);

                    var tmp = new PinnedList<IntPtr>(data.num_elements);
                    data.data = tmp;
                    attr.ReadSample(ref data, defaultTime);

                    var e = new AssetRef();
                    e.name = attr.GetName();
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
