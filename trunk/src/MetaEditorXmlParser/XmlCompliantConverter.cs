using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Xml.Linq;

namespace MetaEditorXmlParser
{
  public class XmlCompliantConverter
  {
    public static XElement ParseNameValueToElement(XElement element)
    {
      Regex re = new Regex(@"([a-zA-Z_\.]+)=(.+)\n");

      if (element.Name == "parameter")
      {
          var ms = re.Matches(element.InnerText());
          foreach (Match m in ms)
              element.Add(new XElement(m.Groups[1].Value.Trim(), m.Groups[2].Value.Trim()));
      }
      else
      {
          foreach (var n in element.Nodes())
              if (n.NodeType == System.Xml.XmlNodeType.Text)
              {
                  var ms = re.Matches(n.ToString());
                  foreach (Match m in ms)
                      element.Add(new XElement(m.Groups[1].Value.Trim(), m.Groups[2].Value.Trim()));
              }
      }
        //string innerText = element.InnerText();
      //int idxFirstTag = innerText.Length;
      //if (element.HasElements && element.Name != "parameter") // parameter sometimes will have "<nobr>" tag in it
      //    idxFirstTag = innerText.IndexOf("<" + element.Elements().ElementAt(0).Name);

      ////if (idxFirstTag > 0)
      //{
      //  var ms = re.Matches(innerText.Substring(0, idxFirstTag));
      //  foreach (Match m in ms)
      //    element.Add(new XElement(m.Groups[1].Value.Trim(), m.Groups[2].Value.Trim()));
      //}
      return element;
    }
  }
}
