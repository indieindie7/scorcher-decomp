import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;
// SetChkSig.java <addr>: give the Watcom stack probe the __chkcall convention (all registers unaffected)
// and the signature void f(int size), so decompiles keep EAX/EDX/EBX/ECX across the call.
public class SetChkSig extends GhidraScript {
  public void run() throws Exception {
    for (String s : getScriptArgs()) {
      Function f = getFunctionAt(toAddr(Long.parseLong(s.replace("0x",""),16)));
      if (f == null) { println("no function " + s); continue; }
      f.setCallingConvention("__chkcall");
      f.setReturnType(DataType.VOID, SourceType.USER_DEFINED);
      f.replaceParameters(Function.FunctionUpdateType.DYNAMIC_STORAGE_FORMAL_PARAMS, true, SourceType.USER_DEFINED,
        new ParameterImpl("size", IntegerDataType.dataType, currentProgram));
      f.setNoReturn(false);
      println("set " + f.getName() + " " + f.getPrototypeString(true, false) + " cc=" + f.getCallingConventionName());
    }
  }
}
