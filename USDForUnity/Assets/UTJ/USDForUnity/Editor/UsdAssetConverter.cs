using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    public class UsdAssetConverter
    {
        Transform m_root;
        string m_assetName;

        AnimationClip m_animClip;

        public UsdAssetConverter(Transform root, string assetName)
        {
            m_root = root;
            m_assetName = assetName;

        }

        public bool Convert()
        {
            m_animClip = new AnimationClip();
            ConvertRecursive(m_root, "");
            AssetDatabase.CreateAsset(m_animClip, "Assets/" + m_assetName + ".anim");
            return true;
        }

        void ConvertRecursive(Transform t, string path)
        {
            path = (string)path.Clone();
            if(path.Length > 0)
            {
                path += "/";
            }
            path += t.name;

            var cmp = t.GetComponent<UsdIComponent>();
            if(cmp != null)
            {
                var schema = cmp.schema.nativeSchemaPtr;
                ConvertXform(usdi.usdiAsXform(schema), path);
                ConvertCamera(usdi.usdiAsCamera(schema), path);
                ConvertMesh(usdi.usdiAsMesh(schema), path);
            }

            int nchildren = t.childCount;
            for (int i = 0; i < nchildren; ++i)
            {
                ConvertRecursive(t.GetChild(i), path);
            }
        }

        void ConvertXform(usdi.Xform xf, string name)
        {
            if (!xf) { return; }

            var tx = new AnimationCurve();
            var ty = new AnimationCurve();
            var tz = new AnimationCurve();
            var rx = new AnimationCurve();
            var ry = new AnimationCurve();
            var rz = new AnimationCurve();
            var sx = new AnimationCurve();
            var sy = new AnimationCurve();
            var sz = new AnimationCurve();

            usdi.usdiXformEachSample(xf, (ref usdi.XformData data, double t_)=> {
                float t = (float)t_;
                if (data.flags.updatedPosition)
                {
                    tx.AddKey(t, data.position.x);
                    ty.AddKey(t, data.position.y);
                    tz.AddKey(t, data.position.z);
                }
                if (data.flags.updatedRotation)
                {
                    var euler = data.rotation.eulerAngles;
                    rx.AddKey(t, euler.x);
                    ry.AddKey(t, euler.y);
                    rz.AddKey(t, euler.z);
                }
                if (data.flags.updatedScale)
                {
                    sx.AddKey(t, data.scale.x);
                    sy.AddKey(t, data.scale.y);
                    sz.AddKey(t, data.scale.z);
                }
            });

            var ttransform = typeof(Transform);
            if (tx.length > 0) m_animClip.SetCurve(name, ttransform, "localPosition.x", tx);
            if (ty.length > 0) m_animClip.SetCurve(name, ttransform, "localPosition.y", ty);
            if (tz.length > 0) m_animClip.SetCurve(name, ttransform, "localPosition.z", tz);
            if (rx.length > 0) m_animClip.SetCurve(name, ttransform, "localEulerAngles.x", rx);
            if (ry.length > 0) m_animClip.SetCurve(name, ttransform, "localEulerAngles.y", ry);
            if (rz.length > 0) m_animClip.SetCurve(name, ttransform, "localEulerAngles.z", rz);
            if (sx.length > 0) m_animClip.SetCurve(name, ttransform, "localScale.x", sx);
            if (sy.length > 0) m_animClip.SetCurve(name, ttransform, "localScale.y", sy);
            if (sz.length > 0) m_animClip.SetCurve(name, ttransform, "localScale.z", sz);
        }

        void ConvertCamera(usdi.Camera cam, string name)
        {
            if (!cam) { return; }
        }

        void ConvertMesh(usdi.Mesh mesh, string name)
        {
            if (!mesh) { return; }
        }
    }
}