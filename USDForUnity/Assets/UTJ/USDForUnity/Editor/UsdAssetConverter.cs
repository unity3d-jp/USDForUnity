using System;
using System.Linq;
using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    public class UsdAssetConverterWindow : EditorWindow
    {
        UsdAssetConverter m_converter;


        public static void Open(Transform target)
        {
            UsdAssetConverterWindow window = (UsdAssetConverterWindow)EditorWindow.GetWindow(typeof(UsdAssetConverterWindow));
            window.titleContent = new GUIContent("Convert Settings");
            window.Initialize(target);
            window.Show();
        }

        public void Initialize(Transform target)
        {
            m_converter = new UsdAssetConverter(target);
            m_converter.assetName = target.name;
        }

        void OnGUI()
        {
            m_converter.assetName = EditorGUILayout.TextField("Asset Name", m_converter.assetName);
            m_converter.keyframeReduction = EditorGUILayout.Toggle("Keyframe Reduction", m_converter.keyframeReduction);
            if(m_converter.keyframeReduction)
            {
                m_converter.keyframeEpsilon = EditorGUILayout.FloatField("  Keyframe Epsilon", m_converter.keyframeEpsilon);
            }
            EditorGUILayout.Space();

            GUILayout.Space(10.0f);

            if (GUILayout.Button("Convert"))
            {
                m_converter.Convert();
                Close();
            }
        }

    }

    public class UsdAssetConverter
    {
        #region fields
        string m_assetName = "UsdAsset";
        bool m_keyframeReduction = true;
        float m_keyframeEpsilon = 0.0001f;

        usdi.ProgressReporter m_reporter;
        Transform m_root;
        AnimationClip m_animClip;
        #endregion


        #region properties
        public string assetName
        {
            get { return m_assetName; }
            set { m_assetName = value; }
        }
        public bool keyframeReduction
        {
            get { return m_keyframeReduction; }
            set { m_keyframeReduction = value; }
        }
        public float keyframeEpsilon
        {
            get { return m_keyframeEpsilon; }
            set { m_keyframeEpsilon = value; }
        }
        #endregion


        #region impl
        public UsdAssetConverter(Transform root)
        {
            m_reporter = new usdi.ProgressReporter();
            m_root = root;
        }

        public bool Convert()
        {
            m_reporter.Open();
            m_animClip = new AnimationClip();
            ConvertRecursive(m_root, "");
            AssetDatabase.CreateAsset(m_animClip, "Assets/" + m_assetName + ".anim");
            m_reporter.Close();
            return true;
        }

        class CurveData
        {
            public AnimationCurve curve;
            public Type type;
            public string path;
            public string field;
            public int length { get { return curve.length; } }

            public CurveData(Type t, string p, string f)
            {
                curve = new AnimationCurve();
                type = t;
                path = p;
                field = f;
            }

            public void Set(AnimationClip clip)
            {
                if (curve.length == 0) return;
                if(curve.length == 2)
                {
                    var k1 = curve[0];
                    var k2 = curve[1];
                    if (k1.value == k2.value &&
                        k1.outTangent == 0.0f && k2.inTangent == 0.0f)
                    {
                        return;
                    }
                }

                clip.SetCurve(path, type, field, curve);
            }
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
                m_reporter.Write("converting " + cmp.schema.primName + " ...\n");
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

        void ConvertXform(usdi.Xform xf, string path)
        {
            if (!xf) { return; }

            var ttrans = typeof(Transform);
            var cvs = new CurveData[]{
                new CurveData(ttrans, path, "localPosition.x"),
                new CurveData(ttrans, path, "localPosition.y"),
                new CurveData(ttrans, path, "localPosition.z"),
                new CurveData(ttrans, path, "localEulerAngles.x"),
                new CurveData(ttrans, path, "localEulerAngles.y"),
                new CurveData(ttrans, path, "localEulerAngles.z"),
                new CurveData(ttrans, path, "localScale.x"),
                new CurveData(ttrans, path, "localScale.y"),
                new CurveData(ttrans, path, "localScale.z"),
            };

            usdi.usdiXformEachSample(xf, (ref usdi.XformData data, double t_)=> {
                float t = (float)t_;
                if (data.flags.updatedPosition)
                {
                    cvs[0].curve.AddKey(t, data.position.x);
                    cvs[1].curve.AddKey(t, data.position.y);
                    cvs[2].curve.AddKey(t, data.position.z);
                }
                if (data.flags.updatedRotation)
                {
                    var euler = data.rotation.eulerAngles;
                    cvs[3].curve.AddKey(t, euler.x);
                    cvs[4].curve.AddKey(t, euler.y);
                    cvs[5].curve.AddKey(t, euler.z);
                }
                if (data.flags.updatedScale)
                {
                    cvs[6].curve.AddKey(t, data.scale.x);
                    cvs[7].curve.AddKey(t, data.scale.y);
                    cvs[8].curve.AddKey(t, data.scale.z);
                }
            });

            if(m_keyframeReduction)
            {
                foreach(var c in cvs)
                {
                    DoReduction(c);
                }
            }
            foreach (var c in cvs)
            {
                c.Set(m_animClip);
            }
        }

        void ConvertCamera(usdi.Camera cam, string name)
        {
            if (!cam) { return; }
        }

        void ConvertMesh(usdi.Mesh mesh, string name)
        {
            if (!mesh) { return; }
        }

        void DoReduction(CurveData cv)
        {
            int before = cv.length;
            AnimationCurveKeyReducer.DoReduction(cv.curve, m_keyframeEpsilon);
            int after = cv.length;
            m_reporter.Write("  key reduction: " + cv.field + " " + before + " -> " + after + "\n");

        }
        #endregion
    }

    // thanks: http://techblog.sega.jp/entry/2016/11/28/100000
    public class AnimationCurveKeyReducer
    {
        static public void DoReduction(AnimationCurve in_curve, float eps = 0.0001f)
        {
            if (in_curve.keys.Length <= 2) return;

            var del_indexes = GetDeleteKeyIndex(in_curve.keys, eps).ToArray();
            foreach (var del_idx in del_indexes.Reverse()) in_curve.RemoveKey(del_idx);
        }

        static IEnumerable<int> GetDeleteKeyIndex(Keyframe[] keys, float eps)
        {
            for (int s_idx = 0, i = 1; i < keys.Length - 1; i++)
            {
                if (IsInterpolationValue(keys[s_idx], keys[i + 1], keys[i], eps))
                {
                    yield return i;
                }
                else
                {
                    s_idx = i;
                }
            }
        }

        static bool IsInterpolationValue(Keyframe key1, Keyframe key2, Keyframe comp, float eps)
        {
            var val1 = GetValueFromTime(key1, key2, comp.time);

            if (eps < System.Math.Abs(comp.value - val1)) return false;

            var time = key1.time + (comp.time - key1.time) * 0.5f;
            val1 = GetValueFromTime(key1, comp, time);
            var val2 = GetValueFromTime(key1, key2, time);

            return (System.Math.Abs(val2 - val1) <= eps) ? true : false;
        }

        static float GetValueFromTime(Keyframe key1, Keyframe key2, float time)
        {
            float t;
            float a, b, c;
            float kd, vd;

            if (key1.outTangent == Mathf.Infinity) return key1.value;

            kd = key2.time - key1.time;
            vd = key2.value - key1.value;
            t = (time - key1.time) / kd;

            a = -2 * vd + kd * (key1.outTangent + key2.inTangent);
            b = 3 * vd - kd * (2 * key1.outTangent + key2.inTangent);
            c = kd * key1.outTangent;

            return key1.value + t * (t * (a * t + b) + c);
        }
    }
}