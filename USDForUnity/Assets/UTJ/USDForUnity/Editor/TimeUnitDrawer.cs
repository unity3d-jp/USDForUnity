using UnityEngine;
using UnityEditor;
using System.Collections;

namespace UTJ
{
    [CustomPropertyDrawer(typeof(TimeUnit))]
    class TimeUnitDrawer : PropertyDrawer
    {
        public override void OnGUI(Rect position, SerializedProperty property, GUIContent label)
        {
            EditorGUI.BeginProperty(position, label, property);
            position = EditorGUI.PrefixLabel(position, GUIUtility.GetControlID(FocusType.Passive), label);

            float typeWidth = 110;
            float scaleWidth = position.width - typeWidth - 5;
            Rect typeRect = new Rect(position.x, position.y, typeWidth, position.height);
            Rect scaleRect = new Rect(position.x + typeWidth + 5, position.y, scaleWidth, position.height);

            var typeProperty = property.FindPropertyRelative("m_type");
            var scaleProperty = property.FindPropertyRelative("m_scale");

            EditorGUI.BeginChangeCheck();
            EditorGUI.PropertyField(typeRect, typeProperty, GUIContent.none);
            var type = (TimeUnit.Types)typeProperty.enumValueIndex;
            if (type == TimeUnit.Types.FreeScale)
            {
                EditorGUI.PropertyField(scaleRect, scaleProperty, GUIContent.none);
            }
            if (EditorGUI.EndChangeCheck())
            {
                var scale = scaleProperty.floatValue;
                scaleProperty.floatValue = TimeUnit.Adjust(type, scale);
            }


            EditorGUI.EndProperty();
        }
    }
}
