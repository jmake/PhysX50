using UnityEngine;

[RequireComponent(typeof(MeshFilter), typeof(MeshRenderer))]
public class VertexVisualizer : MonoBehaviour
{
    public int[] triangles;      // length = nTriangles * 3
    public float[] vertices;     // length = nbVertices * 4

    private Mesh mesh;

    void Start()
    {
        if (vertices == null || vertices.Length % 4 != 0 || triangles == null || triangles.Length % 3 != 0)
        {
            Debug.LogError("Invalid vertex or triangle data.");
            enabled = false;
            return;
        }

        mesh = new Mesh();
        mesh.name = "GeneratedMesh";
        GetComponent<MeshFilter>().mesh = mesh;

        var redMaterial = new Material(Shader.Find("Unlit/Color"));
        redMaterial.color = Color.red;
        redMaterial.SetInt("_Cull", (int)UnityEngine.Rendering.CullMode.Off);

        var renderer = GetComponent<MeshRenderer>();
        renderer.material = redMaterial;
        
        UpdateMesh();
    }

    void FixedUpdate()
    {
        UpdateMesh(); // Updates vertex positions each physics step
    }


    void UpdateMesh()
    {
        int nbVertices = vertices.Length / 4;

        Vector3[] meshVertices = new Vector3[nbVertices];
        for (int i = 0; i < nbVertices; i++)
        {
            meshVertices[i] = new Vector3(
                vertices[i * 4 + 0],
                vertices[i * 4 + 1],
                vertices[i * 4 + 2]
            );
        }

        mesh.Clear();
        mesh.vertices = meshVertices;
        mesh.triangles = triangles;
        mesh.RecalculateNormals();
    }

}