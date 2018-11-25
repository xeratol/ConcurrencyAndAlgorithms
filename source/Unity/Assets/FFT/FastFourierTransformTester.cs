using System;
using System.Collections.Generic;
using System.Numerics;
using UnityEngine;

public class FastFourierTransformTester : MonoBehaviour
{
    private void Log(string s)
    {
        Debug.Log(s);
    }

    private bool Test(List<Complex> data, List<Complex> expOut, bool print = false)
    {
        if (print)
        {
            Log("Input data:");
            PrintVector(data);
        }

        var N = FastFourierTransform.RoundUpPowerOf2((uint)data.Count);
        var bitRev = FastFourierTransform.GenBitReversal(N);
        var twiddle = FastFourierTransform.GenTwiddleFactors(N / 2);

        if (print)
        {
            Log("Bit Reversed Indices:");
            PrintVector(bitRev);
        }

        if (print)
        {
            Log("Twiddle Factors:");
            PrintVector(twiddle);
        }

        var result = FastFourierTransform.Solve(data, bitRev, twiddle);

        if (print)
        {
            Log("Result:");
            PrintVector(result);
        }

        if (result.Count != expOut.Count)
        {
            return false;
        }

        for (var i = 0; i < result.Count; ++i)
        {
            if ( Math.Abs(result[i].Magnitude - expOut[i].Magnitude) > Double.Epsilon )
            {
                return false;
            }
        }

        return true;
    }

    private void PrintVector<T>(List<T> data)
    {
        string s = "";
        for (var i = 0; i < data.Count; ++i)
        {
            s += "[" + i.ToString("00") + "] :" + data[i].ToString() + "\n";
        }
        Log(s);
    }

    private void Start()
    {
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
            new List<Complex>{ 36, new Complex(-4, 9.65), new Complex(-4, 4), new Complex(-4, 1.65), -4, new Complex(-4, -1.65), new Complex( -4, -4), new Complex(-4, -9.65) },
        };

        for (var i = 0; i < inputs.Count; ++i)
        {
            if (Test(inputs[i], outputs[i], true))
            {
                Log("<color=green>Test " + i + " succeeded.</color>");
            }
            else
            {
                Log("<color=red>Test " + i + " failed.</color>");
                PrintVector(outputs[i]);
            }
        }
    }
}
