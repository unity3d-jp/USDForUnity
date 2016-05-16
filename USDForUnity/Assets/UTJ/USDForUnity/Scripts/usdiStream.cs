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

        usdi.Context m_usdi;
        List<usdiElement> m_elements = new List<usdiElement>();


        public static usdiElement usdiCreateNode(Transform parent, usdi.Schema schema)
        {
            GameObject go = null;
            usdiElement elem = null;

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
                elem.usdiInitialize(schema);
            }

            return elem;
        }

        public static void usdiCreateNodeRecursive(Transform parent, usdi.Schema schema, Action<usdiElement> node_handler)
        {
            if(!schema) { return; }

            var elem = usdiCreateNode(parent, schema);
            if(elem != null && node_handler != null) { node_handler(elem); }

            var trans = elem == null ? null : elem.GetComponent<Transform>();
            int num_children = usdi.usdiGetNumChildren(schema);
            for(int ci = 0; ci < num_children; ++ci)
            {
                var child = usdi.usdiGetChild(schema, ci);
                usdiCreateNodeRecursive(trans, child, node_handler);
            }
        }


        void Start()
        {
            m_usdi = usdi.usdiOpen(m_path);
            if(!m_usdi)
            {
                Debug.Log("failed to load " + m_path);
            }
            else
            {
                usdiCreateNodeRecursive(GetComponent<Transform>(), usdi.usdiGetRoot(m_usdi),
                    (e) =>
                    {
                        m_elements.Add(e);
                    });
            }
        }

        void Update()
        {
            foreach(var e in m_elements)
            {
                e.usdiUpdate(m_time);
            }

            m_time += Time.deltaTime;
        }
    }

}
