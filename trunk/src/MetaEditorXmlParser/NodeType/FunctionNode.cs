using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Linq;

namespace MetaEditorXmlParser.NodeType
{
  public class FunctionNode : BaseNode
  {
    public string Name { get; set; }
    public string ReturnType { get; set; }
    public IList<Parameter> Parameters { get; set; }
    public string Example { get; set; }

    public FunctionNode(XElement element) : base(element) {}

    public string ToSciteApiString()
    {
      string desc = Content.Substring(0, Content.IndexOf('.') + 1); // get first sentenece
      string s = Name + "(";
      if (Parameters != null)
        s = Parameters.Aggregate(s, (current, p) => current + (p.Type + " " + p.Name.Replace("<nobr>", "").Replace("</nobr>", "") + ", "));
      return s.TrimEnd().TrimEnd(',') + ")\\n\\n" + desc.Trim();
    }

    protected override void Parse(XElement element)
    {
      Name = element.Element("caption").Value.Replace("()", "");
      ReturnType = element.Element("type").Value;
      Parameters = ExtractParameter(element);
    }
  }
}
