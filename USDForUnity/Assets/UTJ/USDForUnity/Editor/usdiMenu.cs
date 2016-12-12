#if UNITY_EDITOR
using System;
using UnityEngine;
using UnityEditor;

namespace UTJ
{
    public class usdiMenu
    {
        [MenuItem("Assets/Import USD or Alembic")]
        public static void ImportUSD()
        {
            var path = EditorUtility.OpenFilePanel("Select USD or Alembic file (.usd, .usda, .usdc, .abc)", Application.streamingAssetsPath, "");
            if (path == null || path == "") { return; }
            usdiImportWindow.Open(path);
        }
    }

}
#endif // UNITY_EDITOR
