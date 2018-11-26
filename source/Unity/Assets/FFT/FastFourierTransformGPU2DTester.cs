using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FastFourierTransformGPU2DTester : MonoBehaviour
{
    public ComputeShader _shader;
    private int _rowKernelHandle;
    //private int _colKernelHandle;

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
        //_colKernelHandle = _shader.FindKernel("SolveCol");

        var WIDTH = _source.width; // should be power of 2
        _shader.SetInt("WIDTH", WIDTH);

        var HEIGHT = _source.height; // should be power of 2
        _shader.SetInt("HEIGHT", HEIGHT);

        Debug.Log("Width: " + WIDTH);
        Debug.Log("Height: " + HEIGHT);

        _sourceRenderer.material.mainTexture = _source;

        ComputeBuffer bitRev = new ComputeBuffer(WIDTH, sizeof(uint), ComputeBufferType.Default);
        bitRev.SetData(FastFourierTransform.GenBitReversal((uint)WIDTH).ToArray());
        _shader.SetBuffer(_rowKernelHandle, "BitRevRow", bitRev);

        var twiddleRaw = FastFourierTransform.GenTwiddleFactors((uint)WIDTH / 2);
        var twiddleArray = new SimpleComplex[(uint)WIDTH / 2];
        for (var i = 0; i < WIDTH / 2; ++i)
        {
            twiddleArray[i].real = twiddleRaw[i].Real;
            twiddleArray[i].imag = twiddleRaw[i].Imaginary;
        }
        ComputeBuffer twiddle = new ComputeBuffer(WIDTH / 2, sizeof(double) * 2, ComputeBufferType.Default);
        twiddle.SetData(twiddleArray);
        _shader.SetBuffer(_rowKernelHandle, "TwiddleRow", twiddle);

        _shader.SetTexture(_rowKernelHandle, "Src", _source);

        var intermediateData = new SimpleComplex[WIDTH * HEIGHT];
        _intermediateBuffer = new ComputeBuffer(WIDTH * HEIGHT, sizeof(double) * 2, ComputeBufferType.Default);
        _intermediateBuffer.SetData(intermediateData);
        _shader.SetBuffer(_rowKernelHandle, "Intermediate", _intermediateBuffer);

        _intermediateTexture = new RenderTexture(WIDTH, HEIGHT, 24);
        _intermediateTexture.useMipMap = false;
        _intermediateTexture.filterMode = FilterMode.Point;
        _intermediateTexture.enableRandomWrite = true;
        _intermediateTexture.Create();
        _shader.SetTexture(_rowKernelHandle, "IntermediateTexture", _intermediateTexture);

        _shader.Dispatch(_rowKernelHandle, 1, HEIGHT / 8, 1);

        _intermediateRenderer.material.mainTexture = _intermediateTexture;

        _intermediateBuffer.GetData(intermediateData);

        bitRev.Release();
        twiddle.Release();
        _intermediateBuffer.Release();
    }

    private void OnDestroy()
    {
        _intermediateTexture.Release();
    }
}
