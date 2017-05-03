using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ.USD
{
    [ExecuteInEditMode]
    public class UsdStream : MonoBehaviour, ISerializationCallbackReceiver
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
        [SerializeField] usdi.ImportSettings m_importSettings = new usdi.ImportSettings();
        [SerializeField] TimeUnit m_timeUnit = new TimeUnit();
        [SerializeField] double m_time;

        [Header("Debug")]
#if UNITY_EDITOR
        [SerializeField] bool m_forceSingleThread = false;
        [SerializeField] bool m_detailedLog = false;
        bool m_isCompiling = false;
#endif
        [SerializeField] bool m_directVBUpdate = true;
        [SerializeField] bool m_deferredUpdate = false;

        [HideInInspector][SerializeField] string[] m_variantSelections_keys;
        [HideInInspector][SerializeField] VariantSelection[] m_variantSelections_values;
        [HideInInspector][SerializeField] string[] m_perObjectSettings_keys;
        [HideInInspector][SerializeField] usdi.ImportSettings[] m_perObjectSettings_values;
        Dictionary<string, VariantSelection> m_variantSelections = new Dictionary<string, VariantSelection>();
        Dictionary<string, usdi.ImportSettings> m_perObjectSettings = new Dictionary<string, usdi.ImportSettings>();

        List<UsdSchema> m_schemas = new List<UsdSchema>();
        Dictionary<string, UsdSchema> m_schemaLUT = new Dictionary<string, UsdSchema>();

        usdi.Context m_ctx;
        bool m_requestForceUpdate;
        bool m_requestReload;
        double m_prevUpdateTime = Double.NaN;
        usdi.Task m_asyncUpdate;
        #endregion


        #region properties
        public DataPath usdPath
        {
            get { return m_path; }
        }
        public usdi.Context usdiContext
        {
            get { return m_ctx; }
        }
        public usdi.ImportSettings importSettings
        {
            get { return m_importSettings; }
            set { m_importSettings = value; }
        }
        public double playTime
        {
            get { return m_time; }
            set { m_time = value; }
        }
        public TimeUnit timeUnit
        {
            get { return m_timeUnit; }
            set { m_timeUnit = value; }
        }
        public bool directVBUpdate
        {
            get { return m_directVBUpdate && usdi.usdiIsVtxCmdAvailable(); }
            set { m_directVBUpdate = value; }
        }
        public bool deferredUpdate
        {
            get { return m_deferredUpdate; }
            set { m_deferredUpdate = value; }
        }
#if UNITY_EDITOR
        public bool forceSingleThread
        {
            get { return m_forceSingleThread; }
            set { m_forceSingleThread = value; }
        }
#endif
        #endregion


        #region impl
        public void OnBeforeSerialize()
        {
            m_variantSelections_keys = m_variantSelections.Keys.ToArray();
            m_variantSelections_values = m_variantSelections.Values.ToArray();

            m_perObjectSettings_keys = m_perObjectSettings.Keys.ToArray();
            m_perObjectSettings_values = m_perObjectSettings.Values.ToArray();
        }
        public void OnAfterDeserialize()
        {
            // variant selections
            if (m_variantSelections_keys != null &&
                m_variantSelections_values != null &&
                m_variantSelections_keys.Length == m_variantSelections_values.Length)
            {
                int n = m_variantSelections_keys.Length;
                for (int i = 0; i < n; ++i)
                {
                    m_variantSelections[m_variantSelections_keys[i]] = m_variantSelections_values[i];
                }
            }
            m_variantSelections_keys = null;
            m_variantSelections_values = null;

            // per-object import settings
            if (m_perObjectSettings_keys != null &&
                m_perObjectSettings_values != null &&
                m_perObjectSettings_keys.Length == m_perObjectSettings_values.Length)
            {
                int n = m_perObjectSettings_keys.Length;
                for (int i = 0; i < n; ++i)
                {
                    m_perObjectSettings[m_perObjectSettings_keys[i]] = m_perObjectSettings_values[i];
                }
            }
            m_perObjectSettings_keys = null;
            m_perObjectSettings_values = null;
        }


        public void usdiSetVariantSelection(string primPath, int[] selection)
        {
            m_variantSelections[primPath] = new VariantSelection { selections = selection };
            usdiRequestReload();
        }

        public void usdiSetImportSettings(string primPath, ref usdi.ImportSettings settings)
        {
            m_perObjectSettings[primPath] = settings;
            usdiRequestForceUpdate();
        }
        public void usdiDeleteImportSettings(string primPath)
        {
            m_perObjectSettings.Remove(primPath);
            usdiRequestForceUpdate();
        }

        public void usdiRequestForceUpdate()
        {
            m_requestForceUpdate = true;
        }
        public void usdiRequestReload()
        {
            m_requestReload = true;
        }

        public UsdSchema usdiFindSchema(string primPath)
        {
            if(m_schemaLUT.ContainsKey(primPath))
            {
                return m_schemaLUT[primPath];
            }
            return null;
        }
        public UsdSchema usdiFindSchema(usdi.Schema s)
        {
            return usdiFindSchema(usdi.usdiPrimGetPathS(s));
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

        UsdSchema usdiCreateNode(usdi.Schema schema)
        {
            UsdSchema ret = null;
            if (ret == null)
            {
                var s = usdi.usdiAsPoints(schema);
                if (s) ret = new UsdPoints();
            }
            if (ret == null)
            {
                var s = usdi.usdiAsMesh(schema);
                if (s) ret = new UsdMesh();
            }
            if (ret == null)
            {
                var s = usdi.usdiAsCamera(schema);
                if (s) ret = new UsdCamera();
            }
            if (ret == null)
            {
                // Xform must be latter because some schemas are subclass of Xform
                var s = usdi.usdiAsXform(schema);
                if (s) ret = new UsdXform();
            }
            if (ret == null)
            {
                ret = new UsdSchema();
            }
            ret.nativeSchemaPtr = schema;
            ret.stream = this;
            return ret;
        }

        UsdSchema usdiFindOrCreateNode(Transform parent, usdi.Schema schema, ref bool created)
        {
            GameObject go = null;

            // find existing GameObject or create new one
            var name = usdi.usdiPrimGetNameS(schema);
            var child = parent.FindChild(name);
            if (child != null)
            {
                go = child.gameObject;
                created = false;
            }
            else if(go == null)
            {
                go = new GameObject();
                go.name = name;
                go.GetComponent<Transform>().SetParent(parent, false);
                created = true;
            }

            // create USD node
            UsdSchema ret = usdiCreateNode(schema);
            ret.gameObject = go;

            return ret;
        }

        void usdiConstructTree(Transform parent, usdi.Schema schema, Action<UsdSchema> node_handler)
        {
            if(!schema) { return; }

            bool created = false;
            var elem = usdiFindOrCreateNode(parent, schema, ref created);
            if (elem != null)
            {
                node_handler(elem);
            }

            var trans = elem == null ? parent : elem.GetComponent<Transform>();
            int num_children = usdi.usdiPrimGetNumChildren(schema);
            for(int ci = 0; ci < num_children; ++ci)
            {
                var child = usdi.usdiPrimGetChild(schema, ci);
                usdiConstructTree(trans, child, node_handler);
            }
        }

        void usdiConstructMasterTree(usdi.Schema schema, Action<UsdSchema> node_handler)
        {
            if (!schema) { return; }

            var elem = usdiCreateNode(schema);
            node_handler(elem);

            int num_children = usdi.usdiPrimGetNumChildren(schema);
            for (int ci = 0; ci < num_children; ++ci)
            {
                var child = usdi.usdiPrimGetChild(schema, ci);
                usdiConstructMasterTree(child, node_handler);
            }
        }

        void usdiConstructTrees()
        {
            List<GameObject> data = new List<GameObject>();
            foreach (var kvp in m_schemaLUT)
            {
                var e = kvp.Value;
                if (e.gameObject != null)
                {
                    var c = e.gameObject.GetComponent<UsdIComponent>();
                    if(c != null)
                    {
                        c.schema = null;
                        data.Add(e.gameObject);
                    }
                }
            }

            m_schemas = new List<UsdSchema>();
            m_schemaLUT = new Dictionary<string, UsdSchema>();

            // construct master tree
            {
                var nmasters = usdi.usdiGetNumMasters(m_ctx);
                for (int i = 0; i < nmasters; ++i)
                {
                    usdiConstructMasterTree(usdi.usdiGetMaster(m_ctx, i),
                        (e) =>
                        {
                            e.usdiOnLoad();
                            m_schemas.Add(e);
                            m_schemaLUT[e.primPath] = e;
                        });
                }
            }

            // construct non-master tree along with corresponding GameObject
            {
                var root = usdi.usdiGetRoot(m_ctx);
                var nchildren = usdi.usdiPrimGetNumChildren(root);
                for (int i = 0; i < nchildren; ++i)
                {
                    usdiConstructTree(GetComponent<Transform>(), usdi.usdiPrimGetChild(root, i),
                        (e) =>
                        {
                            e.usdiOnLoad();
                            m_schemas.Add(e);
                            m_schemaLUT[e.primPath] = e;
                        });
                }
            }

            // delete GameObjects that lost corresponding USD schema (e.g. variant set has changed)
            foreach (var go in data)
            {
                if(go != null)
                {
                    var c = go.GetComponent<UsdIComponent>();
                    if (c != null && c.schema == null)
                    {
#if UNITY_EDITOR
                        Undo.DestroyObjectImmediate(go);
#else
                        DestroyImmediate(go);
#endif
                    }
                }
            }
        }


        void usdiApplyImportConfig(bool all)
        {
            usdi.usdiSetImportSettings(m_ctx, ref m_importSettings);
            if(all)
            {
                foreach(var v in m_perObjectSettings)
                {
                    var s = usdi.usdiFindSchema(m_ctx, v.Key);
                    if(s)
                    {
                        var tmp = v.Value;
                        usdi.usdiPrimSetOverrideImportSettings(s, true);
                        usdi.usdiPrimSetImportSettings(s, ref tmp);
                    }
                }
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

        bool usdiLoad(string path)
        {
            return usdiLoad(new DataPath(path));
        }

        bool usdiLoad(DataPath path)
        {
            usdiUnload();

            m_path = path;
            m_path.readOnly = true;
            m_ctx = usdi.usdiCreateContext();

            var fullpath = m_path.GetFullPath();
            if (!usdi.usdiOpen(m_ctx, fullpath))
            {
                usdi.usdiDestroyContext(m_ctx);
                m_ctx = default(usdi.Context);
                usdiLog("UsdStream: failed to load " + fullpath);
                return false;
            }

            // apply variant selections
            if(usdiApplyVarianceSelections())
            {
                usdi.usdiRebuildSchemaTree(m_ctx);
            }
            usdiApplyImportConfig(true);

            usdiConstructTrees();

            // fill sample data with initial time
            m_requestForceUpdate = true;
            usdiAsyncUpdate(m_time);
            usdiUpdate(m_time);

            usdiLog("UsdStream: loaded " + fullpath);
            return true;
        }

        public void usdiReload()
        {
            if (!m_ctx) { return; }

            usdiWaitAsyncUpdateTask();

            usdiApplyVarianceSelections();
            usdiApplyImportConfig(true);
            usdi.usdiRebuildSchemaTree(m_ctx);

            usdiConstructTrees();

            // fill sample data with initial time
            m_requestForceUpdate = true;
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

            int c = m_schemas.Count;
            for (int i = 0; i < c; ++i) { m_schemas[i].usdiOnUnload(); }

            m_schemas.Clear();
            m_schemaLUT.Clear();

            usdi.usdiDestroyContext(m_ctx);
            m_ctx = default(usdi.Context);

            usdiLog("UsdStream: unloaded " + m_path.GetFullPath());
        }

        public bool usdiSave()
        {
            return usdi.usdiSave(m_ctx);
        }

        public bool usdiSaveAs(string path)
        {
            return usdi.usdiSaveAs(m_ctx, path);
        }

        public void usdiDetachUsdComponents()
        {
            Action<UnityEngine.Object> deleter = (UnityEngine.Object o) => {
#if UNITY_EDITOR
                Undo.DestroyObjectImmediate(o);
#else
                DestroyImmediate(o);
#endif
            };

            int c = m_schemas.Count;
            foreach (var s in m_schemas)
            {
                var go = s.gameObject;
                if (go != null)
                {
                    var component = go.GetComponent<UsdIComponent>();
                    if (component != null)
                    {
                        deleter(component);
                    }
                }
            }
            deleter(this);
        }

        // possibly called from non-main thread
        void usdiAsyncUpdate(double t)
        {
            usdiApplyImportConfig(false);

            // skip if update is not needed
            if(m_requestForceUpdate)
            {
                usdi.usdiNotifyForceUpdate(m_ctx);
            }
            else if (t == m_prevUpdateTime)
            {
                return;
            }

            usdi.usdiUpdateAllSamples(m_ctx, t);
            int c = m_schemas.Count;
            for (int i = 0; i < c; ++i)
            {
                m_schemas[i].usdiAsyncUpdate(t);
            }
        }

        void usdiUpdate(double t)
        {
            if (!m_requestForceUpdate && t == m_prevUpdateTime) { return; }
            m_requestForceUpdate = false;

            // update all elements
            int c = m_schemas.Count;
            for (int i = 0; i < c; ++i)
            {
                m_schemas[i].usdiUpdate(t);
            }
            usdi.TransformNotfyChange(GetComponent<Transform>());

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
                        }, "UsdStream: " + gameObject.name);
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


        public void Load(string path)
        {
            if(!m_ctx)
            {
                m_path = new DataPath(path);
            }
        }
        public void Load(DataPath path)
        {
            if (!m_ctx)
            {
                m_path = path;
            }
        }
        public bool LoadImmediate(string path)
        {
            return usdiLoad(path);
        }
        public bool LoadImmediate(DataPath path)
        {
            return usdiLoad(path);
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

#if UNITY_EDITOR
        void OnValidate()
        {
            usdiRequestForceUpdate();
        }
#endif
        public static bool GlobalLockout = false;

        void Update()
        {
            if (GlobalLockout)
                return;

#if UNITY_EDITOR
            if (EditorApplication.isCompiling && !m_isCompiling)
            {
                // on compile begin
                m_isCompiling = true;
                usdiUnload();
            }
            else if(!EditorApplication.isCompiling && m_isCompiling)
            {
                // on compile end
                m_isCompiling = false;
                usdi.InitializePluginPass2();
                usdiLoad(m_path);
            }
#endif

            if(!m_ctx) { return; }

            if (m_requestReload)
            {
                usdiReload();
                m_requestReload = false;
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

            if (directVBUpdate)
            {
                GL.IssuePluginEvent(usdi.usdiGetRenderEventFunc(), 0);
            }

#if UNITY_EDITOR
            if (EditorApplication.isPlaying)
#endif
            {
                m_time += Time.deltaTime * m_timeUnit.scale;
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
