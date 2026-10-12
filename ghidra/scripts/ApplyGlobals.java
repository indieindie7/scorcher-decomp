// ApplyGlobals.java <abs file> [dry]: types pass step 2. Lines "0x<addr> <name> [<type>]" ('#' comments).
// Sets a USER_DEFINED label <name> at <addr> (primary) and, when <type> is given, clears the code units
// there and creates the data type: a /Scorcher struct name, a builtin (int, uint, short, ushort, byte,
// char, pointer, float, double), optionally with a "*" suffix (pointer to it) or "[n]" suffix (array).
//@category Scorcher
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.*;
import ghidra.program.model.symbol.*;
import java.nio.file.*;
import java.util.*;
public class ApplyGlobals extends GhidraScript {
  DataType builtin(String t) {
    switch (t) {
      case "int": return IntegerDataType.dataType;
      case "uint": return UnsignedIntegerDataType.dataType;
      case "short": return ShortDataType.dataType;
      case "ushort": return UnsignedShortDataType.dataType;
      case "byte": return ByteDataType.dataType;
      case "char": return CharDataType.dataType;
      case "pointer": return PointerDataType.dataType;
      case "float": return FloatDataType.dataType;
      case "double": return DoubleDataType.dataType;
    }
    return null;
  }
  DataType resolve(String t) {
    int n = 0;
    if (t.endsWith("]")) { int i = t.lastIndexOf('['); n = Integer.decode(t.substring(i + 1, t.length() - 1)); t = t.substring(0, i); }
    boolean ptr = t.endsWith("*");
    if (ptr) t = t.substring(0, t.length() - 1);
    DataType dt = builtin(t);
    if (dt == null) dt = currentProgram.getDataTypeManager().getDataType(new CategoryPath("/Scorcher"), t);
    if (dt == null) return null;
    if (ptr) dt = new PointerDataType(dt, currentProgram.getDataTypeManager());
    if (n > 0) dt = new ArrayDataType(dt, n, dt.getLength(), currentProgram.getDataTypeManager());
    return dt;
  }
  public void run() throws Exception {
    String[] a = getScriptArgs();
    boolean dry = a.length > 1 && a[1].equals("dry");
    int named = 0, typed = 0, bad = 0;
    for (String line : Files.readAllLines(Paths.get(a[0]))) {
      line = line.trim();
      if (line.isEmpty() || line.startsWith("#")) continue;
      String[] p = line.split("\\s+");
      if (p.length < 2) continue;
      Address ad = toAddr(Long.decode(p[0]));
      DataType dt = null;
      if (p.length > 2 && !p[2].equals("-")) {
        dt = resolve(p[2]);
        if (dt == null) { println("unknown type " + p[2] + " at " + p[0]); bad++; continue; }
      }
      if (dry) { named++; if (dt != null) typed++; continue; }
      Symbol s = getSymbolAt(ad);
      if (s != null && s.getSource() == SourceType.USER_DEFINED && !s.getName().equals(p[1]) && s.getSymbolType() == SymbolType.FUNCTION) {
        println("skip function at " + p[0] + " (" + s.getName() + ")"); bad++; continue;
      }
      // an earlier pass's label is renamed in place (createLabel would keep the old name as a second label)
      if (s != null && s.getSource() == SourceType.USER_DEFINED && s.getSymbolType() == SymbolType.LABEL)
        s.setName(p[1], SourceType.USER_DEFINED);
      else createLabel(ad, p[1], true, SourceType.USER_DEFINED);
      named++;
      if (dt != null) {
        try {
          clearListing(ad, ad.add(dt.getLength() - 1));
          createData(ad, dt);
          typed++;
        } catch (Exception e) { println("type fail " + p[0] + " " + p[2] + ": " + e.getMessage()); bad++; }
      }
    }
    println("globals: named " + named + ", typed " + typed + ", problems " + bad + (dry ? " (dry)" : ""));
  }
}
