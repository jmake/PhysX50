using System;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

using System.IO;
using System.Collections.Generic;

using UnityEngine;


public class MeshCreator : MonoBehaviour
{
    Mesh mesh;

    int[] triangles;      // length = nTriangles * 3
    float[] vertices;     // length = nbVertices * nComponents 

    int nComponents = 3; 


    public void Init() 
    {
        vertices = new float[]{}; 
        triangles = new int[]{}; 

        mesh = new Mesh();
        mesh.name = "GeneratedMesh";

        var meshFilter = gameObject.AddComponent<MeshFilter>(); 
        meshFilter.mesh = mesh;

        var renderer = gameObject.AddComponent<MeshRenderer>(); 
    }


    void FixedUpdate()
    {
        //UpdateMesh(); 
    }


    public void UpdateMesh()
    {
        int nbVertices = vertices.Length / nComponents;
        Debug.Log("[UpdateMesh] nbVertices:" + nbVertices);

        Vector3[] meshVertices = new Vector3[nbVertices];
        for (int i = 0; i < nbVertices; i++)
        {
            Vector3 row = new Vector3(
                vertices[i * nComponents + 0],
                vertices[i * nComponents + 1],
                vertices[i * nComponents + 2]
            );
            meshVertices[i] = row; 
            //Debug.Log($"{i}) " + string.Join(" ", row));
        }

        mesh.Clear();
        mesh.vertices = meshVertices;
        mesh.triangles = triangles;
        mesh.RecalculateNormals();
    }


    public void Load(string facesFile, string verticesFile) 
    {
        List<int> faces;
        List<float> vertex;

        string filePath; 
        filePath = Path.Combine(Application.streamingAssetsPath, verticesFile);
        //Debug.Log("[Start] verticesFile:" + filePath );
        LoadFile<float>(filePath, out vertex, show: false);
        Debug.Log("[LoadFile] vertices:'" + vertex.Count / nComponents +"' "); 

        filePath = Path.Combine(Application.streamingAssetsPath, facesFile);
        //Debug.Log("[Start] facesFile:" + filePath );
        LoadFile<int>(filePath, out faces, show: false);
        Debug.Log("[LoadFile] faces:'" + faces.Count / 3 +"' "); 

        triangles = faces.ToArray();
        vertices = vertex.ToArray();

        /*
        string filePath; 
        List<List<int>> faces;
        List<List<float>> vertices;

        filePath = Path.Combine(Application.streamingAssetsPath, verticesFile);
        Debug.Log("[Start] filePath:" + filePath );

        LoadFile<float>(filePath, out vertices, show: false); // 30360) 4.454038 7.50979 2.486248
        Debug.Log("[LoadFile] vertices:'" + vertices.Count +"' "); 

        filePath = Path.Combine(Application.streamingAssetsPath, facesFile);
        Debug.Log("[Start] filePath:" + filePath );

        LoadFile<int>(filePath, out faces, show: false);
        Debug.Log("[LoadFile] faces:'" + faces.Count +"' "); // 60717) 12726 11444 7241
        */
    }


    static bool LoadFile<T>(string filePath, out List<List<T>> data, bool show = false)
    {
        data = new List<List<T>>();

        filePath = Path.GetFullPath(filePath);

        if (!File.Exists(filePath))
        {
            Debug.Log($"[LoadFile] Error: Cannot open file '{filePath}'");
            return false;
        }

        int i = 0;
        foreach (var line in File.ReadLines(filePath))
        {
            List<T> row = new List<T>();
            var parts = line.Split(' ', StringSplitOptions.RemoveEmptyEntries);

            foreach (var val in parts)
            {
                try
                {
                    object converted;
                    if (typeof(T) == typeof(int))
                    {
                        float temp = float.Parse(val, System.Globalization.CultureInfo.InvariantCulture);
                        converted = (int)temp;
                    }
                    else
                    {
                        converted = Convert.ChangeType(val, typeof(T), System.Globalization.CultureInfo.InvariantCulture);
                    }
                    row.Add((T)converted);
                }
                catch (Exception e)
                {
                    Debug.Log($"[LoadFile] Conversion error: '{val}' -> {typeof(T)} | {e.Message}");
                    return false;
                }
            }

            if (show) Debug.Log($"{i++}) " + string.Join(" ", row));
            data.Add(row);
        }

        Debug.Log($"[LoadFile] file:'{filePath}' nlines:{data.Count}");
        return true;
    }


    static bool LoadFile<T>(string filePath, out List<T> data, bool show = false)
    {
        data = new List<T>();

        filePath = Path.GetFullPath(filePath);

        if (!File.Exists(filePath))
        {
            Debug.Log($"[LoadFile] Error: Cannot open file '{filePath}'");
            return false;
        }

        //int i = 0;
        foreach (var line in File.ReadLines(filePath))
        {
            //List<T> row = new List<T>();
            var parts = line.Split(' ', StringSplitOptions.RemoveEmptyEntries);

            foreach (var val in parts)
            {
                try
                {
                    object converted;
                    if (typeof(T) == typeof(int))
                    {
                        float temp = float.Parse(val, System.Globalization.CultureInfo.InvariantCulture);
                        converted = (int)temp;
                    }
                    else
                    {
                        converted = Convert.ChangeType(val, typeof(T), System.Globalization.CultureInfo.InvariantCulture);
                    }
                    data.Add((T)converted);
                }
                catch (Exception e)
                {
                    Debug.Log($"[LoadFile] Conversion error: '{val}' -> {typeof(T)} | {e.Message}");
                    return false;
                }
            }

            //if (show) Debug.Log($"{i++}) " + string.Join(" ", row));
            //data.Add(row);
        }

        Debug.Log($"[LoadFile] file:'{filePath}' nlines:{data.Count}");
        return true;
    } 

}
