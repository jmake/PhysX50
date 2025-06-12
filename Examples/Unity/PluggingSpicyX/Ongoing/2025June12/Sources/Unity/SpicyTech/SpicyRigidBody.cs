using System;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

using System.IO;
using System.Collections.Generic;

using UnityEngine;


public class SpicyRigidBody : MonoBehaviour
{
    public int ibody; 
    public Vector3 spicyxP;
    public Quaternion spicyxQ; 

    SpicyX spicyx; 


    void Start()
    {
        spicyxP = new Vector3();  
        spicyxQ = Quaternion.identity; 
    }


    void Update()
    {
        
    }


    void FixedUpdate()
    {
    }


    public void RigidBodySet(GameObject obj, int id)
    {
        Vector3 scale = obj.transform.localScale;
        Vector3 position = obj.transform.position;
        Quaternion rotation = obj.transform.rotation;

        gameObject.transform.position = position;
        gameObject.transform.rotation = rotation;
        gameObject.transform.localScale = scale; 
        obj.transform.SetParent(gameObject.transform);

        HighlightObject(obj);
        ibody = id; 

        Debug.Log("[RigidBodySet] ibody:" + ibody);
    } 


    public Vector3 PositionGet() 
    {
        Vector3 localPosition = gameObject.transform.localPosition;
        return localPosition; 
    }


    public Quaternion QuaternionGet() 
    {
        Quaternion localRotation = gameObject.transform.localRotation;
        return localRotation; 
    }


    public void GlobalPoseSet(SpicyX spicyx) 
    {
        float[] p = {0.0f, 0.0f, 0.0f};
        float[] q = {0.0f, 0.0f, 0.0f, 0.0f};

        spicyx.GlobalPoseGet(ibody, p, q); 
        Debug.Log("[Tester] pose("+ ibody +").p: " + p[0] +" "+ p[1] +" "+ p[2]);
        Debug.Log("[Tester] pose("+ ibody +").q: " + q[0] +" "+ q[1] +" "+ q[2] +" "+ q[3]);

        spicyxP = new Vector3(p[0], p[1], p[2]);
        spicyxQ = new Quaternion(q[0], q[1], q[2], q[3]); 
        
        gameObject.transform.position = spicyxP;
        gameObject.transform.rotation = spicyxQ;
    }


    public void GlobalPoseGet(SpicyX spicyx) 
    {
        float[] p = {0.0f, 0.0f, 0.0f};
        float[] q = {0.0f, 0.0f, 0.0f, 0.0f};

        spicyx.GlobalPoseGet(ibody, p, q); 
        Debug.Log("[Tester] pose("+ ibody +").p: " + p[0] +" "+ p[1] +" "+ p[2]);
        Debug.Log("[Tester] pose("+ ibody +").q: " + q[0] +" "+ q[1] +" "+ q[2] +" "+ q[3]);

        spicyxP = new Vector3(p[0], p[1], p[2]);
        spicyxQ = new Quaternion(q[0], q[1], q[2], q[3]); 
    }


    void HighlightObject(GameObject obj)
    {
        Renderer renderer = obj.GetComponent<Renderer>();
        if (renderer != null)
        {
            Material originalMaterial = renderer.material;
            Material highlightMat = new Material(originalMaterial);
            highlightMat.color = highlightColor;
            renderer.material = highlightMat;
        }
    }


    [SerializeField] private Color highlightColor = Color.blue;
}