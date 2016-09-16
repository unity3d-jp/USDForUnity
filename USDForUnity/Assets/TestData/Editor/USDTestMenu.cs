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

        const int N = 36;
        const double time = 0.05;
        const float radiusBase = 3.0f;
        const float radiusExpand = 0.0f;
        float ang = 360.0f / N * Mathf.Deg2Rad;

        var root = new GameObject("root").GetComponent<Transform>();

        var opt = new usdiImportOptions();
        for (int i=0; i<N; ++i)
        {
            float r = radiusBase + radiusExpand * i;
            Vector3 pos = new Vector3(r * Mathf.Cos(ang*i), 0.0f, r * Mathf.Sin(ang*i));
            Vector3 forward = -pos.normalized;
            var usd = InstanciateUSD(path, opt, pos, forward, time * i).gameObject;
            usd.GetComponent<Transform>().SetParent(root, true);
        }
        Selection.activeGameObject = root.gameObject;
    }

    static usdiStream InstanciateUSD(string path, usdiImportOptions opt, Vector3 pos, Vector3 forward, double time)
    {
        var usd = UTJ.usdiImportWindow.InstanciateUSD(path, opt, time);
        var trans = usd.GetComponent<Transform>();
        if (trans)
        {
            trans.position = pos;
            trans.forward = forward;
        }
        return usd;
    }

}
