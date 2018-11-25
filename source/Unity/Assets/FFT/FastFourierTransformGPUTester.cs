using System;
using System.Collections.Generic;
using UnityEngine;

public class FastFourierTransformGPUTester : MonoBehaviour
{
    public ComputeShader _shader;
    int _kernelHandle;

    private struct Complex
    {
        public double real;
        public double imag;

        public Complex(double r = 0, double i = 0)
        {
            real = r;
            imag = i;
        }

        public static implicit operator Complex(int r)
        {
            return new Complex(r);
        }

        public double Magnitude
        {
            get
            {
                return Math.Sqrt(real * real + imag * imag);
            }
        }

        public string ToString(string format = "")
        {
            return string.Format("({0}, {1})", real.ToString(format), imag.ToString(format));
        }
    }

    private void Awake()
    {
        _kernelHandle = _shader.FindKernel("Solve");

        List<List<Complex>> inputs = new List<List<Complex>>
        {
            new List<Complex>{ 1, 2, 4, 4 },
            new List<Complex>{ 1, 0, 1, 0 },
            new List<Complex>{ 1, 1, 0, 0 },
            new List<Complex>{ 1, 2, 4, 4, 1, 2, 4, 4 },
            new List<Complex>{ 1, 2, 3, 4, 5, 6, 7, 8 },
        };

        List<List<Complex>> outputs = new List<List<Complex>>
        {
            new List<Complex>{ 11, new Complex(-3, 2), -1, new Complex(-3, -2) },
            new List<Complex>{ 2, 0, 2, 0 },
            new List<Complex>{ 2, new Complex(1, -1), 0, new Complex(1, 1) },
            new List<Complex>{ 22, 0, new Complex(-6, 4), 0, -2, 0, new Complex(- 6, -4), 0 },
            new List<Complex>{ 36, new Complex(-4.0f, 9.65f), new Complex(-4, 4), new Complex(-4f, 1.65f), -4, new Complex(-4f, -1.65f), new Complex(-4, -4), new Complex(-4f, -9.65f) },
        };

        for (int i = 0; i < inputs.Count; ++i)
        {
            var result = Solve(inputs[i]);
            for (var j = 0; j < result.Count; ++j)
            {
                if (Math.Abs(result[j].Magnitude - outputs[i][j].Magnitude) > double.Epsilon)
                {
                    Log("<color=red>Test " + i + " failed</color>");
                    PrintVector(result);
                    PrintVector(outputs[i]);
                    break;
                }
            }
        }
    }

    private List<Complex> Solve(List<Complex> input)
    {
        int N = (int)FastFourierTransform.RoundUpPowerOf2((uint)input.Count);
        _shader.SetInt("N", N);

        ComputeBuffer src = new ComputeBuffer(N, sizeof(double) * 2, ComputeBufferType.Default);
        src.SetData(input.ToArray());
        _shader.SetBuffer(_kernelHandle, "Src", src);

        ComputeBuffer bitRev = new ComputeBuffer(N, sizeof(uint), ComputeBufferType.Default);
        bitRev.SetData(FastFourierTransform.GenBitReversal((uint)N).ToArray());
        _shader.SetBuffer(_kernelHandle, "BitRev", bitRev);

        var twiddleRaw = FastFourierTransform.GenTwiddleFactors((uint)N / 2);
        var twiddleArray = new Complex[(uint)N / 2];
        for (var i = 0; i < N / 2; ++i)
        {
            twiddleArray[i].real = twiddleRaw[i].Real;
            twiddleArray[i].imag = twiddleRaw[i].Imaginary;
        }
        ComputeBuffer twiddle = new ComputeBuffer(N / 2, sizeof(double) * 2, ComputeBufferType.Default);
        twiddle.SetData(twiddleArray);
        _shader.SetBuffer(_kernelHandle, "Twiddle", twiddle);

        Complex [] result = new Complex[N];
        ComputeBuffer dst = new ComputeBuffer(N, sizeof(double) * 2, ComputeBufferType.Default);
        dst.SetData(result);
        _shader.SetBuffer(_kernelHandle, "Dst", dst);

        _shader.Dispatch(_kernelHandle, 1, 1, 1);

        dst.GetData(result);

        src.Release();
        bitRev.Release();
        twiddle.Release();
        dst.Release();

        return new List<Complex>(result);
    }

    private void Log(string s)
    {
        Debug.Log(s);
    }

    private void PrintVector(List<Complex> data)
    {
        string s = "";
        for (var i = 0; i < data.Count; ++i)
        {
            s += "[" + i.ToString("00") + "] :" + data[i].ToString() + "\n";
        }
        Log(s);
    }
}
