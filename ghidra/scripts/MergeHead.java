// MergeHead.java: args are triples <head> <falseEntry|-> <endExcl> (hex). For a real function head whose flow runs into a
// false (mid-instruction) entry: deletes the function at falseEntry, clears the listing in [head,end), disassembles from
// head by flow and recomputes the head function's body from its flow. "-" as falseEntry only re-flows the head.
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.cmd.function.CreateFunctionCmd;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;

public class MergeHead extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs(); FunctionManager fm = currentProgram.getFunctionManager();
        for (int i = 0; i + 2 < a.length; i += 3) {
            Address h = toAddr(Long.parseLong(a[i], 16)), e = toAddr(Long.parseLong(a[i + 2], 16) - 1);
            if (!a[i + 1].equals("-")) { Function g = fm.getFunctionAt(toAddr(Long.parseLong(a[i + 1], 16))); if (g != null) { println("MergeHead: delete " + g.getName()); fm.removeFunction(g.getEntryPoint()); } }
            Function f = fm.getFunctionAt(h);
            if (f == null) { println("MergeHead: no function at " + h); continue; }
            fm.removeFunction(h);
            clearListing(h, e);
            new DisassembleCommand(h, new AddressSet(h, e), true).applyTo(currentProgram, monitor);
            CreateFunctionCmd c = new CreateFunctionCmd(null, h, null, ghidra.program.model.symbol.SourceType.DEFAULT);
            boolean ok = c.applyTo(currentProgram, monitor);
            Function n = fm.getFunctionAt(h);
            println("MergeHead: " + h + " " + (n == null ? "FAILED " + c.getStatusMsg() : n.getName() + " body=" + n.getBody().getNumAddresses() + " max=" + n.getBody().getMaxAddress()));
        }
    }
}
