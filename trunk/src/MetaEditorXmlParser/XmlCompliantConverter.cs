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

      string innerText = element.InnerText();
      int idxFirstTag = innerText.Length;
      if (element.HasElements && element.Name != "parameter")
          idxFirstTag = innerText.IndexOf("<" + element.Elements().ElementAt(0).Name);

      //if (idxFirstTag > 0)
      {
        var ms = re.Matches(innerText.Substring(0, idxFirstTag));
        foreach (Match m in ms)
          element.Add(new XElement(m.Groups[1].Value.Trim(), m.Groups[2].Value.Trim()));
      }
      return element;
    }
  }
}
