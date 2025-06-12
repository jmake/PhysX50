using UnityEditor;
using UnityEngine;

[CustomEditor(typeof(SpicyXPlugging1))]
public class MyComponentEditor : Editor
{
    public override void OnInspectorGUI()
    {
        EditorGUILayout.HelpBox("[MyComponentEditor] This is a custom message", MessageType.Info);
        DrawDefaultInspector();
    }
}