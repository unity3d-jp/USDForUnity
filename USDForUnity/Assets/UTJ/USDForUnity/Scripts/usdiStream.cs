using System;
using System.Collections.Generic;
using UnityEngine;

namespace UTJ
{

    public class usdiStream : MonoBehaviour
    {
        public string   m_path;
        public bool     m_swapHandedness = true;
        public double   m_time;

        usdi.Context m_ctx;
        List<usdiElement> m_elements = new List<usdiElement>();


        public static usdiElement usdiCreateNode(Transform parent, usdi.Schema schema)
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

        public static void usdiCreateNodeRecursive(Transform parent, usdi.Schema schema, Action<usdiElement> node_handler)
        {
            if(!schema) { return; }

            var elem = usdiCreateNode(parent, schema);
            if (elem != null )
            {
                elem.usdiInitialize(schema);
                if (node_handler != null) { node_handler(elem); }
            }

            var trans = elem == null ? null : elem.GetComponent<Transform>();
            int num_children = usdi.usdiGetNumChildren(schema);
            for(int ci = 0; ci < num_children; ++ci)
            {
                var child = usdi.usdiGetChild(schema, ci);
                usdiCreateNodeRecursive(trans, child, node_handler);
            }
        }

        public bool usdiLoad(string path)
        {
            usdiUnload();
            m_path = path;
            m_ctx = usdi.usdiOpen(Application.streamingAssetsPath + "/" + m_path);
            if (!m_ctx)
            {
                Debug.Log("failed to load USD: " + m_path);
                return false;
            }
            else
            {
                usdiCreateNodeRecursive(GetComponent<Transform>(), usdi.usdiGetRoot(m_ctx),
                    (e) =>
                    {
                        m_elements.Add(e);
                    });
                usdiUpdate(0.0);
                return true;
            }
        }

        public void usdiUnload()
        {
            usdi.usdiDestroyContext(m_ctx);
            m_ctx = default(usdi.Context);
        }

        public void usdiUpdate(double t)
        {
            foreach (var e in m_elements)
            {
                e.usdiUpdate(t);
            }
        }


        void Start()
        {
            usdiLoad(m_path);
        }

        void OnDestroy()
        {
            usdiUnload();
        }

        void Update()
        {
            usdiUpdate(m_time);
            m_time += Time.deltaTime;
        }
    }

}
