using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MetaEditorXmlParser.NodeType;

namespace MetaEditorXmlParser
{
  class Program
  {
    static void Main(string[] args)
    {
      var runner = new DataConstructor();
      runner.Analyze();
      //runner.ProduceChmMaterial("html");
      foreach (var f in runner.PageNodes.Where(e => e is FunctionNode))
        Console.WriteLine(((FunctionNode)f).ToSciteApiString());
      //foreach (var s in runner.Constants)
      //    Console.Write(s + " ");
      //Console.ReadLine();
    }
  }
}
