using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq.Expressions;
using UnityEditor;
using UnityEngine;

namespace UTJ.USD
{
    public static class SerializableObjHelper
    {
        private static List<string> BreakdownExpression(Expression<Func<object>> exp)
        {
            Expression node = exp.Body;
            var parts = new List<string>();
            bool done = false;
            while (!done)
            {
                if (node is UnaryExpression)
                {
                    var unode = node as UnaryExpression;
                    var mnode = unode.Operand as MemberExpression;
                    parts.Insert(0, mnode.Member.Name);
                    node = mnode.Expression;
                }
                else if (node is MemberExpression)
                {
                    var mnode = node as MemberExpression;
                    parts.Insert(0, mnode.Member.Name);
                    node = mnode.Expression;
                }
                else
                {
                    done = true;
                    continue;
                }
            }
            return parts;
        }

        public static SerializedProperty FindProperty(this SerializedObject obj, Expression<Func<object>> exp)
        {
            var parts = BreakdownExpression(exp);

            SerializedProperty property = null;
            for ( int i = 1; i < parts.Count; i++)
            {
                if (property == null)
                    property = obj.FindProperty(parts[i]);
                else
                    property = property.FindPropertyRelative(parts[i]);

                if (property == null)
                    return null;
            }

            return property;
        }

        public static SerializedProperty FindPropertyRelative(this SerializedProperty obj, Expression<Func<object>> exp)
        {
            var parts = BreakdownExpression(exp);

            var property = obj;
            for (int i = 1; i < parts.Count; i++)
            {
                if (property == null)
                    return null;

                property = property.FindPropertyRelative(parts[i]);
            }

            return property;
        }

    }
}
