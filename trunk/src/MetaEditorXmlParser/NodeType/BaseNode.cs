using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Xml.Linq;

namespace MetaEditorXmlParser.NodeType
{
  public class Parameter
  {
    public string Name { get; set; }
    public string Type { get; set; }
    public string Description { get; set; }

    public Parameter(string name, string type, string description)
    {
      Name = name;
      Type = type;
      Description = description;
    }

    public Parameter(XElement element)
    {
      Name = element.Element("name") != null ? element.Element("name").Value : string.Empty;
      Type = element.Element("type") != null ? element.Element("type").Value : string.Empty;
      Description = element.Element("description") != null ? element.Element("description").Value : string.Empty;
    }
  }

  public class BaseNode
  {
    public XElement Element { get; set; }
    public string Content { get { return Element == null || Element.Elements("content") == null ? null : Element.Elements("content").Aggregate("", (c, e) => c + e.InnerText()); } }
    public string Key { get { return Element == null || Element.Element("tag") == null ? null : Element.Element("tag").Value; } }
    public string Title { get { return Element == null || Element.Element("caption") == null ? null : Element.Element("caption").Value; } }
    public IList<string> Keywords { get; protected set; }
      public IList<BaseNode> Children { get; set; }

    public BaseNode(XElement element)
    {
      Element = element;
        Children = new List<BaseNode>();
      Parse(element);
    }

    protected BaseNode() { }

    protected IList<Parameter> ExtractParameter(XElement element)
    {
      if (!element.Elements("parameter").Any())
        return null;

      return element.Elements("parameter").Select(e => new Parameter(e)).ToList();
    }

    protected virtual void Parse(XElement element)
    {
      if (element != null && element.Element("keywords") != null)
        Keywords = element.Element("keywords").Value.Split(new[] {';'}, StringSplitOptions.RemoveEmptyEntries).Where(s => !string.IsNullOrWhiteSpace(s.Trim())).ToList();
    }

    public virtual string ToHtml()
    {
      string html = Content;
      if (Element.Element("type") != null)
      {
        string type = Element.Element("type").Value,
            name = (Element.Element("caption").Value.IndexOf(' ') > 0 ? Element.Element("caption").Value.Remove(Element.Element("caption").Value.IndexOf(' ')) : Element.Element("caption").Value).TrimEnd('(', ')');
        string declaration = @"<table class=""docvar"" border=""0"" cellpadding=""0"" cellspacing=""0""><tr valign=top><td><font color=blue>" + type + @"</font> <span class=""docfunc"">" + name + @"</span>";
        if (this is FunctionNode)
        {
            declaration += @"(</td><td>";
            if (((FunctionNode)this).Parameters != null)
            {
                declaration = ((FunctionNode)this).Parameters.Aggregate(declaration, (c, p) => c + ((string.IsNullOrEmpty(p.Name) ? p.Type : @"<td><font color=""blue"">" + p.Type + @"</font>&nbsp;" + p.Name) + ", &nbsp;"));
                declaration = declaration.Remove(declaration.LastIndexOf(", &nbsp;"));
            }
            declaration += @")</td>";
        }
        else
        {
            declaration += @"</td>";
        }
          declaration += @"</tr></table>";

        html = declaration + html;
      }

      if (this is FunctionNode && ((FunctionNode)this).Parameters != null)
      {
        string table = @"<div class=""doctabhdr"">Parameters:</div><table border=""0"" class=""docparams"" cellpadding=""2"" cellspacing=""2"">";
          table = ((FunctionNode)this).Parameters.Aggregate(table, (c, p) => c + (@"<tr><td valign=""top""><b>" + (string.IsNullOrEmpty(p.Name) ? p.Type : p.Name) + @"</b></td><td nowrap valign=""top"">&nbsp; - &nbsp;</td><td width=""100%"">" + p.Description + @"</td></tr>"));
        table += "</table>";

        html += table;
      }

      if (Element.Element("example") != null)
      {
        string example = @"<div class=""doctabhdr"">Sample:</div><pre class=""docmql4"">" + Element.Element("example").InnerText() + @"</pre></div>";

        html += example;
      }

        Regex re = new Regex(@"""help://(\w+)""", RegexOptions.Compiled);
        html = re.Replace(html, @"""$1.html""");

      return html;
    }

    public static BaseNode GetNode(XElement element)
    {
      return element.Name.ToString() != "function" ? new BaseNode(element) : new FunctionNode(element);
    }
  }
}
