using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SampleScript : MonoBehaviour
{
    public ComputeShader _shader;
    public Renderer _renderer; // display the texture

    RenderTexture _tex; // need the texture for writing
    int _kernelHandle;

    private void Awake()
    {
        // 128 x 128 texture using 24 bits (RGB)
        _tex = new RenderTexture(128, 128, 24);
        _tex.enableRandomWrite = true;
        _tex.Create(); // initialize to 0

        _kernelHandle = _shader.FindKernel("CSMain");
        _shader.SetTexture(_kernelHandle, "Result", _tex);
        _renderer.material.mainTexture = _tex;
    }

    void Update()
    {
        _shader.SetFloat("time", Mathf.Repeat(Time.timeSinceLevelLoad, 1.0f)); // [0, 1]
        // 128 / 8 = 16
        // 128 / 32 = 4
        _shader.Dispatch(_kernelHandle, 16, 16, 1);
    }
}
