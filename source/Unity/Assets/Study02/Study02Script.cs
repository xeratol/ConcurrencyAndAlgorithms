using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

struct Patricle
{
    public Vector3 pos;
    public Vector3 dir;
    public float life;
}

public class Study02Script : MonoBehaviour
{
    public ComputeShader _shader;
    public GameObject _objInstance;

    ComputeBuffer _particleBuffer;
    int _kernelHandle;
    const int _numParticles = 1000;
    GameObject[] _instances;

    private void Awake()
    {
        _kernelHandle = _shader.FindKernel("CSMain");

        InitializeParticleBuffer();
        InitializeParticleInstances();
    }

    private void InitializeParticleInstances()
    {
        _instances = new GameObject[_numParticles];
        
        for (var i = 0; i < _numParticles; ++i)
        {
            _instances[i] = Instantiate(_objInstance, transform.position, Quaternion.identity);
        }
    }

    private void OnDestroy()
    {
        _particleBuffer.Release();
    }

    void Update()
    {
        _shader.SetFloat("dt", Time.deltaTime);
        _shader.SetBuffer(_kernelHandle, "ParticleBuffer", _particleBuffer);
        _shader.Dispatch(_kernelHandle, 100, 1, 1);

        UpdateParticleInstances();
    }

    private void UpdateParticleInstances()
    {
        Patricle[] particleData = new Patricle[_numParticles];
        _particleBuffer.GetData(particleData);
        for (var i = 0; i < _numParticles; ++i)
        {
            _instances[i].transform.position = particleData[i].pos;
        }
    }

    void InitializeParticleBuffer()
    {
        _particleBuffer = new ComputeBuffer(_numParticles, sizeof(float) * 7, ComputeBufferType.Default);
        Patricle[] particleData = new Patricle[_numParticles];

        for (var i = 0; i < _numParticles; ++i)
        {
            var p = new Patricle();
            p.pos = Vector3.zero;
            p.dir = UnityEngine.Random.onUnitSphere;
            p.life = UnityEngine.Random.Range(2.0f, 4.0f);
            particleData[i] = p;
        }

        _particleBuffer.SetData(particleData);
    }
}
