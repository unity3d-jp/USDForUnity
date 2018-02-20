#if UNITY_2017_1_OR_NEWER

using System.Collections.Generic;
using UnityEditor;
using UnityEditor.Experimental;
using UnityEditor.Experimental.AssetImporters;
using UnityEngine;
using UTJ;

namespace UTJ.USD
{
    public enum UsdImportMode
    {
        StripUSD,
        RetainUSD
    }

    [ScriptedImporter(4, new[] {"usd", "usda", "usdc"}, 100)]
    public class USDImporter : ScriptedImporter
    {
        [SerializeField] public UsdImportMode m_importMode = UsdImportMode.RetainUSD;
        [SerializeField] public bool m_supportRuntimeUSD = true;
        [SerializeField] public usdi.ImportSettings m_importSettings = usdi.ImportSettings.defaultValue;
        [SerializeField] public TimeUnit m_timeUnit = new TimeUnit();
        [SerializeField] public double m_time;

        [SerializeField] public bool m_forceSingleThread = false;
        [SerializeField] public bool m_deferredUpdate;

        private SortedDictionary<int, Object> m_subObjects;

        public override void OnImportAsset(AssetImportContext ctx)
        {
            var fileName = System.IO.Path.GetFileNameWithoutExtension( ctx.assetPath);

            var go = new GameObject(fileName);
            var usdStream = go.AddComponent<UsdStream>();
            usdStream.importSettings = new usdi.ImportSettings()
            {
                interpolation = m_importSettings.interpolation,
                normalCalculation = m_importSettings.normalCalculation,
                tangentCalculation = m_importSettings.tangentCalculation,
                splitUnit = m_importSettings.splitUnit,
                maxBoneWeights = m_importSettings.maxBoneWeights,
                scaleFactor = m_importSettings.scaleFactor,
                loadAllPayloads = true,
                triangulate = true,
                swapHandedness = m_importSettings.swapHandedness,
                swapFaces = m_importSettings.swapFaces,
            };
            usdStream.forceSingleThread = m_forceSingleThread;
            usdStream.timeUnit = m_timeUnit;
            usdStream.playTime = m_time;

            usdStream.LoadImmediate(ctx.assetPath);

            var material = new Material(Shader.Find("Standard"));
            material.name = "Default Material";
            AddSubAsset(ctx, "Default Material", material);
            m_subObjects = new SortedDictionary<int, UnityEngine.Object>();
            CollectSubAssets(go.transform, material);

            int i = 0;
            foreach (var m in m_subObjects)
            {
                if (System.String.IsNullOrEmpty(m.Value.name) || m.Value.name.IndexOf("<dyn>") == 0)
                    m.Value.name = fileName + "_" + m.Value.GetType().Name + "_" + (++i);
                AddSubAsset(ctx, m.Value.name, m.Value);
            }

            if (m_importMode == UsdImportMode.StripUSD)
                usdStream.UsdDetachUsdComponents();

#if UNITY_2017_3_OR_NEWER
            ctx.AddObjectToAsset(fileName, go);
            ctx.SetMainObject(go);
#else
            ctx.SetMainAsset(fileName, go);
#endif
        }
        public void AddSubAsset(AssetImportContext ctx, string identifier, Object asset)
        {
#if UNITY_2017_3_OR_NEWER
            ctx.AddObjectToAsset(identifier, asset);
#else
            ctx.AddSubAsset(identifier, asset);
#endif
        }


        private void RegisterSubAsset(UnityEngine.Object subAsset)
        {
            if (!m_subObjects.ContainsKey(subAsset.GetInstanceID()))
            {
                m_subObjects.Add(subAsset.GetInstanceID(), subAsset);
            }
        }

        private void CollectSubAssets(Transform node, Material mat)
        {
            var meshFilter = node.GetComponent<MeshFilter>();
            if (meshFilter != null)
                RegisterSubAsset(meshFilter.sharedMesh);

            var renderer = node.GetComponent<MeshRenderer>();
            if (renderer != null)
                renderer.material = mat;

            var skinned = node.GetComponent<SkinnedMeshRenderer>();
            if (skinned != null && skinned.sharedMesh != null)
            {
                RegisterSubAsset(skinned.sharedMesh);
                skinned.material = mat;
            }

            for (int i = 0; i < node.childCount; i++)
                CollectSubAssets(node.GetChild(i), mat);
        }
    }
}

#endif
