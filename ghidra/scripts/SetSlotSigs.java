// SetSlotSigs.java [dry]: types pass step 3. For every routine-table entry labelled rout_<name>
// (_ConRouts: name[16] + RoutPtr[9]), gives each slot function the parameters
// (WorldRAMHeader *wh, void *wram, ObjRAMHeader *oh, void *oram) -- the arguments rout_CallSlot passes --
// in the program's default convention. The return type is kept. Signature source USER_DEFINED.
//@category Scorcher
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.util.*;
public class SetSlotSigs extends GhidraScript {
  public void run() throws Exception {
    boolean dry = getScriptArgs().length > 0 && getScriptArgs()[0].equals("dry");
    DataTypeManager dtm = currentProgram.getDataTypeManager();
    DataType wh = dtm.getDataType(new CategoryPath("/Scorcher"), "WorldRAMHeader");
    DataType oh = dtm.getDataType(new CategoryPath("/Scorcher"), "ObjRAMHeader");
    if (wh == null || oh == null) { println("missing /Scorcher structs"); return; }
    boolean big = currentProgram.getLanguage().isBigEndian();
    Set<Address> seen = new HashSet<>();
    int tables = 0, set = 0, skipped = 0;
    SymbolIterator it = currentProgram.getSymbolTable().getAllSymbols(true);
    while (it.hasNext()) {
      Symbol s = it.next();
      if (!s.getName().startsWith("rout_") || s.getSymbolType() != SymbolType.LABEL) continue;
      Address t = s.getAddress();
      tables++;
      for (int i = 0; i < 9; i++) {
        long p = getInt(t.add(0x10 + 4 * i)) & 0xffffffffL;
        if (p == 0) continue;
        Function f = getFunctionAt(toAddr(p));
        if (f == null) { skipped++; continue; }
        if (!seen.add(f.getEntryPoint())) continue;
        if (dry) { set++; continue; }
        f.replaceParameters(Function.FunctionUpdateType.DYNAMIC_STORAGE_FORMAL_PARAMS, true, SourceType.USER_DEFINED,
          new ParameterImpl("wh", new PointerDataType(wh, dtm), currentProgram),
          new ParameterImpl("wram", PointerDataType.dataType, currentProgram),
          new ParameterImpl("oh", new PointerDataType(oh, dtm), currentProgram),
          new ParameterImpl("oram", PointerDataType.dataType, currentProgram));
        f.setSignatureSource(SourceType.USER_DEFINED);
        set++;
      }
    }
    println("slot sigs: tables " + tables + ", functions " + set + ", no function at pointer " + skipped + (dry ? " (dry)" : ""));
  }
}
