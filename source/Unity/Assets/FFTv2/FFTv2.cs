using UnityEngine;

public class FFTv2 : MonoBehaviour
{
    public ComputeShader _shader;
    private int _convertTexToComplexKernel;
    private int _convertComplexMagToTexKernel;
    //private int _convertComplexPhaseToTexKernel;
    private int _centerComplexKernel;
    private int _bitRevByRowKernel;
    private int _bitRevByColKernel;
    private int _butterflyByRowKernel;
    private int _butterflyByColKernel;

    private ComputeBuffer _bitRevRow;
    private ComputeBuffer _bitRevCol;
    private ComputeBuffer _twiddleRow;
    private ComputeBuffer _twiddleCol;

    // these 2 are to be swapped back and forth
    private ComputeBuffer _bufferA;
    private ComputeBuffer _bufferB;

    public Texture2D _source;
    private RenderTexture _intermediate;
    private RenderTexture _final;

    public Renderer _sourceRenderer;
    public Renderer _intermediateRenderer;
    public Renderer _finalRenderer;

    private struct SimpleComplex
    {
        public float real;
        public float imag;
    }

    const int GROUP_SIZE_X = 16;   // must match shader
    const int GROUP_SIZE_Y = 16;   // must match shader

    #region Unity Methods
    private void Awake()
    {
        if (!IsInputValid())
        {
            return;
        }

        GetKernelHandles();

        InitRenderTextures();
        InitBitRevBuffers();
        InitTwiddleBuffers();
        InitTempBuffers();

        InitRenderers();
    }

    private void Start()
    {
        _shader.SetInt("WIDTH", _source.width);
        _shader.SetInt("HEIGHT", _source.height);

        _shader.SetTexture(_convertTexToComplexKernel, "SrcTex", _source);
        _shader.SetBuffer(_convertTexToComplexKernel, "Dst", _bufferA);
        Dispatch(_convertTexToComplexKernel);

        _shader.SetBuffer(_centerComplexKernel, "Src", _bufferA);
        _shader.SetBuffer(_centerComplexKernel, "Dst", _bufferB);
        Dispatch(_centerComplexKernel);

        _shader.SetBuffer(_bitRevByRowKernel, "BitRevRow", _bitRevRow);
        _shader.SetBuffer(_bitRevByRowKernel, "Src", _bufferB);
        _shader.SetBuffer(_bitRevByRowKernel, "Dst", _bufferA);
        Dispatch(_bitRevByRowKernel);

        _shader.SetBuffer(_butterflyByRowKernel, "TwiddleRow", _twiddleRow);
        var aIsSrc = true;
        for (int stride = 2; stride < _source.width; stride *= 2)
        {
            _shader.SetInt("BUTTERFLY_STRIDE", stride);
            _shader.SetBuffer(_butterflyByRowKernel, "Src", aIsSrc ? _bufferA : _bufferB);
            _shader.SetBuffer(_butterflyByRowKernel, "Dst", aIsSrc ? _bufferB : _bufferA);
            Dispatch(_butterflyByRowKernel);
            aIsSrc = !aIsSrc;
        }

        if (!aIsSrc)
        {
            SwapTempBuffers();
        }

        _shader.SetBuffer(_convertComplexMagToTexKernel, "Src", _bufferA);
        _shader.SetTexture(_convertComplexMagToTexKernel, "DstTex", _intermediate);
        Dispatch(_convertComplexMagToTexKernel);

        _shader.SetBuffer(_bitRevByColKernel, "BitRevCol", _bitRevCol);
        _shader.SetBuffer(_bitRevByColKernel, "Src", _bufferA);
        _shader.SetBuffer(_bitRevByColKernel, "Dst", _bufferB);
        Dispatch(_bitRevByColKernel);

        _shader.SetBuffer(_butterflyByColKernel, "TwiddleCol", _twiddleCol);
        aIsSrc = false;
        for (int stride = 2; stride < _source.height; stride *= 2)
        {
            _shader.SetInt("BUTTERFLY_STRIDE", stride);
            _shader.SetBuffer(_butterflyByColKernel, "Src", aIsSrc ? _bufferA : _bufferB);
            _shader.SetBuffer(_butterflyByColKernel, "Dst", aIsSrc ? _bufferB : _bufferA);
            Dispatch(_butterflyByColKernel);
            aIsSrc = !aIsSrc;
        }

        if (!aIsSrc)
        {
            SwapTempBuffers();
        }

        _shader.SetBuffer(_convertComplexMagToTexKernel, "Src", _bufferA);
        _shader.SetTexture(_convertComplexMagToTexKernel, "DstTex", _final);
        Dispatch(_convertComplexMagToTexKernel);
    }

    private void OnDestroy()
    {
        DestroyRenderTextures();
        DestroyBitRevBuffers();
        DestroyTwiddleBuffers();
        DestroyTempBuffers();
    }
    #endregion

    #region Utility Methods
    static ComputeBuffer CreateComplexBuffer(int width, int height)
    {
        ComputeBuffer buffer = new ComputeBuffer(width * height, sizeof(float) * 2, ComputeBufferType.Default);
        buffer.SetData(new SimpleComplex[width * height]);
        return buffer;
    }

    static ComputeBuffer CreateTwiddleBuffer(uint halfN)
    {
        var twiddleRaw = FastFourierTransform.GenTwiddleFactors(halfN);
        var twiddleArray = new SimpleComplex[halfN];
        for (var i = 0; i < halfN; ++i)
        {
            twiddleArray[i].real = (float)twiddleRaw[i].Real;
            twiddleArray[i].imag = (float)twiddleRaw[i].Imaginary;
        }
        ComputeBuffer twiddle = new ComputeBuffer((int)halfN, sizeof(float) * 2, ComputeBufferType.Default);
        twiddle.SetData(twiddleArray);
        return twiddle;
    }

    static ComputeBuffer CreateBitRevBuffer(uint N)
    {
        ComputeBuffer bitRev = new ComputeBuffer((int)N, sizeof(uint), ComputeBufferType.Default);
        bitRev.SetData(FastFourierTransform.GenBitReversal(N).ToArray());
        return bitRev;
    }

    static RenderTexture CreateRenderTexture(int width, int height)
    {
        RenderTexture tex = new RenderTexture(width, height, 24);
        tex.useMipMap = false;
        tex.wrapMode = TextureWrapMode.Clamp;
        tex.filterMode = FilterMode.Bilinear;
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

        if (_sourceRenderer == null || _intermediateRenderer == null || _finalRenderer == null)
        {
            return false;
        }

        return true;
    }

    private void Dispatch(int kernelHandle)
    {
        _shader.Dispatch(kernelHandle, _source.width / GROUP_SIZE_X, _source.height / GROUP_SIZE_Y, 1);
    }

    private void SwapTempBuffers()
    {
        ComputeBuffer c = _bufferA;
        _bufferA = _bufferB;
        _bufferB = c;
    }
    #endregion

    #region Init Methods
    private void InitTempBuffers()
    {
        _bufferA = CreateComplexBuffer(_source.width, _source.height);
        _bufferB = CreateComplexBuffer(_source.width, _source.height);
    }

    private void InitTwiddleBuffers()
    {
        _twiddleRow = CreateTwiddleBuffer((uint)_source.width / 2);
        _twiddleCol = CreateTwiddleBuffer((uint)_source.height / 2);
    }

    private void InitBitRevBuffers()
    {
        _bitRevRow = CreateBitRevBuffer((uint)_source.width);
        _bitRevCol = CreateBitRevBuffer((uint)_source.height);
    }

    private void InitRenderers()
    {
        _sourceRenderer.material.mainTexture = _source;
        _intermediateRenderer.material.mainTexture = _intermediate;
        _finalRenderer.material.mainTexture = _final;
    }

    private void InitRenderTextures()
    {
        _intermediate = CreateRenderTexture(_source.width, _source.height);
        _final = CreateRenderTexture(_source.width, _source.height);
    }

    private void GetKernelHandles()
    {
        _convertTexToComplexKernel = _shader.FindKernel("ConvertTexToComplex");
        _convertComplexMagToTexKernel = _shader.FindKernel("ConvertComplexMagToTex");
        //_convertComplexPhaseToTexKernel = _shader.FindKernel("ConvertComplexPhaseToTex");
        _centerComplexKernel = _shader.FindKernel("CenterComplex");
        _bitRevByRowKernel = _shader.FindKernel("BitRevByRow");
        _bitRevByColKernel = _shader.FindKernel("BitRevByCol");
        _butterflyByRowKernel = _shader.FindKernel("ButterflyByRow");
        _butterflyByColKernel = _shader.FindKernel("ButterflyByCol");
    }
    #endregion

    #region Clean Up Methods
    private void DestroyTwiddleBuffers()
    {
        _twiddleRow.Release();
        _twiddleCol.Release();
    }

    private void DestroyBitRevBuffers()
    {
        _bitRevRow.Release();
        _bitRevCol.Release();
    }

    private void DestroyTempBuffers()
    {
        _bufferA.Release();
        _bufferB.Release();
    }

    private void DestroyRenderTextures()
    {
        _intermediate.Release();
        _final.Release();
    }
    #endregion
}
