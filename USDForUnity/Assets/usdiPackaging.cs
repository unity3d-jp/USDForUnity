#if UNITY_EDITOR
using UnityEngine;
using UnityEditor;


public class usdiPackaging
{
    [MenuItem("Assets/Make USDForUnity.unitypackage")]
    public static void MakePackage()
    {
        string[] files = new string[]
        {
"Assets/UTJ/USDForUnity",
"Assets/UTJ/Misc",
"Assets/UTJ/Plugins",
"Assets/StreamingAssets/UTJ/USDForUnity",
        };
        AssetDatabase.ExportPackage(files, "USDForUnity.unitypackage", ExportPackageOptions.Recurse);
    }

}
#endif // UNITY_EDITOR
