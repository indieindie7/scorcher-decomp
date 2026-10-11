// Finds undiscovered Watcom (wcc386) functions in x86 programs.
// Candidates: (A) the first non-padding byte after an existing function's end, requiring a Watcom-looking prologue
//             (saved-register pushes, push ebp/mov ebp,esp, sub esp,n, or mov eax,n/call __CHK);
//             (B) any code address referenced by a 4-byte pointer anywhere in initialized memory (loose opcode check).
// Each candidate is disassembled and turned into a function; it is kept only if every instruction's flow lands on
// real instructions and the body reaches a RET. Repeats (with analyzeChanges between passes) until nothing new appears.
//@category Scorcher
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import java.util.*;

public class FindWatcomFunctions extends GhidraScript {
    Memory mem; Listing lst; FunctionManager fm; List<MemoryBlock> code = new ArrayList<>();
    int created = 0, rejected = 0, skippedPointer = 0;
    static final Set<Integer> PUSH = new HashSet<>(Arrays.asList(0x51, 0x52, 0x53, 0x55, 0x56, 0x57));
    static final Set<Integer> BAD_FIRST = new HashSet<>(Arrays.asList(0x00, 0xFF, 0xCC, 0x90, 0xC9, 0xCB, 0xCA, 0x9A, 0xEA,
        0x06, 0x07, 0x0E, 0x16, 0x17, 0x1E, 0x1F, 0x26, 0x27, 0x2E, 0x2F, 0x36, 0x37, 0x3E, 0x3F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65,
        0x67, 0x6C, 0x6D, 0x6E, 0x6F, 0x9B, 0x9E, 0x9F, 0xC4, 0xC5, 0xC8, 0xCE, 0xCF, 0xD4, 0xD5, 0xD6, 0xD7, 0xE0, 0xE1, 0xE2, 0xE3,
        0xE4, 0xE5, 0xE6, 0xE7, 0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF4, 0xF5, 0xF8, 0xF9, 0xFA, 0xFB, 0xFE, 0xC3, 0xC2));

    @Override
    public void run() throws Exception {
        mem = currentProgram.getMemory(); lst = currentProgram.getListing(); fm = currentProgram.getFunctionManager();
        for (MemoryBlock b : mem.getBlocks()) if (b.isInitialized() && b.isExecute()) code.add(b);
        if (code.isEmpty()) for (MemoryBlock b : mem.getBlocks()) if (b.isInitialized()) code.add(b);
        for (MemoryBlock b : code) println("code block " + b.getName() + " " + b.getStart() + "-" + b.getEnd());
        int before = fm.getFunctionCount();
        for (int pass = 1; pass <= 8; pass++) {
            int start = fm.getFunctionCount(), c0 = created;
            // (B) pointer-referenced code
            for (MemoryBlock b : mem.getBlocks()) {
                if (!b.isInitialized()) continue;
                byte[] buf = new byte[(int) b.getSize()];
                b.getBytes(b.getStart(), buf);
                for (int i = 0; i + 4 <= buf.length; i++) {
                    long v = (buf[i] & 0xffL) | (buf[i + 1] & 0xffL) << 8 | (buf[i + 2] & 0xffL) << 16 | (buf[i + 3] & 0xffL) << 24;
                    Address t = inCode(v);
                    if (t != null) tryCandidate(t, false);
                }
            }
            // (A) after existing function ends
            List<Address> ends = new ArrayList<>();
            for (Function f : fm.getFunctions(true)) ends.add(f.getBody().getMaxAddress());
            for (Address e : ends) {
                Address a = e.add(1);
                for (int pad = 0; pad < 16; pad++) {
                    if (inCode(a.getOffset()) == null) { a = null; break; }
                    int bb = mem.getByte(a) & 0xff;
                    if (bb == 0x90 || bb == 0xCC || bb == 0x00) a = a.add(1); else break;
                }
                if (a != null && inCode(a.getOffset()) != null) tryCandidate(a, true);
            }
            println("pass " + pass + ": created " + (created - c0) + " (total functions " + fm.getFunctionCount() + ")");
            analyzeChanges(currentProgram);
            if (fm.getFunctionCount() == start && created == c0) break;
        }
        println("FindWatcomFunctions: created " + created + ", rejected " + rejected + ", functions " + before + " -> " + fm.getFunctionCount());
    }

    Address inCode(long v) {
        for (MemoryBlock b : code) if (v >= b.getStart().getOffset() && v <= b.getEnd().getOffset()) return toAddr(v);
        return null;
    }

    boolean prologue(Address a, boolean strict) throws Exception {
        byte[] p = new byte[8];
        try { mem.getBytes(a, p); } catch (Exception e) { return false; }
        int[] b = new int[8];
        for (int i = 0; i < 8; i++) b[i] = p[i] & 0xff;
        if (BAD_FIRST.contains(b[0]) && !(b[0] == 0xFF && (b[1] & 0x38) == 0x30)) return false;  // allow FF /6 push mem
        if (!strict) return true;
        int n = 0;
        while (n < 6 && PUSH.contains(b[n])) n++;
        if (n >= 1 && !BAD_FIRST.contains(b[n])) return true;
        if (b[0] == 0x55 && ((b[1] == 0x8B && b[2] == 0xEC) || (b[1] == 0x89 && b[2] == 0xE5))) return true;
        if ((b[0] == 0x83 || b[0] == 0x81) && b[1] == 0xEC) return true;
        if (b[0] == 0xB8 && b[5] == 0xE8) return true;
        return false;
    }

    void tryCandidate(Address a, boolean strict) throws Exception {
        if (fm.getFunctionContaining(a) != null) return;
        if (lst.getDefinedDataContaining(a) != null) return;
        Instruction existing = lst.getInstructionAt(a);
        if (existing == null && lst.getInstructionContaining(a) != null) return;  // middle of an instruction
        if (!prologue(a, strict)) return;
        AddressSetView fresh = new AddressSet(); if (existing == null) { ghidra.app.cmd.disassemble.DisassembleCommand dc = new ghidra.app.cmd.disassemble.DisassembleCommand(a, null, true); dc.applyTo(currentProgram, monitor); fresh = dc.getDisassembledAddressSet(); if (fresh == null) fresh = new AddressSet(); }
        Function f = createFunction(a, null);
        if (f != null && valid(f)) { created++; return; }
        if (f != null) removeFunction(f);
        if (!fresh.isEmpty()) clearListing(fresh);
        rejected++;
    }

    boolean valid(Function f) {
        boolean ret = false; int count = 0;
        for (Instruction ins : lst.getInstructions(f.getBody(), true)) {
            count++;
            if (ins.getFlowType().isTerminal() && !ins.getFlowType().isCall()) ret = true;
            if (ins.getFlowType().hasFallthrough()) {
                Address ft = ins.getFallThrough();
                if (ft == null || lst.getInstructionAt(ft) == null) return false;
            }
            if (!ins.getFlowType().isComputed()) for (Address t : ins.getFlows()) {
                if (t.isMemoryAddress() && mem.contains(t) && lst.getInstructionAt(t) == null) return false;
            }
        }
        return ret && count >= 2;
    }
}
