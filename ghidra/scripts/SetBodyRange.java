// SetBodyRange.java: args are pairs <entry> <end> (hex, end exclusive). The span's instructions are taken away from
// any other function and become the whole body of the function at entry (created as DEFAULT if missing).
// For dispatch-table handlers that a neighbour swallowed (e.g. the 387 emulator table at 0x6216c).
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;
import java.util.*;

public class SetBodyRange extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs(); FunctionManager fm = currentProgram.getFunctionManager(); Listing lst = currentProgram.getListing();
        for (int i = 0; i + 1 < a.length; i += 2) {
            Address s = toAddr(Long.parseLong(a[i].replace("0x", ""), 16)), e = toAddr(Long.parseLong(a[i + 1].replace("0x", ""), 16) - 1);
            // drop DEFAULT (FUN_) functions whose entry lies inside the span after the first byte, then disassemble it
            for (Function q : new ArrayList<Function>() {{ for (Function g : fm.getFunctions(new AddressSet(s.add(1), e), true)) add(g); }})
                if (q.getSymbol().getSource() == SourceType.DEFAULT) { println("SetBodyRange: removed " + q.getName()); fm.removeFunction(q.getEntryPoint()); }
            new DisassembleCommand(s, null, true).applyTo(currentProgram, monitor);
            AddressSet nb = new AddressSet();
            for (Instruction ins : lst.getInstructions(new AddressSet(s, e), true)) nb.add(new AddressRangeImpl(ins.getMinAddress(), ins.getMaxAddress()));
            if (nb.isEmpty()) { println("SetBodyRange: no code at " + s); continue; }
            List<Function> others = new ArrayList<>();
            for (Iterator<Function> it = fm.getFunctionsOverlapping(nb); it.hasNext();) { Function q = it.next(); if (!q.getEntryPoint().equals(s)) others.add(q); }
            for (Function q : others) { AddressSetView rest = q.getBody().subtract(nb); if (!rest.contains(q.getEntryPoint())) { println("SetBodyRange: span holds entry of " + q.getName() + ", skipped"); nb = null; break; } q.setBody(rest); }
            if (nb == null) continue;
            Function f = fm.getFunctionAt(s);
            if (f == null) { f = fm.createFunction(null, s, nb, SourceType.DEFAULT); println("SetBodyRange: created at " + s + " " + nb.getNumAddresses()); }
            else { f.setBody(nb); println("SetBodyRange: " + f.getName() + " = " + nb.getNumAddresses()); }
        }
    }
}
