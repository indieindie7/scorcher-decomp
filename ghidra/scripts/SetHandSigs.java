// SetHandSigs.java <abs file> [dry]: types pass step 4. Lines "0x<addr> p1,p2,..." where each p is
// "name" (int) or "*name" (pointer). Replaces the parameter list of the function at addr with these
// parameters in the program's default convention (dynamic storage), keeps the return type, and marks the
// signature USER_DEFINED. Functions already USER_DEFINED (e.g. routine slots, __CHK) are left alone.
//@category Scorcher
import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;
import java.nio.file.*;
import java.util.*;
public class SetHandSigs extends GhidraScript {
  public void run() throws Exception {
    String[] a = getScriptArgs();
    boolean dry = a.length > 1 && a[1].equals("dry");
    int set = 0, missing = 0, kept = 0, fail = 0;
    for (String line : Files.readAllLines(Paths.get(a[0]))) {
      line = line.trim();
      if (line.isEmpty() || line.startsWith("#")) continue;
      String[] p = line.split("\\s+", 2);
      Function f = getFunctionAt(toAddr(Long.decode(p[0])));
      if (f == null) { missing++; continue; }
      if (f.getSignatureSource() == SourceType.USER_DEFINED) { kept++; continue; }
      List<Parameter> ps = new ArrayList<>();
      if (p.length > 1 && !p[1].equals("-"))
        for (String n : p[1].split(",")) {
          boolean ptr = n.startsWith("*");
          DataType dt = ptr ? PointerDataType.dataType : IntegerDataType.dataType;
          ps.add(new ParameterImpl(ptr ? n.substring(1) : n, dt, currentProgram));
        }
      if (dry) { set++; continue; }
      try {
        f.replaceParameters(ps, Function.FunctionUpdateType.DYNAMIC_STORAGE_FORMAL_PARAMS, true, SourceType.USER_DEFINED);
        f.setSignatureSource(SourceType.USER_DEFINED);
        set++;
      } catch (Exception e) { fail++; println("fail " + p[0] + " " + f.getName() + ": " + e.getMessage()); }
    }
    println("hand sigs: set " + set + ", already user " + kept + ", no function " + missing + ", failed " + fail + (dry ? " (dry)" : ""));
  }
}
