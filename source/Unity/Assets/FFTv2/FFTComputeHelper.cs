﻿using System.Collections.Generic;
using UnityEngine;

public class FFTComputeHelper
{
    private ComputeShader _shader;
    const int GROUP_SIZE_X = 16;   // must match shader
    const int GROUP_SIZE_Y = 16;   // must match shader
    private int _width;
    private int _height;

    #region Kernel Handles
    private int _convertTexToComplexKernel;
    private int _convertComplexMagToTexKernel;
    private int _convertComplexMagToTexScaledKernel;
    //private int _convertComplexPhaseToTexKernel;
    private int _centerComplexKernel;
    private int _conjugateComplexKernel;
    private int _divideComplexByDimensionsKernel;
    private int _bitRevByRowKernel;
    private int _bitRevByColKernel;
    private int _butterflyByRowKernel;
    private int _butterflyByColKernel;
    #endregion

    #region Const Buffers
    private ComputeBuffer _bitRevRow;
    private ComputeBuffer _bitRevCol;
    private ComputeBuffer _twiddleRow;
    private ComputeBuffer _twiddleCol;
    #endregion

    #region Temp Buffers
    // these 2 are to be swapped back and forth
    private ComputeBuffer _bufferA;
    private ComputeBuffer _bufferB;
    #endregion

    [System.Diagnostics.DebuggerDisplay("{ToString()}")]
    public struct ComplexF
    {
        public float real;
        public float imag;

        public ComplexF(float r, float i = 0)
        {
            real = r;
            imag = i;
        }

        public static implicit operator ComplexF(int r)
        {
            return new ComplexF(r);
        }

        public override string ToString()
        {
            return string.Format("({0}, {1})", real, imag);
        }
    }

    public FFTComputeHelper(ComputeShader fftShader)
    {
        if (fftShader == null)
        {
            throw new System.Exception("Invalid Parameters");
        }

        _shader = fftShader;

        GetKernelHandles();
    }

    public void Init(int width, int height)
    {
        if (FastFourierTransform.RoundUpPowerOf2((uint)width) != (uint)width ||
            FastFourierTransform.RoundUpPowerOf2((uint)height) != (uint)height ||
            width < GROUP_SIZE_X || height < GROUP_SIZE_Y)
        {
            throw new System.Exception("Invalid Parameters");
        }

        _width = width;
        _height = height;

        _shader.SetInt("WIDTH", _width);
        _shader.SetInt("HEIGHT", _height);

        InitBitRevBuffers();
        InitTwiddleBuffers();
        InitTempBuffers();
    }

    public void Release()
    {
        ReleaseBitRevBuffers();
        ReleaseTwiddleBuffers();
        ReleaseTempBuffers();
    }

    public void Load(Texture2D source)
    {
        if (source == null || source.width != _width || source.height != _height)
        {
            throw new System.Exception("Invalid Parameters");
        }

        ConvertTexToComplex(source, _bufferA);

        // _bufferA contains data
    }

    public void Load(ComplexF [] data)
    {
        _bufferA.SetData(data);

        // _bufferA contains data
    }

    public void Save(ComplexF [] data)
    {
        _bufferA.GetData(data);
    }

    public void RecenterData()
    {
        // _bufferA should contain data
        CenterComplex(_bufferA, _bufferB);
        SwapBuffers(ref _bufferB, ref _bufferA);
        // _bufferA contains data
    }

    public void GetMagnitudeSpectrum(RenderTexture tex)
    {
        // _bufferA should contain data
        ConvertComplexMagToTex(_bufferA, tex);
    }

    public void GetMagnitudeSpectrumScaled(RenderTexture tex)
    {
        // _bufferA should contain data
        ConvertComplexMagToTexScaled(_bufferA, tex);
    }

    public void Forward(RenderTexture intermediate = null)
    {
        if (intermediate != null && (intermediate.width != _width || intermediate.height != _height))
        {
            throw new System.Exception("Invalid Parameters");
        }

        // _bufferA should contain data

        BitRevByRow(_bufferA, _bufferB);
        ButterflyByRow(ref _bufferB, ref _bufferA);

        if (intermediate != null)
        {
            ConvertComplexMagToTexScaled(_bufferA, intermediate);
        }

        BitRevByCol(_bufferA, _bufferB);
        ButterflyByCol(ref _bufferB, ref _bufferA);

        // _bufferA contains data
    }

    public void Inverse(RenderTexture intermediate = null)
    {
        if (intermediate != null && (intermediate.width != _width || intermediate.height != _height))
        {
            throw new System.Exception("Invalid Parameters");
        }

        // _bufferA should contain data

        ConjugateComplex(_bufferA, _bufferB);
        SwapBuffers(ref _bufferA, ref _bufferB);

        Forward(intermediate);

        ConjugateComplex(_bufferA, _bufferB);
        DivideComplexByDimensions(_bufferB, _bufferA);

        // _bufferA contains data
    }

    #region Init Methods
    private void GetKernelHandles()
    {
        _convertTexToComplexKernel = _shader.FindKernel("ConvertTexToComplex");
        _convertComplexMagToTexKernel = _shader.FindKernel("ConvertComplexMagToTex");
        _convertComplexMagToTexScaledKernel = _shader.FindKernel("ConvertComplexMagToTexScaled");
        //_convertComplexPhaseToTexKernel = _shader.FindKernel("ConvertComplexPhaseToTex");
        _centerComplexKernel = _shader.FindKernel("CenterComplex");
        _conjugateComplexKernel = _shader.FindKernel("ConjugateComplex");
        _divideComplexByDimensionsKernel = _shader.FindKernel("DivideComplexByDimensions");
        _bitRevByRowKernel = _shader.FindKernel("BitRevByRow");
        _bitRevByColKernel = _shader.FindKernel("BitRevByCol");
        _butterflyByRowKernel = _shader.FindKernel("ButterflyByRow");
        _butterflyByColKernel = _shader.FindKernel("ButterflyByCol");
    }

    private void InitBitRevBuffers()
    {
        _bitRevRow = CreateBitRevBuffer((uint)_width);
        _bitRevCol = CreateBitRevBuffer((uint)_height);
    }

    private void InitTwiddleBuffers()
    {
        _twiddleRow = CreateTwiddleBuffer((uint)_width / 2);
        _twiddleCol = CreateTwiddleBuffer((uint)_height / 2);
    }

    private void InitTempBuffers()
    {
        _bufferA = CreateComplexBuffer(_width, _height);
        _bufferB = CreateComplexBuffer(_width, _height);
    }
    #endregion

    #region Utility Methods
    private static ComputeBuffer CreateTwiddleBuffer(uint halfN)
    {
        var twiddleRaw = FastFourierTransform.GenTwiddleFactors(halfN);
        var twiddleArray = new ComplexF[halfN];
        for (var i = 0; i < halfN; ++i)
        {
            twiddleArray[i].real = (float)twiddleRaw[i].Real;
            twiddleArray[i].imag = (float)twiddleRaw[i].Imaginary;
        }
        ComputeBuffer twiddle = new ComputeBuffer((int)halfN, sizeof(float) * 2);
        twiddle.SetData(twiddleArray);
        return twiddle;
    }

    private static ComputeBuffer CreateBitRevBuffer(uint N)
    {
        ComputeBuffer bitRev = new ComputeBuffer((int)N, sizeof(uint));
        bitRev.SetData(FastFourierTransform.GenBitReversal(N).ToArray());
        return bitRev;
    }

    private static ComputeBuffer CreateComplexBuffer(int width, int height)
    {
        ComputeBuffer buffer = new ComputeBuffer(width * height, sizeof(float) * 2, ComputeBufferType.Default);
        //buffer.SetData(new SimpleComplex[width * height]);
        return buffer;
    }

    private static void SwapBuffers(ref ComputeBuffer a, ref ComputeBuffer b)
    {
        ComputeBuffer c = a;
        a = b;
        b = c;
    }
    #endregion

    #region Shader Methods
    private void Dispatch(int kernelHandle)
    {
        _shader.Dispatch(kernelHandle, _width / GROUP_SIZE_X, _height / GROUP_SIZE_Y, 1);
    }

    private void ConvertTexToComplex(Texture2D src, ComputeBuffer dst)
    {
        _shader.SetTexture(_convertTexToComplexKernel, "SrcTex", src);
        _shader.SetBuffer(_convertTexToComplexKernel, "Dst", dst);
        Dispatch(_convertTexToComplexKernel);
    }

    private void CenterComplex(ComputeBuffer src, ComputeBuffer dst)
    {
        _shader.SetBuffer(_centerComplexKernel, "Src", src);
        _shader.SetBuffer(_centerComplexKernel, "Dst", dst);
        Dispatch(_centerComplexKernel);
    }

    private void BitRevByRow(ComputeBuffer src, ComputeBuffer dst)
    {
        _shader.SetBuffer(_bitRevByRowKernel, "BitRevRow", _bitRevRow);
        _shader.SetBuffer(_bitRevByRowKernel, "Src", src);
        _shader.SetBuffer(_bitRevByRowKernel, "Dst", dst);
        Dispatch(_bitRevByRowKernel);
    }

    private void BitRevByCol(ComputeBuffer src, ComputeBuffer dst)
    {
        _shader.SetBuffer(_bitRevByColKernel, "BitRevCol", _bitRevCol);
        _shader.SetBuffer(_bitRevByColKernel, "Src", src);
        _shader.SetBuffer(_bitRevByColKernel, "Dst", dst);
        Dispatch(_bitRevByColKernel);
    }

    // Both src and dst will be modified
    private void ButterflyByRow(ref ComputeBuffer src, ref ComputeBuffer dst)
    {
        _shader.SetBuffer(_butterflyByRowKernel, "TwiddleRow", _twiddleRow);
        var swapped = false;
        for (int stride = 2; stride <= _width; stride *= 2)
        {
            _shader.SetInt("BUTTERFLY_STRIDE", stride);
            _shader.SetBuffer(_butterflyByRowKernel, "Src", swapped ? dst : src);
            _shader.SetBuffer(_butterflyByRowKernel, "Dst", swapped ? src : dst);
            Dispatch(_butterflyByRowKernel);
            swapped = !swapped;
        }

        if (!swapped)
        {
            SwapBuffers(ref src, ref dst);
        }
    }

    // Both src and dst will be modified
    private void ButterflyByCol(ref ComputeBuffer src, ref ComputeBuffer dst)
    {
        _shader.SetBuffer(_butterflyByColKernel, "TwiddleCol", _twiddleCol);
        var swapped = false;
        for (int stride = 2; stride <= _height; stride *= 2)
        {
            _shader.SetInt("BUTTERFLY_STRIDE", stride);
            _shader.SetBuffer(_butterflyByColKernel, "Src", swapped ? dst : src);
            _shader.SetBuffer(_butterflyByColKernel, "Dst", swapped ? src : dst);
            Dispatch(_butterflyByColKernel);
            swapped = !swapped;
        }

        if (!swapped)
        {
            SwapBuffers(ref src, ref dst);
        }
    }

    private void ConvertComplexMagToTex(ComputeBuffer src, RenderTexture dst)
    {
        _shader.SetBuffer(_convertComplexMagToTexKernel, "Src", src);
        _shader.SetTexture(_convertComplexMagToTexKernel, "DstTex", dst);
        Dispatch(_convertComplexMagToTexKernel);
    }

    private void ConvertComplexMagToTexScaled(ComputeBuffer src, RenderTexture dst)
    {
        _shader.SetBuffer(_convertComplexMagToTexScaledKernel, "Src", src);
        _shader.SetTexture(_convertComplexMagToTexScaledKernel, "DstTex", dst);
        Dispatch(_convertComplexMagToTexScaledKernel);
    }

    private void ConjugateComplex(ComputeBuffer src, ComputeBuffer dst)
    {
        _shader.SetBuffer(_conjugateComplexKernel, "Src", src);
        _shader.SetBuffer(_conjugateComplexKernel, "Dst", dst);
        Dispatch(_conjugateComplexKernel);
    }

    private void DivideComplexByDimensions(ComputeBuffer src, ComputeBuffer dst)
    {
        _shader.SetBuffer(_divideComplexByDimensionsKernel, "Src", src);
        _shader.SetBuffer(_divideComplexByDimensionsKernel, "Dst", dst);
        Dispatch(_divideComplexByDimensionsKernel);
    }
    #endregion

    #region Clean Up Methods
    private void ReleaseTwiddleBuffers()
    {
        _twiddleRow.Release();
        _twiddleCol.Release();
    }

    private void ReleaseBitRevBuffers()
    {
        _bitRevRow.Release();
        _bitRevCol.Release();
    }

    private void ReleaseTempBuffers()
    {
        _bufferA.Release();
        _bufferB.Release();
    }
    #endregion
}
