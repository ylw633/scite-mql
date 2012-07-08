using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;
using MetaEditorXmlParser.NodeType;

namespace MetaEditorXmlParser
{
  class Program
  {
    static void Main(string[] args)
    {
      var runner = new DataConstructor();
      runner.Analyze();
        //using (var writer = new XmlTextWriter("output.xml", Encoding.UTF8))
        //{
        //    foreach (var e in runner.Root.Children)
        //        e.Element.WriteTo(writer);
        //}
      runner.ProduceChmMaterial("html");
      //foreach (var f in runner.PageNodes.Where(e => e is FunctionNode))
      //  Console.WriteLine(((FunctionNode)f).ToSciteApiString());
      //foreach (var s in runner.Constants)
      //    Console.Write(s + " ");
      //Console.ReadLine();
    }
  }
}
