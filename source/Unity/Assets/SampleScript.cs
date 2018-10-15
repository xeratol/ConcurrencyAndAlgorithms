using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SampleScript : MonoBehaviour
{
    public ComputeShader shader;

    RenderTexture _tex;
    int _kernelHandle;
    Renderer _renderer;

    private void Awake()
    {
        _tex = new RenderTexture(256, 256, 24);
        _tex.enableRandomWrite = true;
        _tex.Create();

        _kernelHandle = shader.FindKernel("CSMain");
        shader.SetTexture(_kernelHandle, "Result", _tex);

        _renderer = GetComponent<Renderer>();
    }

    void Update()
    {
        shader.SetFloat("time", Mathf.Repeat(Time.timeSinceLevelLoad, 1.0f));
        shader.Dispatch(_kernelHandle, 256 / 8, 256 / 8, 1);

        _renderer.material.SetTexture("_MainTex", _tex);
    }

    private void OnDestroy()
    {
    }
}
