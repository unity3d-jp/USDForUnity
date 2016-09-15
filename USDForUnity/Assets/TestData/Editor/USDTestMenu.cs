using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using UTJ;

public class USDTestMenu
{
    [MenuItem("Assets/Test/Benchmark USD")]
    public static void ImportUSD()
    {
        var path = UTJ.usdiMenu.MakeRelativePath(EditorUtility.OpenFilePanel("Select USD or Alembic file (.usd, .usda, .usdc, .abc)",
            Application.streamingAssetsPath, ""));
        if (path == null || path == "") { return; }

        var opt = new usdiImportOptions();
        Vector3 pos = Vector3.zero;
        Vector3 forward = Vector3.forward;
        InstanciateUSD(path, opt, pos, forward);
    }

    static usdiStream InstanciateUSD(string path, usdiImportOptions opt, Vector3 pos, Vector3 forward)
    {
        var usd = UTJ.usdiImportWindow.InstanciateUSD(path, opt);
        var trans = usd.GetComponent<Transform>();
        if (trans)
        {
            trans.position = pos;
            trans.forward = forward;
        }
        return usd;
    }

}
