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

        var renderer = GetComponent<MeshRenderer>();
        renderer.material = LoadMaterial();
        
        UpdateMesh();
    }


    Material LoadMaterial()
    {
        // Path relative to a "Resources" folder (you must move your material into one)
        return Resources.Load<Material>("GlowingGold"); // Blue, GlowingGold, Red, ShinyDarkBlue GreyTransparent
    }

    Material SimpleMaterialCreate() 
    {
        var material = new Material(Shader.Find("Unlit/Color"));
        material.color = Color.red;
        return material; 
    }


    Material CreatePlasticMaterial()
    {
        var material = new Material(Shader.Find("Standard"));
        material.color = new Color(1f, 0.2f, 0.1f);  // plastic-like red
        material.SetFloat("_Glossiness", 0.8f);      // smooth, shiny surface
        material.SetFloat("_Metallic", 0.0f);        // plastic is non-metallic
        return material;
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