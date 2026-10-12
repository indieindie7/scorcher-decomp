// CommitParams.java [count|apply] [lo hi]: types pass step 1. Decompiles every function (optionally only
// those in [lo,hi)) and commits the decompiler's parameter list and return type to the database
// (source ANALYSIS, so hand-typed USER_DEFINED signatures are left alone). "count" only reports.
//@category Scorcher
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.pcode.*;
import ghidra.program.model.symbol.SourceType;
public class CommitParams extends GhidraScript {
  public void run() throws Exception {
    String[] a = getScriptArgs();
    boolean apply = a.length > 0 && a[0].equals("apply");
    long lo = a.length > 2 ? Long.decode(a[1]) : 0, hi = a.length > 2 ? Long.decode(a[2]) : Long.MAX_VALUE;
    int total = 0, withParams = 0, user = 0, done = 0, failed = 0, nonVoid = 0;
    for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
      total++;
      if (f.getParameterCount() > 0) withParams++;
      if (f.getSignatureSource() == SourceType.USER_DEFINED) user++;
      if (!f.getReturnType().getName().equals("void") && !f.getReturnType().getName().equals("undefined")) nonVoid++;
    }
    println("before: functions " + total + ", with params " + withParams + ", non-void return " + nonVoid + ", user signatures " + user);
    if (!apply) return;
    DecompInterface d = new DecompInterface();
    d.openProgram(currentProgram);
    for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
      if (monitor.isCancelled()) break;
      long off = f.getEntryPoint().getOffset();
      if (off < lo || off >= hi || f.isThunk() || f.isExternal()) continue;
      if (f.getSignatureSource() == SourceType.USER_DEFINED) continue;
      DecompileResults r = d.decompileFunction(f, 30, monitor);
      HighFunction h = r.getHighFunction();
      if (h == null) { failed++; continue; }
      try {
        HighFunctionDBUtil.commitParamsToDatabase(h, true, HighFunctionDBUtil.ReturnCommitOption.COMMIT, SourceType.ANALYSIS);
        done++;
      } catch (Exception e) { failed++; println("fail " + f.getName() + ": " + e.getMessage()); }
    }
    d.dispose();
    withParams = 0; nonVoid = 0;
    for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
      if (f.getParameterCount() > 0) withParams++;
      if (!f.getReturnType().getName().equals("void") && !f.getReturnType().getName().equals("undefined")) nonVoid++;
    }
    println("after: committed " + done + ", failed " + failed + ", with params " + withParams + ", non-void return " + nonVoid);
  }
}
