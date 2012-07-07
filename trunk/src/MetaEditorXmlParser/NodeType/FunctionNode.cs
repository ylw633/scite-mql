using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
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
        var content = Regex.Split(Content, @"\.(([\s\n]+)|(\<br\s*/*\>))");
      string desc = content[0] + "."; // get first sentenece
        desc = Regex.Replace(desc, @"<[^>]*>", String.Empty); // strip out html tags
        desc = Regex.Replace(desc, @"\s+", " "); // replace multiple white space to 1
        desc = desc.Replace("\n", " ");
      string s = (Name.IndexOf(' ') > 0 ? Name.Remove(Name.IndexOf(' ')) : Name) + "(";
      if (Parameters != null)
          s = Parameters.Aggregate(s, (current, p) => current + (p.Type.Replace("&amp;", "&") + ((!string.IsNullOrWhiteSpace(p.Name) ? " " + p.Name.Replace("<nobr>", "").Replace("</nobr>", "") : "") + (string.IsNullOrEmpty(p.DefaultValue) ? "" : "=" + p.DefaultValue)) + ", "));
      return s.TrimEnd().TrimEnd(',') + ")\\n\\n" + desc;
    }

    protected override void Parse(XElement element)
    {
        base.Parse(element);
      Name = element.Element("caption").Value.Replace("()", "");
      ReturnType = element.Element("type").Value;
      Parameters = ExtractParameter(Element);
    }
  }
}
