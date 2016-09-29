using System;
using System.Collections.Generic;
using System.Threading;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{
    [Serializable]
    public class usdiImportOptions
    {
        public usdi.InterpolationType interpolation = usdi.InterpolationType.Linear;
        public usdi.NormalCalculationType normalCalculation = usdi.NormalCalculationType.WhenMissing;
        public float scale = 1.0f;
        public bool swapHandedness = true;
        public bool swapFaces = true;
    }


    [ExecuteInEditMode]
    public class usdiStream : MonoBehaviour
    {
        static List<usdiStream> s_instances = new List<usdiStream>();

        #region fields 
        [SerializeField] string m_path;
        [SerializeField] usdiImportOptions m_importOptions = new usdiImportOptions();
        [SerializeField] double m_time;
        [SerializeField] double m_timeScale = 1.0;

        [Header("Debug")]
#if UNITY_EDITOR
        [SerializeField] bool m_forceSingleThread = false;
        [SerializeField] bool m_detailedLog = false;
        bool m_isCompiling = false;
#endif
        [SerializeField] bool m_directVBUpdate = true;
        [SerializeField] bool m_deferredUpdate = true;

        usdi.Context m_ctx;
        List<usdiElement> m_elements = new List<usdiElement>();
        double m_prevUpdateTime = Double.NaN;
        usdi.usdiTaskFunc m_taskFunc;
        int m_taskHandle = 0;
        #endregion


        #region properties
        public string usdPath { get { return m_path; } }
        public usdiImportOptions usdImportOptions
        {
            get { return m_importOptions; }
            set { m_importOptions = value; }
        }
        public double usdTime
        {
            get { return m_time; }
            set { m_time = value; }
        }
        public double usdTimeScale
        {
            get { return m_timeScale; }
            set { m_timeScale = value; }
        }
        public bool directVBUpdate { get { return m_directVBUpdate; } }
        public bool deferredUpdate { get { return m_deferredUpdate; } }
#if UNITY_EDITOR
        public bool usdForceSingleThread { get { return m_forceSingleThread; } }
#endif
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

        static usdiElement usdiCreateNode(Transform parent, usdi.Schema schema)
        {
            {
                var name = usdi.S(usdi.usdiGetName(schema));
                var child = parent.FindChild(name);
                if (child != null)
                {
                    return child.GetComponent<usdiElement>();
                }
            }

            GameObject go = null;
            usdiElement elem = null;

            if (go == null)
            {
                var points = usdi.usdiAsPoints(schema);
                if (points)
                {
                    go = new GameObject();
                    elem = go.AddComponent<usdiPoints>();
                }
            }
            if (go == null)
            {
                var mesh = usdi.usdiAsMesh(schema);
                if(mesh)
                {
                    go = new GameObject();
                    elem = go.AddComponent<usdiMesh>();
                }
            }
            if (go == null)
            {
                var cam = usdi.usdiAsCamera(schema);
                if (cam)
                {
                    go = new GameObject();
                    elem = go.AddComponent<usdiCamera>();
                }
            }
            if (go == null)
            {
                var xf = usdi.usdiAsXform(schema);
                if (xf)
                {
                    go = new GameObject();
                    elem = go.AddComponent<usdiXform>();
                }
            }

            if(go != null)
            {
                go.GetComponent<Transform>().SetParent(parent);
                go.name = usdi.S(usdi.usdiGetName(schema));
            }

            return elem;
        }

        void usdiCreateNodeRecursive(Transform parent, usdi.Schema schema, Action<usdiElement> node_handler)
        {
            if(!schema) { return; }

            var elem = usdiCreateNode(parent, schema);
            if (elem != null )
            {
                elem.stream = this;
                elem.usdiOnLoad(schema);
                if (node_handler != null) { node_handler(elem); }
            }

            var trans = elem == null ? parent : elem.GetComponent<Transform>();
            int num_children = usdi.usdiGetNumChildren(schema);
            for(int ci = 0; ci < num_children; ++ci)
            {
                var child = usdi.usdiGetChild(schema, ci);
                usdiCreateNodeRecursive(trans, child, node_handler);
            }
        }

        void usdiApplyImportConfig()
        {
            usdi.ImportConfig conf;
            conf.interpolation = m_importOptions.interpolation;
            conf.normal_calculation = m_importOptions.normalCalculation;
            conf.scale = m_importOptions.scale;
            conf.triangulate = true;
            conf.swap_handedness = m_importOptions.swapHandedness;
            conf.swap_faces = m_importOptions.swapFaces;
            conf.split_mesh = true;
            conf.double_buffering = m_deferredUpdate;
            usdi.usdiSetImportConfig(m_ctx, ref conf);
        }

        public bool usdiLoad(string path)
        {
            usdiUnload();

            m_path = path;
            m_ctx = usdi.usdiCreateContext();
            usdiApplyImportConfig();
            if (!usdi.usdiOpen(m_ctx, Application.streamingAssetsPath + "/" + m_path))
            {
                usdi.usdiDestroyContext(m_ctx);
                m_ctx = default(usdi.Context);
                usdiLog("usdiStream: failed to load " + m_path);
                return false;
            }

            usdiCreateNodeRecursive(GetComponent<Transform>(), usdi.usdiGetRoot(m_ctx),
                (e) => { m_elements.Add(e); });

            usdiAsyncUpdate(m_time);
            usdiUpdate(m_time);

            usdiLog("usdiStream: loaded " + m_path);
            return true;
        }

        public void usdiUnload()
        {
            if(m_ctx)
            {
                usdiWaitAsyncUpdateTask();
                usdi.usdiWaitAsyncRead();
                usdi.usdiExtClearTaskQueue(0);
                usdi.usdiExtClearTaskQueue(1);

                int c = m_elements.Count;
                for (int i = 0; i < c; ++i)
                {
                    m_elements[i].usdiOnUnload();
                }

                usdi.usdiDestroyContext(m_ctx);
                m_ctx = default(usdi.Context);

                usdiLog("usdiStream: unloaded " + m_path);
            }
        }

        void usdiAsyncUpdate(double t)
        {
            // skip if update is not needed
            if (t == m_prevUpdateTime) { return; }

            usdiApplyImportConfig();
            usdi.usdiUpdateAllSamples(m_ctx, t);

            // update all elements
            int c = m_elements.Count;
            for (int i = 0; i < c; ++i)
            {
                m_elements[i].usdiAsyncUpdate(t);
            }
        }

        void usdiUpdate(double t)
        {
            if (t == m_prevUpdateTime) { return; }

            // update all elements
            int c = m_elements.Count;
            for(int i=0; i<c; ++i)
            {
                m_elements[i].usdiUpdate(t);
            }

            m_prevUpdateTime = t;
        }


        void usdiKickAsyncUpdateTask()
        {
            // kick async update tasks
#if UNITY_EDITOR
            if (m_forceSingleThread)
            {
                usdiAsyncUpdate(m_time);
            }
            else
#endif
            {
                if(m_taskFunc == null) { m_taskFunc = usdiAsyncTask; }
                m_taskHandle = usdi.usdiExtTaskRun(m_taskFunc, IntPtr.Zero);
            }
        }

        void usdiAsyncTask(IntPtr arg)
        {
            try
            {
                usdiAsyncUpdate(m_time);
            }
            finally { }
        }

        void usdiWaitAsyncUpdateTask()
        {
            usdi.usdiExtTaskWait(m_taskHandle);
            m_taskHandle = 0;
        }

        #endregion


        #region callbacks
        void Awake()
        {
            usdi.InitializePlugin();
        }

        void Start()
        {
            usdiLoad(m_path);
        }

        void OnEnable()
        {
            s_instances.Add(this);
        }

        void OnDisable()
        {
#if UNITY_EDITOR
            if (!EditorApplication.isPlaying && EditorApplication.isPlayingOrWillChangePlaymode)
            {
                usdiUnload();
            }
#endif
            s_instances.Remove(this);
        }

        void OnDestroy()
        {
            usdiUnload();
        }

        void OnApplicationQuit()
        {
            usdiUnload();
        }


        static int s_nth_Update;
        static int s_nth_LateUpdate;

        void Update()
        {
            ++s_nth_Update;
            s_nth_LateUpdate = 0;
#if UNITY_EDITOR
            if (EditorApplication.isCompiling && !m_isCompiling)
            {
                m_isCompiling = true;
                usdiUnload();
            }
            else if(!EditorApplication.isCompiling && m_isCompiling)
            {
                m_isCompiling = false;
                usdiLoad(m_path);
            }
#endif

            if (!m_deferredUpdate
#if UNITY_EDITOR
                || !EditorApplication.isPlaying
#endif
                )
            {
                usdiKickAsyncUpdateTask();
            }
        }

        void LateUpdate()
        {
            ++s_nth_LateUpdate;
            s_nth_Update = 0;

            usdiWaitAsyncUpdateTask();
            if (s_nth_LateUpdate == 1)
            {
                usdi.usdiWaitAsyncRead();
                if(m_directVBUpdate)
                {
                    for(int i=0; i<s_instances.Count; ++i)
                    {
                        s_instances[i].usdiWaitAsyncUpdateTask();
                    }
                    GL.IssuePluginEvent(usdi.usdiGetRenderEventFunc(), usdi.usdiExtIncrementTaskIndex());
                }
            }

            usdiUpdate(m_time);

#if UNITY_EDITOR
            if (EditorApplication.isPlaying)
#endif
            {
                m_time += Time.deltaTime * m_timeScale;
            }

            if (m_deferredUpdate
#if UNITY_EDITOR
                && EditorApplication.isPlaying
#endif
                )
            {
                usdiKickAsyncUpdateTask();
            }
        }
        #endregion
    }

}
