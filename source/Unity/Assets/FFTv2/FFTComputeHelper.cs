using UnityEngine;

public class FFTComputeHelper
{
    private ComputeShader _shader;
    const int GROUP_SIZE_X = 16;   // must match shader
    const int GROUP_SIZE_Y = 16;   // must match shader

    #region Kernel Handles
    private int _convertTexToComplexKernel;
    private int _convertComplexMagToTexKernel;
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

    private int _width;
    private int _height;

    private struct SimpleComplex
    {
        public float real;
        public float imag;
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
            FastFourierTransform.RoundUpPowerOf2((uint)height) != (uint)height)
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

    public void Forward(Texture2D source, RenderTexture final, RenderTexture intermediate = null)
    {
        if (source == null || source.width != _width || source.height != _height ||
            final == null || final.width != _width || final.height != _height)
        {
            throw new System.Exception("Invalid Parameters");
        }

        if (intermediate != null && (intermediate.width != _width || intermediate.height != _height))
        {
            throw new System.Exception("Invalid Parameters");
        }

        ConvertTexToComplex(source, _bufferA);
        CenterComplex(_bufferA, _bufferB);

        BitRevByRow(_bufferB, _bufferA);
        ButterflyByRow(_bufferA, _bufferB);

        if (intermediate != null)
        {
            ConvertComplexMagToTex(_bufferB, intermediate);
        }

        BitRevByCol(_bufferB, _bufferA);
        ButterflyByCol(_bufferA, _bufferB);

        ConvertComplexMagToTex(_bufferB, final);

        // _bufferB contains the output of the FFT
    }

    public void Inverse(RenderTexture final, RenderTexture intermediate = null)
    {
        if (final == null || final.width != _width || final.height != _height)
        {
            throw new System.Exception("Invalid Parameters");
        }

        if (intermediate != null && (intermediate.width != _width || intermediate.height != _height))
        {
            throw new System.Exception("Invalid Parameters");
        }

        // we assume that _bufferB contains the data from the Forward

        ConjugateComplex(_bufferB, _bufferA);

        BitRevByRow(_bufferA, _bufferB);
        ButterflyByRow(_bufferB, _bufferA);

        if (intermediate != null)
        {
            ConvertComplexMagToTex(_bufferB, intermediate);
        }

        BitRevByCol(_bufferA, _bufferB);
        ButterflyByCol(_bufferB, _bufferA);

        ConjugateComplex(_bufferA, _bufferB);
        DivideComplexByDimensions(_bufferB, _bufferA);

        ConvertComplexMagToTex(_bufferB, final);
    }

    #region Init Methods
    private void GetKernelHandles()
    {
        _convertTexToComplexKernel = _shader.FindKernel("ConvertTexToComplex");
        _convertComplexMagToTexKernel = _shader.FindKernel("ConvertComplexMagToTex");
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
        var twiddleArray = new SimpleComplex[halfN];
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
        buffer.SetData(new SimpleComplex[width * height]);
        return buffer;
    }

    private void SwapBuffers(ComputeBuffer a, ComputeBuffer b)
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
    private void ButterflyByRow(ComputeBuffer src, ComputeBuffer dst)
    {
        _shader.SetBuffer(_butterflyByRowKernel, "TwiddleRow", _twiddleRow);
        var srcIsSrc = true; // TODO what a stupid name
        for (int stride = 2; stride <= _width; stride *= 2)
        {
            _shader.SetInt("BUTTERFLY_STRIDE", stride);
            _shader.SetBuffer(_butterflyByRowKernel, "Src", srcIsSrc ? src : dst);
            _shader.SetBuffer(_butterflyByRowKernel, "Dst", srcIsSrc ? dst : src);
            Dispatch(_butterflyByRowKernel);
            srcIsSrc = !srcIsSrc;
        }

        if (srcIsSrc)
        {
            SwapBuffers(src, dst);
        }
    }

    // Both src and dst will be modified
    private void ButterflyByCol(ComputeBuffer src, ComputeBuffer dst)
    {
        _shader.SetBuffer(_butterflyByColKernel, "TwiddleCol", _twiddleCol);
        var srcIsSrc = true; // TODO what a stupid name
        for (int stride = 2; stride <= _height; stride *= 2)
        {
            _shader.SetInt("BUTTERFLY_STRIDE", stride);
            _shader.SetBuffer(_butterflyByColKernel, "Src", srcIsSrc ? src : dst);
            _shader.SetBuffer(_butterflyByColKernel, "Dst", srcIsSrc ? dst : src);
            Dispatch(_butterflyByColKernel);
            srcIsSrc = !srcIsSrc;
        }

        if (srcIsSrc)
        {
            SwapBuffers(src, dst);
        }
    }

    private void ConvertComplexMagToTex(ComputeBuffer src, RenderTexture dst)
    {
        _shader.SetBuffer(_convertComplexMagToTexKernel, "Src", src);
        _shader.SetTexture(_convertComplexMagToTexKernel, "DstTex", dst);
        Dispatch(_convertComplexMagToTexKernel);
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
