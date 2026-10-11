// Names functions from the Zyrinx engine routine table (LORT.BIN, scordos, SCORWIN).
// Entry = _ConRouts from MAINENG.ASC: char ConRoutName[16] + 9 function pointers RoutPtr[9] at offset 16
// (index = _RJmpPreInit(0) Init Frmupdate Visible Coli Display RateLoop CloseDown(7), 8 = spare, always 0).
// Scans the whole program for entries, labels the table "rout_<name>", creates/names the slot functions "<name>_<slot>".
// Re-runnable: an old "<x>_<slot>" name/label on a slot function is replaced (10-10 fix: slots used to be read at +20).
//@category Scorcher
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.symbol.*;
import java.util.*;

public class NameRoutineTable extends GhidraScript {
    static final String[] SLOTS = {"PreInit", "Init", "Frmupdate", "Visible", "Coli", "Display", "RateLoop", "CloseDown", "Spare"};
    static final int NAMELEN = 16, NSLOT = 9, ENTRY = 52;

    boolean isSlotName(String s) {
        for (String k : SLOTS) if (s.endsWith("_" + k)) return true;
        return false;
    }

    @Override
    public void run() throws Exception {
        Memory mem = currentProgram.getMemory();
        SymbolTable st = currentProgram.getSymbolTable();
        long lo = mem.getMinAddress().getOffset(), hi = mem.getMaxAddress().getOffset();
        int entries = 0, named = 0, created = 0, removed = 0;
        boolean evenOnly = currentProgram.getLanguage().getProcessor().toString().startsWith("Super");  // SH-2 code is 2-aligned
        long nextFree = 0;
        Set<Address> touched = new HashSet<>();
        for (ghidra.program.model.mem.MemoryBlock blk : mem.getBlocks()) {
          if (!blk.isInitialized()) continue;
          for (long a = blk.getStart().getOffset(); a + ENTRY <= blk.getEnd().getOffset(); a += 4) {
            if (a < nextFree) continue;   // inside the previous accepted entry
            byte[] nm = new byte[NAMELEN];
            mem.getBytes(toAddr(a), nm);
            int len = 0;
            while (len < NAMELEN && nm[len] != 0) len++;
            if (len < 3 || len > NAMELEN - 1) continue;
            boolean ok = (nm[0] >= 'a' && nm[0] <= 'z');
            for (int i = 1; i < len && ok; i++) {
                char c = (char) nm[i];
                ok = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
            }
            for (int i = len; i < NAMELEN && ok; i++) ok = nm[i] == 0;
            if (!ok) continue;
            long[] p = new long[NSLOT];
            boolean any = false, valid = true;
            for (int i = 0; i < NSLOT; i++) {
                p[i] = mem.getInt(toAddr(a + NAMELEN + 4 * i)) & 0xffffffffL;
                if (p[i] == 0) continue;
                any = true;
                if (p[i] < lo || p[i] >= hi || !mem.contains(toAddr(p[i])) || (evenOnly && (p[i] & 1) != 0)) valid = false;
            }
            if (!any || !valid || p[NSLOT - 1] != 0) continue;
            String name = new String(nm, 0, len);
            entries++; nextFree = a + ENTRY;
            // drop stale rout_ labels at this entry, then label it
            for (Symbol s : st.getSymbols(toAddr(a))) if (s.getName().startsWith("rout_") && !s.getName().equals("rout_" + name)) { s.delete(); removed++; }
            createLabel(toAddr(a), "rout_" + name, true, SourceType.USER_DEFINED);
            for (int i = 0; i < NSLOT - 1; i++) {
                if (p[i] == 0) continue;
                Address fa = toAddr(p[i]);
                Function f = getFunctionAt(fa);
                if (f == null) {
                    disassemble(fa);
                    f = createFunction(fa, null);
                    if (f == null) { printerr("no function at " + fa + " for " + name + "_" + SLOTS[i]); continue; }
                    created++;
                }
                String fn = name + "_" + SLOTS[i];
                if (!touched.contains(fa)) {
                    // first routine to claim this address in this run: remove old slot names/labels, set the primary name
                    touched.add(fa);
                    for (Symbol s : st.getSymbols(fa)) {
                        if (s.isPrimary() || !isSlotName(s.getName())) continue;
                        s.delete(); removed++;
                    }
                    if (!f.getName().equals(fn)) { f.setName(fn, SourceType.USER_DEFINED); named++; }
                } else if (!f.getName().equals(fn)) {
                    // the same address serves several routines; keep the first name, add the rest as labels
                    createLabel(fa, fn, false, SourceType.USER_DEFINED); named++;
                }
            }
          }
        }
        println("routine entries " + entries + ", names set " + named + ", functions created " + created + ", stale symbols removed " + removed);
    }
}
