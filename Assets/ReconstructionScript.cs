using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Web;
using System.Text;
using UnityEngine;
using UnityEngine.Rendering;
using Unity.Collections.LowLevel.Unsafe;
using UnityEditor;
using Unity.Collections;

[RequireComponent(typeof(MeshFilter))]
public class ReconstructionScript : MonoBehaviour
{
    public string PATH = "C:/Users/furka/Desktop/Meshes/HumanBase.off";
    public int RES_MULTIPLIER = 6;
    public float samplingD = 1.12f;
    [DllImport("RecDLL", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl, EntryPoint = "LoadPC_andReconstruct")]
    private extern static IntPtr LoadPC_andReconstruct([In, MarshalAs(UnmanagedType.LPStr)] string PATH, int res_multiplier, float samplingD, ref int VertexCount, ref double Timing);
    // Start is called before the first frame update
    void Start()
    {
        //Generate();
    }

    // Update is called once per frame
    void Update()
    {
    }

    public void Generate()
    {
        Mesh mesh = new Mesh { name = "Reconstructed Mesh"};
        GetComponent<MeshFilter>().mesh = mesh;
        byte[] bytedata;
        int VertexCount = 0; double Timing = 0.0;
        unsafe
        {
            mesh.Clear();

            IntPtr RECDATA = LoadPC_andReconstruct(PATH, RES_MULTIPLIER, samplingD, ref VertexCount, ref Timing);
            bytedata = new byte[VertexCount * 12];
            Marshal.Copy(RECDATA, bytedata, 0, VertexCount * 12);
        }
        Debug.Log("Vertex Count: " + VertexCount + " and Timing: " + Timing);



        Vector3[] finaldata = new Vector3[VertexCount];
        for (int Vertex_i = 0; Vertex_i < VertexCount; Vertex_i++)
        {
            finaldata[Vertex_i] = new Vector3();
            finaldata[Vertex_i].x = BitConverter.ToSingle(bytedata, (Vertex_i * 3) * sizeof(float));
            finaldata[Vertex_i].y = BitConverter.ToSingle(bytedata, (1 + (Vertex_i * 3)) * sizeof(float));
            finaldata[Vertex_i].z = BitConverter.ToSingle(bytedata, (2 + (Vertex_i * 3)) * sizeof(float));
        }

        int[] finalib = new int[VertexCount];
        for (int i = 0; i < VertexCount; i++) { finalib[i] = i; };

        mesh.indexFormat = IndexFormat.UInt32;
        mesh.vertices = finaldata;
        mesh.triangles = finalib;
        mesh.RecalculateBounds();
        
    }
}