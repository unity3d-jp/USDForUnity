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
            var path = MakeRelativePath(EditorUtility.OpenFilePanel("Select USD or Alembic file (.usd, .usda, .usdc, .abc)",
                Application.streamingAssetsPath, ""));
            if (path == null || path == "") { return; }
            usdiImportWindow.Open(path);
        }

        public static string MakeRelativePath(string path)
        {
            if (path == "") { return path; }
            Uri pathToAssets = new Uri(Application.streamingAssetsPath + "/");
            return pathToAssets.MakeRelativeUri(new Uri(path)).ToString();
        }
    }

}
#endif // UNITY_EDITOR
