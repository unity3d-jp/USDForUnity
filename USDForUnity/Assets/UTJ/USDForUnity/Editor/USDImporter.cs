#if UNITY_5_7_OR_NEWER || ENABLE_SCRIPTED_IMPORTERS

using System;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;
using UTJ;

namespace UTJ.USD
{
	public enum UsdImportMode
	{
		StripUSD,
		RetainUSD
	}


	[ScriptedImporter(4, new[] {"usd", "usda", "usdc"})]
	public class USDImporter : ScriptedImporter
	{
		[SerializeField] public UsdImportMode m_importMode = UsdImportMode.RetainUSD;
		[SerializeField] public bool m_SupportRuntimeUSD = true;
		[SerializeField] public usdi.ImportSettings m_importSettings = usdi.ImportSettings.default_value;
		[SerializeField] public TimeUnit m_timeUnit = new TimeUnit();
		[SerializeField] public double m_time;

		[SerializeField] public bool m_forceSingleThread = false;
		[SerializeField] public bool m_directVBUpdate = true;
		[SerializeField] public bool m_deferredUpdate;

		private SortedDictionary<int, UnityEngine.Object> _subObjects;

		public override void OnImportAsset(ImportAssetEventArgs args)
		{
			var fileName = System.IO.Path.GetFileNameWithoutExtension(args.AssetSourcePath);

			var go = new GameObject(fileName);
			var usdStream = go.AddComponent<UsdStream>();
			usdStream.importSettings = new usdi.ImportSettings()
			{
				interpolation = m_importSettings.interpolation,
				normalCalculation = m_importSettings.normalCalculation,
				tangentCalculation = m_importSettings.tangentCalculation,
				maxBoneWeights = m_importSettings.maxBoneWeights,
				scale = m_importSettings.scale,
				loadAllPayloads = true,
				triangulate = true,
				swapHandedness = m_importSettings.swapHandedness,
				swapFaces = m_importSettings.swapFaces,
				splitMesh = true,
				doubleBuffering = true,
			};
			usdStream.directVBUpdate = m_directVBUpdate;
			usdStream.forceSingleThread = m_forceSingleThread;
			usdStream.timeUnit = m_timeUnit;
			usdStream.playTime = m_time;

			usdStream.LoadImmediate(args.AssetSourcePath);

			var material = new Material(Shader.Find("Standard")) {};
			material.name = "Material_0";
			args.AddSubAsset("Default Material", material);
			_subObjects = new SortedDictionary<int, UnityEngine.Object>();
			CollectSubAssets(go.transform, args, material);

			int i = 0;
			foreach (var m in _subObjects)
			{
				if (String.IsNullOrEmpty(m.Value.name) || m.Value.name.IndexOf("<dyn>") == 0)
					m.Value.name = fileName + "_" + m.Value.GetType().Name + "_" + (++i);
				args.AddSubAsset(m.Value.name, m.Value);
			}

			if (m_importMode == UsdImportMode.StripUSD)
				usdStream.usdiDetachUsdComponents();

			args.SetMainAsset(fileName, go);
		}

		private void RegisterSubAsset(UnityEngine.Object subAsset)
		{
			if (!_subObjects.ContainsKey(subAsset.GetInstanceID()))
			{
				_subObjects.Add(subAsset.GetInstanceID(), subAsset);
			}
		}

		private void CollectSubAssets(Transform node, ImportAssetEventArgs args, Material mat)
		{
			var schemas = node.GetComponents<UsdXformComponent>();
			foreach (var s in schemas)
			{
				s.schema.overrideImportSettings = true;
				s.schema.importSettings = m_importSettings;
			}

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
				CollectSubAssets(node.GetChild(i), args, mat);
		}
	}
}

#endif
