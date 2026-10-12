// GrowToRange.java: args are pairs <entry> <end> (hex, end exclusive). Disassembles undefined bytes in [entry,end)
// and sets the function at entry to every instruction in that span that no other function owns.
// A single arg @<file> reads the pairs from that file (whitespace separated).
// For single-prologue / single-ret functions whose body Ghidra left short (code reached only through flows it lost).
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;

public class GrowToRange extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs(); if (a.length == 1 && a[0].startsWith("@")) a = new String(java.nio.file.Files.readAllBytes(java.nio.file.Paths.get(a[0].substring(1)))).trim().split("[ \t\r\n]+"); FunctionManager fm = currentProgram.getFunctionManager(); Listing lst = currentProgram.getListing();
        for (int i = 0; i + 1 < a.length; i += 2) {
            Address s = toAddr(Long.parseLong(a[i].replace("0x", ""), 16)), e = toAddr(Long.parseLong(a[i + 1].replace("0x", ""), 16) - 1);
            Function f = fm.getFunctionAt(s);
            if (f == null) { println("GrowToRange: no function at " + s); continue; }
            AddressSet span = new AddressSet(s, e);
            AddressSet und = new AddressSet();
            for (AddressRange r : span) { Address p = r.getMinAddress(); while (p.compareTo(e) <= 0) { if (lst.getCodeUnitContaining(p) == null || lst.getUndefinedDataAt(p) != null) und.add(p); p = p.add(1); } }
            if (!und.isEmpty()) new DisassembleCommand(und, null, true).applyTo(currentProgram, monitor);
            AddressSet nb = new AddressSet();
            for (Instruction ins : lst.getInstructions(span, true)) {
                Function o = fm.getFunctionContaining(ins.getAddress());
                if (o == null || o.getEntryPoint().equals(s)) nb.add(new AddressRangeImpl(ins.getMinAddress(), ins.getMaxAddress()));
            }
            long old = f.getBody().getNumAddresses();
            try { f.setBody(f.getBody().union(nb)); println("GrowToRange: " + f.getName() + " " + old + " -> " + f.getBody().getNumAddresses()); }
            catch (Exception x) { println("GrowToRange: failed " + f.getName() + ": " + x); }
        }
    }
}
