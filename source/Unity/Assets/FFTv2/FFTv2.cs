﻿using System;
using System.Diagnostics;
using UnityEngine;

public class FFTv2 : MonoBehaviour
{
    public ComputeShader _shader;
    private FFTComputeHelper _helper;

    public Texture2D _source;
    private RenderTexture _intermediateForward;
    private RenderTexture _finalForward;
    private RenderTexture _intermediateInverse;
    private RenderTexture _finalInverse;

    public Renderer _sourceRenderer;
    public Renderer _intermediateForwardRenderer;
    public Renderer _finalForwardRenderer;
    public Renderer _intermediateInverseRenderer;
    public Renderer _finalInverseRenderer;

    #region Unity Methods
    private void Awake()
    {
        if (!IsInputValid())
        {
            return;
        }

        InitRenderTextures();
        InitRenderers();

        _helper = new FFTComputeHelper(_shader);
    }

    private void Start()
    {
        UnityEngine.Debug.Log(Stopwatch.IsHighResolution);
        
        _helper.Init(_source.width, _source.height);
        _helper.Load(_source);
        _helper.RecenterData();

        var sw = new Stopwatch();
        sw.Start();

        _helper.Forward(_intermediateForward);
        _helper.GetMagnitudeSpectrumScaled(_finalForward);

        sw.Stop();
        var numTicksSoFar = sw.ElapsedTicks;
        sw.Start();

        _helper.Inverse(_intermediateInverse);
        _helper.GetMagnitudeSpectrum(_finalInverse);

        sw.Stop();
        var totalTicks = sw.ElapsedTicks + numTicksSoFar;
        UnityEngine.Debug.LogFormat("Elapsed Time: {0}s", (float)totalTicks / TimeSpan.TicksPerSecond);
    }

    private void OnDestroy()
    {
        _helper.Release();
        DestroyRenderTextures();
    }
    #endregion

    #region Utility Methods
    static RenderTexture CreateRenderTexture(int width, int height)
    {
        RenderTexture tex = new RenderTexture(width, height, 24);
        tex.useMipMap = false;
        tex.wrapMode = TextureWrapMode.Clamp;
        tex.filterMode = FilterMode.Point;
        tex.enableRandomWrite = true;
        tex.Create();
        return tex;
    }

    private bool IsInputValid()
    {
        if (_shader == null)
        {
            return false;
        }

        if (FastFourierTransform.RoundUpPowerOf2((uint)_source.width) != (uint)_source.width ||
            FastFourierTransform.RoundUpPowerOf2((uint)_source.height) != (uint)_source.height)
        {
            return false;
        }

        if (_sourceRenderer == null ||
            _intermediateForwardRenderer == null || _finalForwardRenderer == null ||
            _intermediateInverseRenderer == null || _finalInverseRenderer == null)
        {
            return false;
        }

        return true;
    }
    #endregion

    #region Init Methods
    private void InitRenderers()
    {
        _sourceRenderer.material.mainTexture = _source;
        _intermediateForwardRenderer.material.mainTexture = _intermediateForward;
        _finalForwardRenderer.material.mainTexture = _finalForward;
        _intermediateInverseRenderer.material.mainTexture = _intermediateInverse;
        _finalInverseRenderer.material.mainTexture = _finalInverse;
    }

    private void InitRenderTextures()
    {
        _intermediateForward = CreateRenderTexture(_source.width, _source.height);
        _finalForward = CreateRenderTexture(_source.width, _source.height);
        _intermediateInverse = CreateRenderTexture(_source.width, _source.height);
        _finalInverse = CreateRenderTexture(_source.width, _source.height);
    }
    #endregion

    #region Clean Up Methods
    private void DestroyRenderTextures()
    {
        _intermediateForward.Release();
        _finalForward.Release();
        _intermediateInverse.Release();
        _finalInverse.Release();
    }
    #endregion
}
