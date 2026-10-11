// Restore functions listed in a file ("<name> <addr>" per line; may be empty) and regrow every function body by
// following flow from its entry while never crossing another function's entry. Bodies only grow, except where a
// restored function has to be carved out of the function that swallowed it.
import ghidra.app.script.GhidraScript;
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.util.*;
public class RepairBodies extends GhidraScript {
    FunctionManager fm; Listing lst;
    int gapDis = 0;
    AddressSet body(Address entry) { return body(entry, false); }
    AddressSet body(Address entry, boolean stopAtBodies) {
        AddressSet set = new AddressSet(); Deque<Address> work = new ArrayDeque<>(); work.add(entry);
        while (!work.isEmpty()) {
            Address a = work.poll();
            if (set.contains(a)) continue;
            if (!a.equals(entry) && fm.getFunctionAt(a) != null) continue;
            if (stopAtBodies && !a.equals(entry)) { Function o = fm.getFunctionContaining(a); if (o != null && !o.getEntryPoint().equals(entry)) continue; }
            Instruction ins = lst.getInstructionAt(a);
            if (ins == null && currentProgram.getMemory().contains(a) && lst.getDefinedDataContaining(a) == null && lst.getInstructionContaining(a) == null) {
                // flow reached undisassembled bytes (e.g. the gap FixSplitEntries left before a tail): disassemble and continue
                DisassembleCommand dc = new DisassembleCommand(a, null, true);
                if (dc.applyTo(currentProgram, monitor)) { gapDis++; ins = lst.getInstructionAt(a); }
            }
            if (ins == null) continue;
            set.add(a, a.add(ins.getLength() - 1));
            FlowType ft = ins.getFlowType();
            if (ft.isCall()) {
                boolean noret = false;
                for (Address t : ins.getFlows()) { Function c = fm.getFunctionAt(t); if (c != null && c.hasNoReturn()) noret = true; }
                if (!noret && ins.getFallThrough() != null) work.add(ins.getFallThrough());
                continue;
            }
            if (ins.getFallThrough() != null) work.add(ins.getFallThrough());
            if (ft.isJump()) for (Address t : ins.getFlows()) work.add(t);
        }
        return set;
    }
    public void run() throws Exception {
        fm = currentProgram.getFunctionManager(); lst = currentProgram.getListing();
        String[] args = getScriptArgs(); int restored = 0, carved = 0;
        if (args.length > 0) {
            BufferedReader br = new BufferedReader(new FileReader(args[0])); String line;
            while ((line = br.readLine()) != null) {
                String[] t = line.trim().split("\s+"); if (t.length < 2) continue;
                Address a = toAddr(Long.parseLong(t[1], 16));
                if (fm.getFunctionAt(a) != null) continue;
                AddressSet b = body(a, true);
                if (b.isEmpty()) { println("no code at " + t[1]); continue; }
                Function owner = fm.getFunctionContaining(a);
                if (owner != null) { owner.setBody(owner.getBody().subtract(b)); carved++; }
                for (Function g : fm.getFunctions(b, true)) if (!g.getEntryPoint().equals(a)) { println("restore " + t[0] + ": body hits " + g.getName()); }
                Symbol s = currentProgram.getSymbolTable().getPrimarySymbol(a);
                String name = (s != null && s.getSource() != SourceType.DEFAULT) ? null : (t[0].startsWith("FUN_") || t[0].startsWith("thunk_") ? null : t[0]);
                try { fm.createFunction(name, a, b, name == null ? SourceType.DEFAULT : SourceType.IMPORTED); restored++; }
                catch (Exception e) { println("restore failed " + t[0] + " @ " + t[1] + ": " + e); }
            }
            br.close();
        }
        // drop CALL_RETURN flow overrides left on calls to functions that do return
        int ovr = 0;
        for (Instruction ins : lst.getInstructions(true)) {
            if (ins.getFlowOverride() != FlowOverride.CALL_RETURN) continue;
            boolean noret = false;
            for (Reference r : ins.getReferencesFrom()) { if (!r.getReferenceType().isCall()) continue; Function c = fm.getFunctionAt(r.getToAddress()); if (c != null && c.hasNoReturn()) noret = true; }
            if (noret) continue;
            ins.setFlowOverride(FlowOverride.NONE); ovr++;
        }
        println("cleared " + ovr + " CALL_RETURN overrides");
        // disassemble the fall-through after every call to a function that returns
        int dis = 0;
        for (Instruction ins : lst.getInstructions(true)) {
            if (!ins.getFlowType().isCall()) continue;
            boolean noret = false;
            for (Address t : ins.getFlows()) { Function c = fm.getFunctionAt(t); if (c != null && c.hasNoReturn()) noret = true; }
            if (noret) continue;
            Address ft = ins.getAddress().add(ins.getLength());
            if (lst.getInstructionAt(ft) != null || lst.getDefinedDataContaining(ft) != null) continue;
            if (!currentProgram.getMemory().contains(ft)) continue;
            DisassembleCommand dc = new DisassembleCommand(ft, null, true); if (dc.applyTo(currentProgram, monitor)) dis++;
        }
        println("disassembled after " + dis + " call sites");
        if (dis > 0) analyzeChanges(currentProgram);
        int merged = 0;
        List<Function> snap = new ArrayList<>(); for (Function f : fm.getFunctions(true)) snap.add(f);
        for (Function f : snap) {
            Address e = f.getEntryPoint();
            if (f.getSymbol().getSource() == SourceType.USER_DEFINED) continue;
            boolean referenced = false, jumpOnly = true; Function owner = null;
            for (Reference r : currentProgram.getReferenceManager().getReferencesTo(e)) {
                if (r.getReferenceType().isFallthrough()) continue;
                referenced = true;
                if (!r.getReferenceType().isJump()) { jumpOnly = false; break; }
                Function src = fm.getFunctionContaining(r.getFromAddress());
                if (src == null || src.getEntryPoint().equals(e)) { jumpOnly = false; break; }
                owner = src;
            }
            if (referenced && !jumpOnly) continue;
            if (!referenced) {   // only reachable by falling out of the previous instruction
                Instruction prev = lst.getInstructionBefore(e);
                if (prev == null || !e.equals(prev.getFallThrough())) continue;
                String m = prev.getMnemonicString().toLowerCase();
                if (m.equals("nop") || m.startsWith("int")) continue;
                if (prev.getFlowType().isCall()) { boolean noret = false; for (Address t : prev.getFlows()) { Function c = fm.getFunctionAt(t); if (c != null && c.hasNoReturn()) noret = true; } if (noret) continue; }
                owner = fm.getFunctionContaining(prev.getAddress());
            }
            if (owner == null || owner.getEntryPoint().equals(e) || owner.isThunk()) continue;
            println("merge fragment " + f.getName() + " into " + owner.getName() + (referenced ? " (jump target)" : ""));
            if (!fm.removeFunction(e)) println("  removeFunction failed " + e); merged++;
        }
        println("merged " + merged + " fragments");
        long before = 0; for (Function f : fm.getFunctions(true)) before += f.getBody().getNumAddresses();
        int grown = 0, blocked = 0;
        List<Function> all = new ArrayList<>(); for (Function f : fm.getFunctions(true)) all.add(f);
        for (Function f : all) {
            if (f.isThunk()) continue;
            AddressSet nb = body(f.getEntryPoint()); AddressSet add = nb.subtract(f.getBody());
            if (add.isEmpty()) continue;
            AddressSet ok = new AddressSet();
            for (AddressRange r : add) {
                // a function whose entry lies inside the range blocks it; a neighbour's body merely overlapping it
                // (e.g. a shared epilogue another function jumps into) is subtracted and the free part is taken
                boolean entryInside = false;
                for (Function q : fm.getFunctions(new AddressSet(r), true)) if (!q.getEntryPoint().equals(f.getEntryPoint())) entryInside = true;
                if (entryInside) { blocked++; if (blocked <= 12) println("blocked " + f.getName() + " range " + r + ": function entry inside"); continue; }
                AddressSet rr = new AddressSet(r);
                for (java.util.Iterator<Function> it = fm.getFunctionsOverlapping(new AddressSet(r)); it.hasNext();) { Function q = it.next(); if (!q.getEntryPoint().equals(f.getEntryPoint())) rr = rr.subtract(q.getBody()); }
                if (rr.isEmpty()) { blocked++; continue; }
                ok.add(rr);
            }
            if (ok.isEmpty()) continue;
            try { f.setBody(f.getBody().union(ok)); grown++; } catch (Exception e) { println("grow failed " + f.getName() + ": " + e); }
        }
        // orphan code: instructions no function owns -> new function at the start of each run
        int orphans = 0; AddressSet owned = new AddressSet(); for (Function f : fm.getFunctions(true)) owned.add(f.getBody());
        Address runStart = null, last = null;
        List<Address> starts = new ArrayList<>();
        for (Instruction ins : lst.getInstructions(true)) {
            Address a = ins.getAddress();
            if (owned.contains(a)) { runStart = null; last = null; continue; }
            if (runStart == null || last == null || !a.equals(last.add(1))) { runStart = a; starts.add(a); }
            last = ins.getMaxAddress();
        }
        for (Address a : starts) { if (fm.getFunctionContaining(a) != null) continue; Function nf = createFunction(a, null); if (nf != null) orphans++; }
        println("orphan runs " + starts.size() + ", functions created " + orphans);
        long after = 0; int n = 0; for (Function f : fm.getFunctions(true)) { after += f.getBody().getNumAddresses(); n++; }
        println("flow gaps disassembled " + gapDis);
        println("RepairBodies: restored " + restored + " (carved " + carved + "), grown " + grown + ", blocked ranges " + blocked + "; functions " + n + ", code bytes " + before + " -> " + after);
    }
}
