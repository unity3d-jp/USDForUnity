using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
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
        #region fields 
        [SerializeField] DataPath m_path;
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
        usdi.ImportConfig m_prevConfig;
        bool m_updateNeeded;
        double m_prevUpdateTime = Double.NaN;
        usdi.Task m_asyncUpdate;
        #endregion


        #region properties
        public DataPath usdPath { get { return m_path; } }
        public usdiImportOptions importOptions
        {
            get { return m_importOptions; }
            set { m_importOptions = value; }
        }
        public double playTime
        {
            get { return m_time; }
            set { m_time = value; }
        }
        public double timeScale
        {
            get { return m_timeScale; }
            set { m_timeScale = value; }
        }
        public bool directVBUpdate { get { return m_directVBUpdate; } }
        public bool deferredUpdate { get { return m_deferredUpdate; } }
#if UNITY_EDITOR
        public bool forceSingleThread {
            get { return m_forceSingleThread; }
            set { m_forceSingleThread = value; }
        }
#endif
        #endregion


        #region impl

        public void usdiNotifyUpdateNeeded()
        {
            m_updateNeeded = true;
        }

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
                var name = usdi.S(usdi.usdiPrimGetName(schema));
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
                // Xform must be last because some schemas are subclass of Xform
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
                go.name = usdi.S(usdi.usdiPrimGetName(schema));
            }

            return elem;
        }

        void usdiCreateNodeRecursive(Transform parent, usdi.Schema schema, Action<usdiElement, usdi.Schema> node_handler)
        {
            if(!schema) { return; }

            var elem = usdiCreateNode(parent, schema);
            if (elem != null )
            {
                elem.stream = this;
                elem.usdiOnLoad(schema);
                node_handler(elem, schema);
            }

            var trans = elem == null ? parent : elem.GetComponent<Transform>();
            int num_children = usdi.usdiPrimGetNumChildren(schema);
            for(int ci = 0; ci < num_children; ++ci)
            {
                var child = usdi.usdiPrimGetChild(schema, ci);
                usdiCreateNodeRecursive(trans, child, node_handler);
            }
        }



        void usdiApplyImportConfig()
        {
            usdi.ImportConfig conf;
            conf.interpolation = m_importOptions.interpolation;
            conf.normal_calculation = m_importOptions.normalCalculation;
            conf.scale = m_importOptions.scale;
            conf.load_all_payloads = true;
            conf.triangulate = true;
            conf.swap_handedness = m_importOptions.swapHandedness;
            conf.swap_faces = m_importOptions.swapFaces;
            conf.split_mesh = true;
            conf.double_buffering = m_deferredUpdate;

            if(!usdi.Equals(ref m_prevConfig, ref conf))
            {
                m_prevConfig = conf;
                usdi.usdiSetImportConfig(m_ctx, ref conf);
                usdiNotifyUpdateNeeded();
            }
        }

        public bool usdiLoad(string path)
        {
            return usdiLoad(new DataPath(path));
        }

        public bool usdiLoad(DataPath path)
        {
            usdiUnload();

            m_path = path;
            m_ctx = usdi.usdiCreateContext();
            usdiApplyImportConfig();

            var fullpath = m_path.GetFullPath();
            if (!usdi.usdiOpen(m_ctx, fullpath))
            {
                usdi.usdiDestroyContext(m_ctx);
                m_ctx = default(usdi.Context);
                usdiLog("usdiStream: failed to load " + fullpath);
                return false;
            }

            usdiCreateNodeRecursive(GetComponent<Transform>(), usdi.usdiGetRoot(m_ctx),
                (e, schema) =>
                {
                    m_elements.Add(e);
                });

            usdiAsyncUpdate(m_time);
            usdiUpdate(m_time);

            usdiLog("usdiStream: loaded " + fullpath);
            return true;
        }

        public void usdiUnload()
        {
            if(m_ctx)
            {
                usdiWaitAsyncUpdateTask();
                m_asyncUpdate = null;

                int c = m_elements.Count;
                for (int i = 0; i < c; ++i)
                {
                    m_elements[i].usdiOnUnload();
                }

                usdi.usdiDestroyContext(m_ctx);
                m_ctx = default(usdi.Context);

                usdiLog("usdiStream: unloaded " + m_path.GetFullPath());
            }
        }

        // possibly called from non-main thread
        void usdiAsyncUpdate(double t)
        {
            usdiApplyImportConfig();

            // skip if update is not needed
            if (!m_updateNeeded && t == m_prevUpdateTime) { return; }

            usdi.usdiUpdateAllSamples(m_ctx, t);
            int c = m_elements.Count;
            for (int i = 0; i < c; ++i)
            {
                m_elements[i].usdiAsyncUpdate(t);
            }
        }

        void usdiUpdate(double t)
        {
            if (!m_updateNeeded && t == m_prevUpdateTime) { return; }
            m_updateNeeded = false;

            // update all elements
            int c = m_elements.Count;
            for (int i = 0; i < c; ++i)
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
                if (m_asyncUpdate == null)
                {
                    m_asyncUpdate = new usdi.DelegateTask(
                        (arg) =>
                        {
                            try
                            {
                                usdiAsyncUpdate(m_time);
                            }
                            finally { }
                        }, "usdiStream: " + gameObject.name);
                }
                m_asyncUpdate.Run();
            }
        }

        void usdiWaitAsyncUpdateTask()
        {
            if(m_asyncUpdate != null)
            {
                m_asyncUpdate.Wait();
            }
        }

        #endregion


        #region callbacks
        void OnApplicationQuit()
        {
            usdi.FinalizePlugin();
        }

        void Awake()
        {
            usdi.InitializePluginPass1();
            usdi.InitializePluginPass2();
        }

        void OnDestroy()
        {
            usdiUnload();
        }

        void Start()
        {
            usdiLoad(m_path);
        }

        void OnEnable()
        {
        }

        void OnDisable()
        {
#if UNITY_EDITOR
            if (!EditorApplication.isPlaying && EditorApplication.isPlayingOrWillChangePlaymode)
            {
                usdiUnload();
            }
#endif
        }


        void Update()
        {
#if UNITY_EDITOR
            if (EditorApplication.isCompiling && !m_isCompiling)
            {
                m_isCompiling = true;
                usdiUnload();
            }
            else if(!EditorApplication.isCompiling && m_isCompiling)
            {
                m_isCompiling = false;
                usdi.usdiInitialize();
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
            usdiWaitAsyncUpdateTask();
            usdiUpdate(m_time);

            usdi.usdiUniTransformNotfyChange(GetComponent<Transform>());

            if (m_directVBUpdate)
            {
                GL.IssuePluginEvent(usdi.usdiGetRenderEventFunc(), 0);
            }

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
