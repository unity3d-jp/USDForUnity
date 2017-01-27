#if UNITY_EDITOR
using System;
using UnityEngine;
using UnityEditor;

namespace UTJ
{
    public class UsdMenu
    {
        [MenuItem("Assets/Import USD or Alembic")]
        public static void ImportUSD()
        {
            var path = EditorUtility.OpenFilePanel("Select USD or Alembic file (.usd, .usda, .usdc, .abc)", Application.streamingAssetsPath, "");
            if (path == null || path == "") { return; }
            UsdImportWindow.Open(path);
        }
        [MenuItem("Assets/Convert USD to Alembic")]
        public static void ConvertUSDToAlembic()
        {
            var usd_path = EditorUtility.OpenFilePanel("Select USD file (.usd, .usda, .usdc)", Application.streamingAssetsPath, "");
            if (usd_path == null || usd_path == "") { return; }

            var abc_path = System.IO.Path.ChangeExtension(usd_path, ".abc");

            usdi.InitializePluginPass1();
            usdi.InitializePluginPass2();
            usdi.ConvertUSDToAlembic(usd_path, abc_path);
        }
    }

}
#endif // UNITY_EDITOR
