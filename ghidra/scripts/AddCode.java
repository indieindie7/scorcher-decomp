// AddCode.java: args are triples <funcEntry> <start> <endExcl> (hex). Disassembles [start,end) and adds every instruction
// there that no other function owns to the function at funcEntry. For switch case blocks Ghidra did not reach, when the
// jump table sits inside the span (GrowToRange would decode the table as code).
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;

public class AddCode extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs(); FunctionManager fm = currentProgram.getFunctionManager(); Listing lst = currentProgram.getListing();
        for (int i = 0; i + 2 < a.length; i += 3) {
            Function f = fm.getFunctionAt(toAddr(Long.parseLong(a[i], 16)));
            Address s = toAddr(Long.parseLong(a[i + 1], 16)), e = toAddr(Long.parseLong(a[i + 2], 16) - 1);
            if (f == null) { println("AddCode: no function at " + a[i]); continue; }
            AddressSet span = new AddressSet(s, e);
            new DisassembleCommand(s, span, true).applyTo(currentProgram, monitor);
            AddressSet nb = new AddressSet();
            for (Instruction ins : lst.getInstructions(span, true)) {
                Function o = fm.getFunctionContaining(ins.getAddress());
                if (o == null || o.equals(f)) nb.add(new AddressRangeImpl(ins.getMinAddress(), ins.getMaxAddress()));
            }
            long old = f.getBody().getNumAddresses();
            try { f.setBody(f.getBody().union(nb)); println("AddCode: " + f.getName() + " " + old + " -> " + f.getBody().getNumAddresses()); }
            catch (Exception x) { println("AddCode: failed " + f.getName() + ": " + x); }
        }
    }
}
