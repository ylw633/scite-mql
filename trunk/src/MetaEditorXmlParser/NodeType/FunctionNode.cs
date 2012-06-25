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
      Parameters = ExtractParameter(Element);
      string desc = Content.Substring(0, Content.IndexOf('.') + 1); // get first sentenece
      string s = (Name.IndexOf(' ') > 0 ? Name.Remove(Name.IndexOf(' ')) : Name) + "(";
      if (Parameters != null)
        s = Parameters.Aggregate(s, (current, p) => current + (p.Type + (!string.IsNullOrWhiteSpace(p.Name) ? " " + p.Name.Replace("<nobr>", "").Replace("</nobr>", "") : "") + ", "));
      return s.TrimEnd().TrimEnd(',') + ")\\n\\n" + desc.Trim().Replace("\n", " ");
    }

    protected override void Parse(XElement element)
    {
      Name = element.Element("caption").Value.Replace("()", "");
      ReturnType = element.Element("type").Value;
    }
  }
}
