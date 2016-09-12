using System;
using System.Runtime.InteropServices;
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
            Bool,
            Byte,
            Int,
            UInt,
            Float,
            Float2,
            Float3,
            Float4,
            Quaternion,
            Token,
            String,
            Asset,
            UnknownArray = 0x100,
            BoolArray,
            ByteArray,
            IntArray,
            UIntArray,
            FloatArray,
            Float2Array,
            Float3Array,
            Float4Array,
            QuaternionArray,
            TokenArray,
            StringArray,
            AssetArray,
        };

        public enum InterpolationType
        {
            None,
            Linear,
        };

        public enum TopologyVariance
        {
            Constant, // both vertices and topologies are constant
            Homogenous, // vertices are not constant (= animated). topologies are constant.
            Heterogenous, // both vertices and topologies are not constant
        };

        public static double default_time {
            get { return Double.NaN; }
        }

        public struct ImportConfig
        {
            public InterpolationType interpolation;
            public float scale;
            public Bool triangulate;
            public Bool swap_handedness;
            public Bool swap_faces;

            public static ImportConfig default_value
            {
                get
                {
                    return new ImportConfig
                    {
                        interpolation = InterpolationType.Linear,
                        scale = 1.0f,
                        triangulate = true,
                        swap_handedness = true,
                        swap_faces = true,
                    };
                }
            }
        };

        public struct ExportConfig
        {
            public float scale;
            public Bool swap_handedness;
            public Bool swap_faces;

            public static ExportConfig default_value
            {
                get
                {
                    return new ExportConfig
                    {
                        scale = 1.0f,
                        swap_handedness = true,
                        swap_faces = true,
                    };
                }
            }
        };

        public struct XformData
        {
            public Vector3     position;
            public Quaternion  rotation;
            public Vector3     scale;
        }

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
            public uint peak_num_points;
            public uint peak_num_counts;
            public uint peak_num_indices;
            public uint peak_num_indices_triangulated;
            public TopologyVariance topology_variance;
            public Bool has_normals;
            public Bool has_uvs;
            public Bool has_velocities;
        };

        public struct MeshData
        {
            public IntPtr   points;
            public IntPtr   velocities;
            public IntPtr   normals;
            public IntPtr   uvs;
            public IntPtr   counts;
            public IntPtr   indices;
            public IntPtr   indices_triangulated;

            public int      num_points;
            public int      num_counts;
            public int      num_indices;
            public int      num_indices_triangulated;
        }

        public struct PointsSummary
        {
            public int peak_num_points;
            public Bool has_velocities;
        };

        public struct PointsData
        {
            public IntPtr points;
            public IntPtr velocities;

            public int num_points;
        }

        [DllImport ("AddDLLSearchPath")] public static extern void AddDLLSearchPath();
        [DllImport ("AddDLLSearchPath")] public static extern void SetEnv(string name, string value);

        // Context interface
        [DllImport ("usdi")] public static extern Context       usdiCreateContext();
        [DllImport ("usdi")] public static extern void          usdiDestroyContext(Context ctx);
        [DllImport ("usdi")] public static extern Bool          usdiOpen(Context ctx, string path);
        [DllImport ("usdi")] public static extern Bool          usdiCreateStage(Context ctx, string path);
        [DllImport ("usdi")] public static extern Bool          usdiSave(Context ctx);
        [DllImport ("usdi")] public static extern Bool          usdiWrite(Context ctx, string path);

        [DllImport ("usdi")] public static extern void          usdiSetImportConfig(Context ctx, ref ImportConfig conf);
        [DllImport ("usdi")] public static extern void          usdiGetImportConfig(Context ctx, ref ImportConfig conf);
        [DllImport ("usdi")] public static extern void          usdiSetExportConfig(Context ctx, ref ExportConfig conf);
        [DllImport ("usdi")] public static extern void          usdiGetExportConfig(Context ctx, ref ExportConfig conf);
        [DllImport ("usdi")] public static extern Schema        usdiGetRoot(Context ctx);

        // Schema interface
        [DllImport ("usdi")] public static extern int           usdiGetID(Schema schema);
        [DllImport ("usdi")] public static extern IntPtr        usdiGetPath(Schema schema);
        [DllImport ("usdi")] public static extern IntPtr        usdiGetName(Schema schema);
        [DllImport ("usdi")] public static extern IntPtr        usdiGetTypeName(Schema schema);
        [DllImport ("usdi")] public static extern Schema        usdiGetParent(Schema schema);
        [DllImport ("usdi")] public static extern int           usdiGetNumChildren(Schema schema);
        [DllImport ("usdi")] public static extern Schema        usdiGetChild(Schema schema, int i);
        [DllImport ("usdi")] public static extern int           usdiGetNumAttributes(Schema schema);
        [DllImport ("usdi")] public static extern Attribute     usdiGetAttribute(Schema schema, int i);
        [DllImport ("usdi")] public static extern Attribute     usdiFindAttribute(Schema schema, string name);
        [DllImport ("usdi")] public static extern Attribute     usdiCreateAttribute(Schema schema, string name, AttributeType type);

        // Xform interface
        [DllImport ("usdi")] public static extern Xform         usdiAsXform(Schema schema);
        [DllImport ("usdi")] public static extern Xform         usdiCreateXform(Context ctx, Schema parent, string name);
        [DllImport ("usdi")] public static extern Bool          usdiXformReadSample(Xform xf, ref XformData dst, double t);
        [DllImport ("usdi")] public static extern Bool          usdiXformWriteSample(Xform xf, ref XformData src, double t);

        // Camera interface
        [DllImport ("usdi")] public static extern Camera        usdiAsCamera(Schema schema);
        [DllImport ("usdi")] public static extern Camera        usdiCreateCamera(Context ctx, Schema parent, string name);
        [DllImport ("usdi")] public static extern Bool          usdiCameraReadSample(Camera cam, ref CameraData dst, double t);
        [DllImport ("usdi")] public static extern Bool          usdiCameraWriteSample(Camera cam, ref CameraData src, double t);

        // Mesh interface
        [DllImport ("usdi")] public static extern Mesh          usdiAsMesh(Schema schema);
        [DllImport ("usdi")] public static extern Mesh          usdiCreateMesh(Context ctx, Schema parent, string name);
        [DllImport ("usdi")] public static extern void          usdiMeshGetSummary(Mesh mesh, ref MeshSummary dst);
        [DllImport ("usdi")] public static extern Bool          usdiMeshReadSample(Mesh mesh, ref MeshData dst, double t);
        [DllImport ("usdi")] public static extern Bool          usdiMeshWriteSample(Mesh mesh, ref MeshData src, double t);

        // Points interface
        [DllImport ("usdi")] public static extern Points        usdiAsPoints(Schema schema);
        [DllImport ("usdi")] public static extern Points        usdiCreatePoints(Context ctx, Schema parent, string name);
        [DllImport ("usdi")] public static extern void          usdiPointsGetSummary(Points points, ref PointsSummary dst);
        [DllImport ("usdi")] public static extern Bool          usdiPointsReadSample(Points points, ref PointsData dst, double t);
        [DllImport ("usdi")] public static extern Bool          usdiPointsWriteSample(Points points, ref PointsData src, double t);

        // Attribute interface
        [DllImport ("usdi")] public static extern IntPtr        usdiAttrGetName(Attribute attr);
        [DllImport ("usdi")] public static extern IntPtr        usdiAttrGetTypeName(Attribute attr);
        [DllImport ("usdi")] public static extern AttributeType usdiAttrGetType(Attribute attr);
        [DllImport ("usdi")] public static extern int           usdiAttrGetArraySize(Attribute attr, double t);
        [DllImport ("usdi")] public static extern int           usdiAttrGetNumSamples(Attribute attr);
        [DllImport ("usdi")] public static extern IntPtr        usdiAttrReadSample(Attribute attr, IntPtr dst, double t);
        [DllImport ("usdi")] public static extern IntPtr        usdiAttrReadArraySample(Attribute attr, IntPtr dst, int size, double t);
        [DllImport ("usdi")] public static extern IntPtr        usdiAttrWriteSample(Attribute attr, IntPtr src, double t);
        [DllImport ("usdi")] public static extern IntPtr        usdiAttrWriteArraySample(Attribute attr, IntPtr src, int size, double t);


        public static string S(IntPtr cstring) { return Marshal.PtrToStringAnsi(cstring); }
        public static IntPtr GetArrayPtr(Array v) { return v == null ? IntPtr.Zero : Marshal.UnsafeAddrOfPinnedArrayElement(v, 0); }

        public static void InitializePlugin()
        {
            usdi.AddDLLSearchPath();
            usdi.SetEnv("PXR_PLUGINPATH_NAME", Application.streamingAssetsPath + "/UTJ/USDForUnity/plugins");
        }
    }
}