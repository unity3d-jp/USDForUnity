#if UNITY_EDITOR
using System;
using UnityEngine;
using UnityEditor;

namespace UTJ
{
    public class usdiMenu
    {
        [MenuItem("Assets/UTJ/USD/Import USD")]
        public static void ImportUSD()
        {
            var path = MakeRelativePath(EditorUtility.OpenFilePanel("Select usd file (.usd, .usda, etc) in StreamingAssets directory",
                Application.streamingAssetsPath, ""));
            if (path == null || path == "") { return; }
            usdiImportWindow.Open(path);
        }

        static string MakeRelativePath(string path)
        {
            Uri pathToAssets = new Uri(Application.streamingAssetsPath + "/");
            return pathToAssets.MakeRelativeUri(new Uri(path)).ToString();
        }
    }

}
#endif // UNITY_EDITOR
