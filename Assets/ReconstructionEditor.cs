using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

[CustomEditor(typeof(ReconstructionScript))]
[CanEditMultipleObjects]
public class ReconstructionEditor : Editor
{
    SerializedProperty pathProp;
    SerializedProperty res_multiplierProp;
    SerializedProperty samplingDProp;

    void OnEnable()
    {
        pathProp = serializedObject.FindProperty("PATH");
        res_multiplierProp = serializedObject.FindProperty("RES_MULTIPLIER");
        samplingDProp = serializedObject.FindProperty("samplingD");
    }
    public override void OnInspectorGUI()
    {
        serializedObject.Update();
        
        ReconstructionScript myScript = (ReconstructionScript)target;
        pathProp.stringValue = EditorGUILayout.TextField("PATH: ", myScript.PATH);
        EditorGUILayout.IntSlider(res_multiplierProp, 1, 100, new GUIContent("Resolution Multiplier"));
        samplingDProp.floatValue = EditorGUILayout.FloatField("SamplingD: ", samplingDProp.floatValue);

        if (GUILayout.Button("Generate"))
        {
            myScript.Generate();
        }
        serializedObject.ApplyModifiedProperties();
    }
}
