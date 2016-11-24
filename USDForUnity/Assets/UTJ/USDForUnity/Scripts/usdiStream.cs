using System;
using System.Collections.Generic;
using System.Linq;
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
    public class usdiStream : MonoBehaviour, ISerializationCallbackReceiver
    {
        #region types
        // just for serialize int[][] (Unity doesn't serialize array of arrays)
        [Serializable]
        public class VariantSelection
        {
            public int[] selections;
        }
        #endregion


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

        Dictionary<string, VariantSelection> m_variantSelections = new Dictionary<string, VariantSelection>();
        [SerializeField] string[] m_variantSelectionsKeys; // just for serialization
        [SerializeField] VariantSelection[] m_variantSelectionsValues; // just for serialization


        usdi.Context m_ctx;
        List<usdiElement> m_elements = new List<usdiElement>();
        usdi.ImportConfig m_prevConfig;
        bool m_updateRequired;
        bool m_reloadRequired;
        double m_prevUpdateTime = Double.NaN;
        usdi.Task m_asyncUpdate;
        #endregion


        #region properties
        public DataPath usdPath { get { return m_path; } }
        public usdi.Context usdiContext { get { return m_ctx; } }
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
        public void OnBeforeSerialize()
        {
            // serialize m_variantSelections
            m_variantSelectionsKeys = m_variantSelections.Keys.ToArray();
            m_variantSelectionsValues = m_variantSelections.Values.ToArray();
        }
        public void OnAfterDeserialize()
        {
            // deserialize m_variantSelections
            if (m_variantSelectionsKeys != null && m_variantSelectionsValues != null)
            {
                int n = m_variantSelectionsKeys.Length;
                for (int i = 0; i < n; ++i)
                {
                    m_variantSelections[m_variantSelectionsKeys[i]] = m_variantSelectionsValues[i];
                }
            }
            m_variantSelectionsKeys = null;
            m_variantSelectionsValues = null;
        }

        public void usdiSetVariantSelection(string primPath, int[] selection)
        {
            m_variantSelections[primPath] = new VariantSelection { selections = selection };
            m_reloadRequired = true;
        }


        public void usdiNotifyUpdateNeeded()
        {
            m_updateRequired = true;
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

        static usdiElement usdiFindOrCreateNode(Transform parent, usdi.Schema schema, ref bool created)
        {
            var name = usdi.usdiPrimGetNameS(schema);
            var child = parent.FindChild(name);
            if (child != null)
            {
                var c = child.GetComponent<usdiElement>();
                if(c != null)
                {
                    created = false;
                    return c;
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
                go.name = name;
                created = true;
            }

            return elem;
        }

        void usdiConstructNodeRecursive(Transform parent, usdi.Schema schema,
            Action<usdiElement, usdi.Schema, bool> node_handler)
        {
            if(!schema) { return; }

            bool created = false;
            var elem = usdiFindOrCreateNode(parent, schema, ref created);
            if (elem != null)
            {
                elem.stream = this;
                node_handler(elem, schema, created);
            }

            var trans = elem == null ? parent : elem.GetComponent<Transform>();
            int num_children = usdi.usdiPrimGetNumChildren(schema);
            for(int ci = 0; ci < num_children; ++ci)
            {
                var child = usdi.usdiPrimGetChild(schema, ci);
                usdiConstructNodeRecursive(trans, child, node_handler);
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

        bool usdiApplyVarianceSelections()
        {
            bool applied = false;
            foreach(var kvp in m_variantSelections)
            {
                var p = usdi.usdiFindSchema(m_ctx, kvp.Key);
                var selections = kvp.Value.selections;
                int nvals = selections.Length;
                for (int s = 0; s < nvals; ++s)
                {
                    if(usdi.usdiPrimSetVariantSelection(p, s, selections[s]))
                    {
                        applied = true;
                    }
                }
            }
            return applied;
        }

        public bool usdiLoad(string path)
        {
            return usdiLoad(new DataPath(path));
        }

        public bool usdiLoad(DataPath path)
        {
            usdiUnload();

            m_path = path;
            m_path.readOnly = true;
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

            if(usdiApplyVarianceSelections())
            {
                usdi.usdiRebuildSchemaTree(m_ctx);
            }
            usdiConstructNodeRecursive(GetComponent<Transform>(), usdi.usdiGetRoot(m_ctx),
                (e, schema, created) =>
                {
                    e.usdiOnLoad(schema);
                    m_elements.Add(e);
                });

            usdiAsyncUpdate(m_time);
            usdiUpdate(m_time);

            usdiLog("usdiStream: loaded " + fullpath);
            return true;
        }

        void usdiReload()
        {
            if (!m_ctx) { return; }

            usdiWaitAsyncUpdateTask();

            {
                int c = m_elements.Count;
                for (int i = 0; i < c; ++i)
                {
                    m_elements[i].usdiOnUnload();
                }
            }

            usdiApplyImportConfig();
            usdiApplyVarianceSelections();
            usdi.usdiRebuildSchemaTree(m_ctx);

            // reconstruct schema tree
            usdiConstructNodeRecursive(GetComponent<Transform>(), usdi.usdiGetRoot(m_ctx),
                (e, schema, created) =>
                {
                    if(created)
                    {
                        e.usdiOnLoad(schema);
                        m_elements.Add(e);
                    }
                    else
                    {
                        e.usdiOnReload();
                    }
                });
            {
                // delete elements that doesn't exist in USD tree
                int c = m_elements.Count;
                for (int i = 0; i < c; ++i)
                {
                    if (!m_elements[i])
                    {
                        m_elements[i] = null;
                    }
                    else if (!m_elements[i].schema)
                    {
                        m_elements[i].usdiDestroy();
                        m_elements[i] = null;
                    }
                }
                m_elements.RemoveAll(e => e == null);
            }

            usdiAsyncUpdate(m_time);
            usdiUpdate(m_time);

            var fullpath = m_path.GetFullPath();
            usdiLog("usdiStream: reloaded " + fullpath);
        }

        public void usdiUnload()
        {
            if(!m_ctx) { return; }

            usdiWaitAsyncUpdateTask();
            m_asyncUpdate = null;

            int c = m_elements.Count;
            for (int i = 0; i < c; ++i) { m_elements[i].usdiOnUnload(); }

            usdi.usdiDestroyContext(m_ctx);
            m_ctx = default(usdi.Context);

            usdiLog("usdiStream: unloaded " + m_path.GetFullPath());
        }

        // possibly called from non-main thread
        void usdiAsyncUpdate(double t)
        {
            usdiApplyImportConfig();

            // skip if update is not needed
            if (!m_updateRequired && t == m_prevUpdateTime) { return; }

            usdi.usdiUpdateAllSamples(m_ctx, t);
            int c = m_elements.Count;
            for (int i = 0; i < c; ++i)
            {
                m_elements[i].usdiAsyncUpdate(t);
            }
        }

        void usdiUpdate(double t)
        {
            if (!m_updateRequired && t == m_prevUpdateTime) { return; }
            m_updateRequired = false;

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

            if(!m_ctx) { return; }

            if (m_reloadRequired)
            {
                usdiReload();
                m_reloadRequired = false;
            }

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
            if (!m_ctx) { return; }

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
