// Removes bogus functions whose entry lies a few bytes after the fall-through of the preceding instruction
// (Ghidra started them mid-instruction), clears their listing and re-disassembles from the fall-through so
// RepairBodies can grow the owning function over them. Arg "list" only prints the candidates.
import ghidra.app.script.GhidraScript;
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.util.*;
public class FixSplitEntries extends GhidraScript {
    public void run() throws Exception {
        FunctionManager fm = currentProgram.getFunctionManager(); Listing lst = currentProgram.getListing();
        boolean listOnly = getScriptArgs().length > 0 && getScriptArgs()[0].equals("list");
        // optional: further args = only fix entries at these addresses (hex); "list" may precede them
        java.util.Set<Long> only = new java.util.HashSet<>();
        for (String a : getScriptArgs()) { if (a.equals("list")) continue; only.add(Long.parseLong(a.replaceFirst("^0x", ""), 16)); }
        List<Function> snap = new ArrayList<>(); for (Function f : fm.getFunctions(true)) snap.add(f);
        int fixed = 0;
        for (Function f : snap) {
            Address e = f.getEntryPoint();
            if (f.getSymbol().getSource() == SourceType.USER_DEFINED) continue;
            if (!only.isEmpty() && !only.contains(e.getOffset())) continue;
            boolean referenced = false;
            for (Reference r : currentProgram.getReferenceManager().getReferencesTo(e)) { if (!r.getReferenceType().isFallthrough()) { referenced = true; break; } }
            if (referenced) continue;
            Instruction prev = lst.getInstructionBefore(e);
            if (prev == null) continue;
            Address ft = prev.getFallThrough();
            if (ft == null || ft.compareTo(e) >= 0 || e.subtract(ft) > 8) continue;
            Function owner = fm.getFunctionContaining(prev.getAddress());
            if (owner == null || owner.getEntryPoint().equals(e)) continue;
            println("split entry " + f.getName() + " at " + e + ": restart " + ft + " inside " + owner.getName() + " (prev " + prev + ")");
            if (listOnly) continue;
            Address max = f.getBody().getMaxAddress();
            fm.removeFunction(e);
            lst.clearCodeUnits(ft, max, false);
            new DisassembleCommand(ft, null, true).applyTo(currentProgram, monitor);
            fixed++;
        }
        println("FixSplitEntries: fixed " + fixed);
    }
}
