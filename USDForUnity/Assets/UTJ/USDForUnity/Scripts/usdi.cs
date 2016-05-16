using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Reflection;
using UnityEngine;

namespace UTJ
{
    public static class usdi
    {
        public struct Context
        {
            public System.IntPtr ptr;
            public static implicit operator bool(Context v) { return v.ptr != IntPtr.Zero; }
        }

        public struct Schema
        {
            public System.IntPtr ptr;
            public static implicit operator bool(Schema v) { return v.ptr != IntPtr.Zero; }
        }

        public struct Xform
        {
            public System.IntPtr ptr;
            public static implicit operator bool(Xform v) { return v.ptr != IntPtr.Zero; }
        }

        public struct Camera
        {
            public System.IntPtr ptr;
            public static implicit operator bool(Camera v) { return v.ptr != IntPtr.Zero; }
        }

        public struct Mesh
        {
            public System.IntPtr ptr;
            public static implicit operator bool(Mesh v) { return v.ptr != IntPtr.Zero; }
        }

        public struct Points
        {
            public System.IntPtr ptr;
            public static implicit operator bool(Points v) { return v.ptr != IntPtr.Zero; }
        }


        public enum SchemaType
        {
            Unknown,
            Xform,
            Camera,
            Mesh,
            Points,
        }

        public enum TopologyVariance
        {
            Constant, // both vertices and topologies are constant
            Homogenous, // vertices are not constant (= animated). topologies are constant.
            Heterogenous, // both vertices and topologies are not constant
        };

        public struct Time
        {
            double time;
        };

        public struct ImportConfig
        {
            Bool triangulate;
            Bool swap_handedness;
            Bool swap_faces;
        };

        public struct ExportConfig
        {
            Bool ascii;
            Bool swap_handedness;
            Bool swap_faces;
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
            public Bool has_velocities;
        };

        public struct MeshData
        {
            public IntPtr  points;
            public IntPtr  velocities;
            public IntPtr  normals;
            public IntPtr  counts;
            public IntPtr  indices;
            public IntPtr  indices_triangulated;

            public uint    num_points;
            public uint    num_counts;
            public uint    num_indices;
            public uint    num_indices_triangulated;
        }

        public struct PointsSummary
        {
            public uint peak_num_points;
            public TopologyVariance topology_variance;
            public Bool has_velocities;
        };

        public struct PointsData
        {
            public IntPtr points;
            public IntPtr velocities;

            public uint num_points;
        }


        [DllImport ("usdi")] public static extern Context       usdiOpen(string path);
        [DllImport ("usdi")] public static extern Context       usdiCreateContext();
        [DllImport ("usdi")] public static extern void          usdiDestroyContext(Context ctx);
        [DllImport ("usdi")] public static extern Bool          usdiWrite(Context ctx, string path);

        [DllImport ("usdi")] public static extern void          usdiSetImportConfig(Context ctx, ref ImportConfig conf);
        [DllImport ("usdi")] public static extern void          usdiGetImportConfig(Context ctx, ref ImportConfig conf);
        [DllImport ("usdi")] public static extern void          usdiSetExportConfig(Context ctx, ref ExportConfig conf);
        [DllImport ("usdi")] public static extern void          usdiGetExportConfig(Context ctx, ref ExportConfig conf);
        [DllImport ("usdi")] public static extern Schema        usdiGetRoot(Context ctx);

        [DllImport ("usdi")] public static extern int           usdiGetID(Schema schema);
        [DllImport ("usdi")] public static extern IntPtr        usdiGetPath(Schema schema);
        [DllImport ("usdi")] public static extern IntPtr        usdiGetName(Schema schema);
        [DllImport ("usdi")] public static extern SchemaType    usdiGetType(Schema schema);
        [DllImport ("usdi")] public static extern Schema        usdiGetParent(Schema schema);
        [DllImport ("usdi")] public static extern int           usdiGetNumChildren(Schema schema);
        [DllImport ("usdi")] public static extern Schema        usdiGetChild(Schema schema, int i);

        [DllImport ("usdi")] public static extern Xform         usdiAsXform(Schema schema);
        [DllImport ("usdi")] public static extern Xform         usdiCreateXform(Schema parent, string name);
        [DllImport ("usdi")] public static extern Bool          usdiXformReadSample(Xform xf, ref XformData dst, double t);
        [DllImport ("usdi")] public static extern Bool          usdiXformWriteSample(Xform xf, ref XformData src, double t);

        [DllImport ("usdi")] public static extern Camera        usdiAsCamera(Schema schema);
        [DllImport ("usdi")] public static extern Camera        usdiCreateCamera(Schema parent, string name);
        [DllImport ("usdi")] public static extern Bool          usdiCameraReadSample(Camera cam, ref CameraData dst, double t);
        [DllImport ("usdi")] public static extern Bool          usdiCameraWriteSample(Camera cam, ref CameraData src, double t);

        [DllImport ("usdi")] public static extern Mesh          usdiAsMesh(Schema schema);
        [DllImport ("usdi")] public static extern Mesh          usdiCreateMesh(Schema parent, string name);
        [DllImport ("usdi")] public static extern void          usdiMeshGetSummary(Mesh mesh, ref MeshSummary dst);
        [DllImport ("usdi")] public static extern Bool          usdiMeshReadSample(Mesh mesh, ref MeshData dst, double t);
        [DllImport ("usdi")] public static extern Bool          usdiMeshWriteSample(Mesh mesh, ref MeshData src, double t);
        
        [DllImport ("usdi")] public static extern Points        usdiAsPoints(Schema schema);
        [DllImport ("usdi")] public static extern Points        usdiCreatePoints(Schema parent, string name);
        [DllImport ("usdi")] public static extern void          usdiPointsGetSummary(Points points, ref PointsSummary dst);
        [DllImport ("usdi")] public static extern Bool          usdiPointsReadSample(Points points, ref PointsData dst, double t);
        [DllImport ("usdi")] public static extern Bool          usdiPointsWriteSample(Points points, ref PointsData src, double t);

    }
}