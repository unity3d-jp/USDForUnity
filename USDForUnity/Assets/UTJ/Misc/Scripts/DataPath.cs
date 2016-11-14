using System;
using UnityEngine;


namespace UTJ
{
    [Serializable]
    public class DataPath
    {
        public enum Root
        {
            Absolute,
            CurrentDirectory,
            PersistentDataPath,
            StreamingAssetsPath,
            TemporaryCachePath,
            DataPath,
        }

        [SerializeField] Root m_root;
        [SerializeField] string m_leaf;

        public Root root {
            get { return m_root; }
            set { m_root = value; }
        }
        public string leaf {
            get { return m_leaf; }
            set { m_leaf = value; }
        }

        public DataPath() { }
        public DataPath(Root root, string leaf)
        {
            m_root = root;
            m_leaf = leaf;
        }

        public DataPath(string path)
        {
            if (path.Contains(Application.streamingAssetsPath))
            {
                m_root = Root.StreamingAssetsPath;
                m_leaf = path.Replace(Application.streamingAssetsPath, "");
            }
            else if (path.Contains(Application.dataPath))
            {
                m_root = Root.DataPath;
                m_leaf = path.Replace(Application.dataPath, "");
            }
            else if (path.Contains(Application.persistentDataPath))
            {
                m_root = Root.PersistentDataPath;
                m_leaf = path.Replace(Application.persistentDataPath, "");
            }
            else if (path.Contains(Application.temporaryCachePath))
            {
                m_root = Root.TemporaryCachePath;
                m_leaf = path.Replace(Application.temporaryCachePath, "");
            }
            else
            {
                m_root = Root.Absolute;
                m_leaf = path;
            }
        }

        public string GetFullPath()
        {
            if (m_root == Root.Absolute ||
                m_root == Root.CurrentDirectory)
            {
                return m_leaf;
            }

            string ret = "";
            switch (m_root)
            {
                case Root.PersistentDataPath:
                    ret = Application.persistentDataPath;
                    break;
                case Root.StreamingAssetsPath:
                    ret = Application.streamingAssetsPath;
                    break;
                case Root.TemporaryCachePath:
                    ret = Application.temporaryCachePath;
                    break;
                case Root.DataPath:
                    ret = Application.dataPath;
                    break;
            }

            if (!m_leaf.StartsWith("/"))
            {
                ret += "/";
            }
            ret += m_leaf;
            return ret;
        }

        public void CreateDirectory()
        {
            System.IO.Directory.CreateDirectory(GetFullPath());
        }
    }
}