#if UNITY_EDITOR
using UnityEngine;
using UnityEditor;


public class usdiPackaging
{
    [MenuItem("Assets/UTJ/USD/Make Packages")]
    public static void MakePackage()
    {
        string[] files = new string[]
        {
"Assets/UTJ/USDForUnity",
"Assets/UTJ/Plugins/x86_64/usdi.so",
"Assets/StreamingAssets/UTJ/USDForUnity",
        };
        AssetDatabase.ExportPackage(files, "USDForUnity.unitypackage", ExportPackageOptions.Recurse);
    }

}
#endif // UNITY_EDITOR
