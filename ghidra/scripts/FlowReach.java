// FlowReach.java: args are entry addresses (hex). Follows the code flow from each entry (fall-through and jumps, not calls,
// stopping at returns) and prints every other function entry reached and every flow target that is not an instruction
// start (a conflicting decode, usually a false entry inside a real function). Read-only.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.util.*;

public class FlowReach extends GhidraScript {
    @Override
    public void run() throws Exception {
        Listing lst = currentProgram.getListing(); FunctionManager fm = currentProgram.getFunctionManager();
        for (String s : getScriptArgs()) {
            Address e = toAddr(Long.parseLong(s.replace("0x", ""), 16));
            Deque<Address> q = new ArrayDeque<>(); Set<Address> seen = new HashSet<>(); q.add(e);
            Set<String> hits = new TreeSet<>(); Address max = e; int n = 0;
            while (!q.isEmpty() && n < 5000) {
                Address p = q.poll(); if (!seen.add(p)) continue; n++;
                if (!p.equals(e)) { Function g = fm.getFunctionAt(p); if (g != null) hits.add("entry " + p + " " + g.getName()); }
                Instruction ins = lst.getInstructionAt(p);
                if (ins == null) { CodeUnit cu = lst.getCodeUnitContaining(p); Function o = fm.getFunctionContaining(p); hits.add("noinst " + p + " cu=" + (cu == null ? "-" : cu.getMinAddress()) + " owner=" + (o == null ? "-" : o.getName())); continue; }
                if (ins.getMaxAddress().compareTo(max) > 0) max = ins.getMaxAddress();
                FlowType ft = ins.getFlowType();
                if (ft.isTerminal()) continue;
                if (ft.isJump()) for (Address t : ins.getFlows()) q.add(t);
                if (!ft.isJump() || ft.isConditional()) { Address f = ins.getFallThrough(); if (f != null) q.add(f); }
            }
            println("FlowReach " + e + " max=" + max + " insts=" + n + " " + hits);
        }
    }
}
