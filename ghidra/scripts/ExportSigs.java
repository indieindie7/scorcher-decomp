// ExportSigs.java <abs out.txt>: writes "addr<TAB>source<TAB>prototype" for every function and
// "addr<TAB>G<TAB>name<TAB>type" for every USER_DEFINED global label (types pass record for the repo).
//@category Scorcher
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.io.*;
public class ExportSigs extends GhidraScript {
  public void run() throws Exception {
    int nf = 0, ng = 0;
    try (PrintWriter w = new PrintWriter(new FileWriter(getScriptArgs()[0]))) {
      for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
        w.println(f.getEntryPoint() + "\t" + f.getSignatureSource() + "\t" + f.getPrototypeString(true, false));
        nf++;
      }
      SymbolIterator it = currentProgram.getSymbolTable().getAllSymbols(true);
      while (it.hasNext()) {
        Symbol s = it.next();
        if (s.getSymbolType() != SymbolType.LABEL || s.getSource() != SourceType.USER_DEFINED || !s.getName().startsWith("g_")) continue;
        Data d = getDataAt(s.getAddress());
        w.println(s.getAddress() + "\tG\t" + s.getName() + "\t" + (d == null ? "-" : d.getDataType().getName()));
        ng++;
      }
    }
    println("exported " + nf + " signatures, " + ng + " globals");
  }
}
