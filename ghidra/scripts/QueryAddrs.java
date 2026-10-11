// For each address argument print what the listing holds there: instruction/data, containing function, references to it.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
public class QueryAddrs extends GhidraScript {
    public void run() throws Exception {
        for (String s : getScriptArgs()) {
            Address a = toAddr(Long.parseLong(s.replace("0x", ""), 16));
            Instruction ins = currentProgram.getListing().getInstructionContaining(a);
            Data d = currentProgram.getListing().getDataContaining(a);
            Function f = getFunctionContaining(a);
            StringBuilder refs = new StringBuilder();
            for (Reference r : currentProgram.getReferenceManager().getReferencesTo(a)) refs.append(" ").append(r.getReferenceType()).append("@").append(r.getFromAddress());
            println("Q " + a + " ins=" + (ins == null ? "-" : ins.getAddress() + ":" + ins) + " ft=" + (ins == null ? "-" : String.valueOf(ins.getFallThrough()) + (ins.isFallThroughOverridden() ? "(ovr)" : "")) + " data=" + (d == null ? "-" : d.getAddress() + ":" + d.getDataType().getName() + (d.isDefined() ? "" : "(undef)")) + " func=" + (f == null ? "-" : f.getName() + "@" + f.getEntryPoint() + " size=" + f.getBody().getNumAddresses()) + " refs:" + refs);
        }
    }
}
