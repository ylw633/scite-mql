using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Linq;

namespace MetaEditorXmlParser
{
  public static class XElementExtension
  {
      public static string InnerText(this XElement element)
      {
        var reader = element.CreateReader();
        reader.MoveToContent();
        return reader.ReadInnerXml();
      }
  }
}
