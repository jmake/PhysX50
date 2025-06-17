using UnityEditor;
using UnityEngine;

namespace SpicyTech {
    [CustomEditor(typeof(SpicyPlugging))]
    public class ComponentEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            EditorGUILayout.HelpBox(SpicyPlugging.something, MessageType.Info);
            DrawDefaultInspector();
        }
    }
} // SpicyTech