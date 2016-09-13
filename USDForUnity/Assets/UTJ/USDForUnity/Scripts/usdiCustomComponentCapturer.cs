using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif


namespace UTJ
{
    public abstract class usdiCustomComponentCapturer : MonoBehaviour
    {
        public abstract void CreateUSDObject(usdi.Context ctx, usdi.Schema parent);

        // capture data. called from main thread.
        public abstract void Capture(double t);

        // write data to USD. called from worker thread.
        // you can write data in Capture(), but do it in Flush() is better for performance.
        public abstract void Flush(double t);
    }
}
