// Clear wrong non-returning flags, disassemble past the affected call sites and regrow function bodies.
// args: addresses of functions that really never return (kept); every other noreturn flag is cleared.
import ghidra.app.script.GhidraScript;
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.cmd.function.CreateFunctionCmd;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.util.exception.*;
import java.util.*;
public class FixNoReturn extends GhidraScript {
    public void run() throws Exception {
        Set<Address> keep = new HashSet<>();
        for (String a : getScriptArgs()) keep.add(toAddr(Long.parseLong(a.replace("0x", ""), 16)));
        try { setAnalysisOption(currentProgram, "Non-Returning Functions - Discovered", "false"); } catch (Exception e) { println("option: " + e); }
        FunctionManager fm = currentProgram.getFunctionManager(); Listing lst = currentProgram.getListing();
        long before = 0; for (Function f : fm.getFunctions(true)) before += f.getBody().getNumAddresses();
        int cleared = 0, dis = 0, grown = 0, removed = 0;
        for (int pass = 0; pass < 6; pass++) {
            int passGrown = 0;
            for (Function f : fm.getFunctions(true)) {
                if (!f.hasNoReturn() || keep.contains(f.getEntryPoint())) continue;
                f.setNoReturn(false); cleared++;
                for (Reference r : currentProgram.getReferenceManager().getReferencesTo(f.getEntryPoint())) {
                    if (!r.getReferenceType().isCall()) continue;
                    Instruction ins = lst.getInstructionAt(r.getFromAddress());
                    if (ins == null) continue;
                    if (ins.isFallThroughOverridden()) ins.clearFallThroughOverride();
                    Address ft = ins.getFallThrough();
                    if (ft != null && lst.getInstructionAt(ft) == null && lst.getDefinedDataAt(ft) == null) {
                        DisassembleCommand dc = new DisassembleCommand(ft, null, true); dc.applyTo(currentProgram, monitor); dis++;
                    }
                }
            }
            List<Function> all = new ArrayList<>(); for (Function f : fm.getFunctions(true)) all.add(f);
            for (Function f : all) {
                if (fm.getFunctionAt(f.getEntryPoint()) == null) continue;   // removed meanwhile
                AddressSetView nb = CreateFunctionCmd.getFunctionBody(currentProgram, f.getEntryPoint());
                if (nb == null || nb.getNumAddresses() <= f.getBody().getNumAddresses()) continue;
                for (int tries = 0; tries < 4; tries++) {
                    try { f.setBody(nb); grown++; passGrown++; break; }
                    catch (Exception e) {
                        if (!e.getClass().getSimpleName().contains("Overlapping")) { println("cannot grow " + f.getName() + ": " + e); break; }
                        boolean any = false;
                        for (Function g : fm.getFunctions(nb, true)) {
                            if (g.getEntryPoint().equals(f.getEntryPoint())) continue;
                            println("remove overlapped " + g.getName() + " @ " + g.getEntryPoint() + " (inside " + f.getName() + ")");
                            fm.removeFunction(g.getEntryPoint()); removed++; any = true;
                        }
                        if (!any) { println("cannot grow " + f.getName()); break; }
                    }
                }
            }
            println("pass " + pass + ": grown " + passGrown);
            if (passGrown == 0) break;
        }
        analyzeChanges(currentProgram);
        int still = 0; for (Function f : fm.getFunctions(true)) if (f.hasNoReturn() && !keep.contains(f.getEntryPoint())) { still++; println("still noreturn " + f.getName() + " @ " + f.getEntryPoint()); }
        long after = 0; int n = 0; for (Function f : fm.getFunctions(true)) { after += f.getBody().getNumAddresses(); n++; }
        println("FixNoReturn: cleared " + cleared + ", disassembled " + dis + " sites, grown " + grown + ", removed " + removed + ", still " + still + "; functions " + n + ", code bytes " + before + " -> " + after);
    }
}
