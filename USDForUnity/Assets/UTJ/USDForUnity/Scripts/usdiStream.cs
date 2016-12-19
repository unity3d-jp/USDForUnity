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
    public class SerializableDictionary<K, V> : Dictionary<K, V>, ISerializationCallbackReceiver
    {
        [SerializeField] K[] m_keys;
        [SerializeField] V[] m_values;

        public void OnBeforeSerialize()
        {
            m_keys = Keys.ToArray();
            m_values = Values.ToArray();
        }
        public void OnAfterDeserialize()
        {
            if (m_keys != null && m_values != null && m_keys.Length == m_values.Length)
            {
                int n = m_keys.Length;
                for (int i = 0; i < n; ++i)
                {
                    this[m_keys[i]] = m_values[i];
                }
            }
            m_keys = null;
            m_values = null;
        }
    }


    [ExecuteInEditMode]
    public class usdiStream : MonoBehaviour
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

        [SerializeField] SerializableDictionary<string, VariantSelection> m_variantSelections = new SerializableDictionary<string, VariantSelection>();
        List<usdiSchema> m_schemas = new List<usdiSchema>();
        Dictionary<string, usdiSchema> m_schemaLUT = new Dictionary<string, usdiSchema>();

        usdi.Context m_ctx;
        bool m_updateRequired;
        bool m_updateElementsListRequired;
        bool m_reloadRequired;
        double m_prevUpdateTime = Double.NaN;
        usdi.Task m_asyncUpdate;
        bool m_recordUndo;
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
        public double timeScale
        {
            get { return m_timeScale; }
            set { m_timeScale = value; }
        }
        public bool directVBUpdate
        {
            get { return m_directVBUpdate && usdi.usdiIsVtxCmdAvailable(); }
            set { m_directVBUpdate = value; }
        }
        public bool deferredUpdate
        {
            get { return m_deferredUpdate; }
        }
        public bool recordUndo
        {
            get { return m_recordUndo; }
            set { m_recordUndo = value; }
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
        public void usdiSetVariantSelection(string primPath, int[] selection)
        {
            m_variantSelections[primPath] = new VariantSelection { selections = selection };
            usdiReload();
        }

        public void usdiNotifyForceUpdate()
        {
            m_updateRequired = true;
        }

        public void usdiNotifyUpdateElementsList()
        {
            m_updateElementsListRequired = true;
        }

        public usdiSchema usdiFindSchema(string primPath)
        {
            if(m_schemaLUT.ContainsKey(primPath))
            {
                return m_schemaLUT[primPath];
            }
            return null;
        }
        public usdiSchema usdiFindSchema(usdi.Schema s)
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

        usdiSchema usdiCreateNode(usdi.Schema schema)
        {
            usdiSchema ret = null;
            if (ret == null)
            {
                var s = usdi.usdiAsPoints(schema);
                if (s) ret = new usdiPoints();
            }
            if (ret == null)
            {
                var s = usdi.usdiAsMesh(schema);
                if (s) ret = new usdiMesh();
            }
            if (ret == null)
            {
                var s = usdi.usdiAsCamera(schema);
                if (s) ret = new usdiCamera();
            }
            if (ret == null)
            {
                // Xform must be latter because some schemas are subclass of Xform
                var s = usdi.usdiAsXform(schema);
                if (s) ret = new usdiXform();
            }
            if (ret == null)
            {
                ret = new usdiSchema();
            }
            ret.nativeSchemaPtr = schema;
            ret.stream = this;
            return ret;
        }

        usdiSchema usdiFindOrCreateNode(Transform parent, usdi.Schema schema, ref bool created)
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
                go.GetComponent<Transform>().SetParent(parent);
                created = true;
            }

            // create USD node
            usdiSchema ret = usdiCreateNode(schema);
            ret.gameObject = go;

            return ret;
        }

        void usdiConstructTree(Transform parent, usdi.Schema schema, Action<usdiSchema> node_handler)
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

        void usdiConstructMasterTree(usdi.Schema schema, Action<usdiSchema> node_handler)
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
                    var c = e.gameObject.GetComponent<usdiIElement>();
                    if(c != null)
                    {
                        c.schema = null;
                        data.Add(e.gameObject);
                    }
                }
            }

            m_schemas = new List<usdiSchema>();
            m_schemaLUT = new Dictionary<string, usdiSchema>();

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
                    var c = go.GetComponent<usdiIElement>();
                    if (c != null && c.schema == null)
                    {
                        DestroyImmediate(go);
                    }
                }
            }
        }

        void usdiUpdateElementsList()
        {
            // delete elements that doesn't exist in USD tree
            int c = m_schemas.Count;
            for (int i = 0; i < c; ++i)
            {
                if (!m_schemas[i].nativeSchemaPtr)
                {
                    m_schemaLUT.Remove(m_schemas[i].primPath);
                    m_schemas[i].usdiDestroy();
                    m_schemas[i] = null;
                }
            }
            m_schemas.RemoveAll(e => e == null);
        }


        void usdiApplyImportConfig()
        {
            usdi.usdiSetImportSettings(m_ctx, ref m_importSettings);
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
            usdiApplyImportConfig();

            var fullpath = m_path.GetFullPath();
            if (!usdi.usdiOpen(m_ctx, fullpath))
            {
                usdi.usdiDestroyContext(m_ctx);
                m_ctx = default(usdi.Context);
                usdiLog("usdiStream: failed to load " + fullpath);
                return false;
            }

            // apply variant selections
            if(usdiApplyVarianceSelections())
            {
                usdi.usdiRebuildSchemaTree(m_ctx);
            }

            usdiConstructTrees();

            // fill sample data with initial time
            usdiAsyncUpdate(m_time);
            usdiUpdate(m_time);

            usdiLog("usdiStream: loaded " + fullpath);
            return true;
        }

        public void usdiReload()
        {
            if (!m_ctx) { return; }

            usdiWaitAsyncUpdateTask();

            usdiApplyImportConfig();
            usdiApplyVarianceSelections();
            usdi.usdiRebuildSchemaTree(m_ctx);

            usdiConstructTrees();

            // fill sample data with initial time
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

            usdiLog("usdiStream: unloaded " + m_path.GetFullPath());
        }

        public void usdiDetach()
        {
            int c = m_schemas.Count;
#if UNITY_EDITOR
            if(recordUndo)
            {
                recordUndo = false;
                Undo.DestroyObjectImmediate(this);
            }
            else
#endif
            {
                DestroyImmediate(this);
            }
        }

        public void usdiMakePrefab()
        {
            // todo
            Debug.Log("not implemented yet");
        }

        // possibly called from non-main thread
        void usdiAsyncUpdate(double t)
        {
            usdiApplyImportConfig();

            // skip if update is not needed
            if(m_updateRequired)
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
            if (!m_updateRequired && t == m_prevUpdateTime) { return; }
            m_updateRequired = false;

            // update all elements
            int c = m_schemas.Count;
            for (int i = 0; i < c; ++i)
            {
                m_schemas[i].usdiUpdate(t);
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
            usdiNotifyForceUpdate();
        }
#endif


        void Update()
        {
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

            if (m_reloadRequired)
            {
                usdiReload();
                m_reloadRequired = false;
                m_updateElementsListRequired = false;
            }
            if (m_updateElementsListRequired)
            {
                usdiUpdateElementsList();
                m_updateElementsListRequired = false;
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

            usdi.TransformNotfyChange(GetComponent<Transform>());

            if (directVBUpdate)
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
