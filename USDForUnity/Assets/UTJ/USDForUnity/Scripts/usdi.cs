using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Reflection;
using UnityEngine;

namespace UTJ
{
    static class usdi
    {
        public struct ImportContext
        {
            public System.IntPtr ptr;
            public static implicit operator bool(ImportContext v) { return v.ptr != IntPtr.Zero; }
        }

        public struct ExportContext
        {
            public System.IntPtr ptr;
            public static implicit operator bool(ExportContext v) { return v.ptr != IntPtr.Zero; }
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

        public enum SchemaType
        {
            Unknown,
            Xform,
            Camera,
            Mesh,
        }

        public struct XformData
        {
            public Vector3     position;
            public Quaternion  rotation;
            public Vector3     scale;
        }

        public struct CameraData
        {
        }

        public struct MeshData
        {
            public IntPtr  points;
            public IntPtr  normals;
            public IntPtr  face_vertex_counts;
            public IntPtr  face_vertex_indices;

            public uint    num_points;
            public uint    num_face_vertex_counts;
            public uint    num_face_vertex_indices;
        }


        [DllImport ("usdi")] public static extern ImportContext usdiCreateImportContext(string path);
        [DllImport ("usdi")] public static extern void          usdiDestroyImportContext(ImportContext ctx);
        [DllImport ("usdi")] public static extern Schema        usdiGetRoot(ImportContext ctx);

        [DllImport ("usdi")] public static extern IntPtr        usdiGetPath(Schema schema);
        [DllImport ("usdi")] public static extern IntPtr        usdiGetName(Schema schema);
        [DllImport ("usdi")] public static extern SchemaType    usdiGetType(Schema schema);
        [DllImport ("usdi")] public static extern Schema        usdiGetParent(Schema schema);
        [DllImport ("usdi")] public static extern int           usdiGetNumChildren(Schema schema);
        [DllImport ("usdi")] public static extern Schema        usdiGetChild(Schema schema, int i);

        [DllImport ("usdi")] public static extern Xform         usdiAsXform(Schema schema);
        [DllImport ("usdi")] public static extern Xform         usdiCreateXform(Schema parent, string name);
        [DllImport ("usdi")] public static extern void          usdiXformReadSample(Xform xf, ref XformData dst, double t);
        [DllImport ("usdi")] public static extern void          usdiXformWriteSample(Xform xf, ref XformData src, double t);

        [DllImport ("usdi")] public static extern Camera        usdiAsCamera(Schema schema);
        [DllImport ("usdi")] public static extern Camera        usdiCreateCamera(Schema parent, string name);
        [DllImport ("usdi")] public static extern void          usdiCameraReadSample(Camera cam, ref CameraData dst, double t);
        [DllImport ("usdi")] public static extern void          usdiCameraWriteSample(Camera cam, ref CameraData src, double t);

        [DllImport ("usdi")] public static extern Mesh          usdiAsMesh(Schema schema);
        [DllImport ("usdi")] public static extern Mesh          usdiCreateMesh(Schema parent, string name);
        [DllImport ("usdi")] public static extern void          usdiMeshReadSample(Mesh mesh, ref MeshData dst, double t);
        [DllImport ("usdi")] public static extern void          usdiMeshWriteSample(Mesh mesh, ref MeshData src, double t);

    }
}