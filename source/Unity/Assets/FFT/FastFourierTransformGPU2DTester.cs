using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FastFourierTransformGPU2DTester : MonoBehaviour
{
    public ComputeShader _shader;
    private int _rowKernelHandle;
    private int _colKernelHandle;

    ComputeBuffer _bitRevRow;
    ComputeBuffer _bitRevCol;
    ComputeBuffer _twiddleRow;
    ComputeBuffer _twiddleCol;

    public Texture2D _source;
    private ComputeBuffer _intermediateBuffer;
    private RenderTexture _intermediateTexture;
    private ComputeBuffer _finalBuffer;
    private RenderTexture _finalTexture;

    public Renderer _sourceRenderer;
    public Renderer _intermediateRenderer;
    public Renderer _finalRenderer;

    private struct SimpleComplex
    {
        public double real;
        public double imag;
    }

    private void Awake()
    {
        _rowKernelHandle = _shader.FindKernel("SolveRow");
        _colKernelHandle = _shader.FindKernel("SolveCol");

        var WIDTH = _source.width; // should be power of 2
        _shader.SetInt("WIDTH", WIDTH);

        var HEIGHT = _source.height; // should be power of 2
        _shader.SetInt("HEIGHT", HEIGHT);

        _sourceRenderer.material.mainTexture = _source;
        _intermediateRenderer.material.mainTexture = _intermediateTexture;
        _finalRenderer.material.mainTexture = _finalTexture;

        _shader.SetTexture(_rowKernelHandle, "Src", _source);

        _bitRevRow = CreateBitRevBuffer((uint)WIDTH);
        _shader.SetBuffer(_rowKernelHandle, "BitRevRow", _bitRevRow);
        _bitRevCol = CreateBitRevBuffer((uint)HEIGHT);
        _shader.SetBuffer(_colKernelHandle, "BitRevCol", _bitRevCol);

        _twiddleRow = CreateTwiddleBuffer((uint)WIDTH / 2);
        _shader.SetBuffer(_rowKernelHandle, "TwiddleRow", _twiddleRow);
        _twiddleCol = CreateTwiddleBuffer((uint)HEIGHT / 2);
        _shader.SetBuffer(_colKernelHandle, "TwiddleCol", _twiddleCol);

        _intermediateBuffer = CreateComplexBuffer(WIDTH, HEIGHT);
        _shader.SetBuffer(_rowKernelHandle, "Intermediate", _intermediateBuffer);
        _shader.SetBuffer(_colKernelHandle, "Intermediate", _intermediateBuffer);

        _finalBuffer = CreateComplexBuffer(WIDTH, HEIGHT);
        _shader.SetBuffer(_colKernelHandle, "Final", _finalBuffer);

        _intermediateTexture = CreateRenderTexture(WIDTH, HEIGHT);
        _shader.SetTexture(_rowKernelHandle, "IntermediateTexture", _intermediateTexture);
        _finalTexture = CreateRenderTexture(WIDTH, HEIGHT);
        _shader.SetTexture(_colKernelHandle, "FinalTexture", _finalTexture);

        _shader.Dispatch(_rowKernelHandle, 1, HEIGHT / 8, 1);
        _shader.Dispatch(_colKernelHandle, WIDTH / 8, 1, 1);
    }

    private void OnDestroy()
    {
        _bitRevRow.Release();
        _bitRevCol.Release();
        _twiddleRow.Release();
        _twiddleCol.Release();

        _intermediateBuffer.Release();
        _intermediateTexture.Release();
        _finalBuffer.Release();
        _finalTexture.Release();
    }

    ComputeBuffer CreateBitRevBuffer(uint N)
    {
        ComputeBuffer bitRev = new ComputeBuffer((int)N, sizeof(uint), ComputeBufferType.Default);
        bitRev.SetData(FastFourierTransform.GenBitReversal(N).ToArray());
        return bitRev;
    }

    ComputeBuffer CreateTwiddleBuffer(uint halfN)
    {
        var twiddleRaw = FastFourierTransform.GenTwiddleFactors(halfN);
        var twiddleArray = new SimpleComplex[halfN];
        for (var i = 0; i < halfN; ++i)
        {
            twiddleArray[i].real = twiddleRaw[i].Real;
            twiddleArray[i].imag = twiddleRaw[i].Imaginary;
        }
        ComputeBuffer twiddle = new ComputeBuffer((int)halfN, sizeof(double) * 2, ComputeBufferType.Default);
        twiddle.SetData(twiddleArray);
        return twiddle;
    }

    ComputeBuffer CreateComplexBuffer(int width, int height)
    {
        ComputeBuffer buffer = new ComputeBuffer(width * height, sizeof(double) * 2, ComputeBufferType.Default);
        buffer.SetData(new SimpleComplex[width * height]);
        return buffer;
    }

    RenderTexture CreateRenderTexture(int width, int height)
    {
        RenderTexture tex = new RenderTexture(width, height, 24);
        tex.useMipMap = false;
        tex.wrapMode = TextureWrapMode.Clamp;
        tex.filterMode = FilterMode.Bilinear;
        tex.enableRandomWrite = true;
        tex.Create();
        return tex;
    }
}
