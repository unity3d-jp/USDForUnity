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
        var path = EditorUtility.OpenFilePanel("Select USD or Alembic file (.usd, .usda, .usdc, .abc)", Application.streamingAssetsPath, "");
        if (path == null || path == "") { return; }

        var root = new GameObject("root").GetComponent<Transform>();
        var opt = new usdi.ImportSettings();

        int N1 = 14;
        int N2 = 22;
        int N3 = 32;
        double delay = 0.05;
        InstanciateUSDCircular(path, opt, root, 1.4f, N1, delay * (N1+N2), delay);
        InstanciateUSDCircular(path, opt, root, 2.2f, N2, delay * N1, delay);
        InstanciateUSDCircular(path, opt, root, 3.0f, N3, 0.0, delay);
        Selection.activeGameObject = root.gameObject;
    }

    static void InstanciateUSDCircular(string path, usdi.ImportSettings opt, Transform root, float radius, int N, double timeStart=0.0, double timeDelay=0.05)
    {
        float ang = 360.0f / N * Mathf.Deg2Rad;

        for (int i = 0; i < N; ++i)
        {
            float r = radius;
            Vector3 pos = new Vector3(r * Mathf.Cos(ang * i), 0.0f, r * Mathf.Sin(ang * i));
            Vector3 forward = -pos.normalized;
            var usd = InstanciateUSD(path, opt, pos, forward, timeStart + timeDelay * i).gameObject;
            usd.GetComponent<Transform>().SetParent(root, true);
        }

    }

    static usdiStream InstanciateUSD(string path, usdi.ImportSettings opt, Vector3 pos, Vector3 forward, double time)
    {
        var usd = UTJ.usdiImportWindow.InstanciateUSD(path, (stream) => {
            stream.importSettings = opt;
            stream.playTime = time;
        } );
        var trans = usd.GetComponent<Transform>();
        if (trans)
        {
            trans.position = pos;
            trans.forward = forward;
        }
        return usd;
    }

}
