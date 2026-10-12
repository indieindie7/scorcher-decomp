// UnownedScan.java: lists the ranges of executable memory that no function body owns. One line per range:
// U <start> <endExcl> <len> <first 8 bytes> | refs into the range as <from>@<owning function> (up to 4).
// args: optional <out.txt> (default: println). Read-only; padding-only ranges (00/90/CC, or lea eax,[eax] 8D 40 00) are skipped.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import ghidra.program.model.symbol.*;
import java.io.*;

public class UnownedScan extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs(); PrintWriter out = a.length > 0 ? new PrintWriter(new FileWriter(a[0])) : null;
        FunctionManager fm = currentProgram.getFunctionManager(); Memory mem = currentProgram.getMemory(); ReferenceManager rm = currentProgram.getReferenceManager();
        AddressSet owned = new AddressSet();
        for (Function f : fm.getFunctions(true)) owned.add(f.getBody());
        AddressSet exec = new AddressSet();
        for (MemoryBlock b : mem.getBlocks()) if (b.isExecute() && b.isInitialized()) exec.add(b.getStart(), b.getEnd());
        int n = 0;
        for (AddressRange r : exec.subtract(owned)) {
            long len = r.getLength(); byte[] buf = new byte[(int) Math.min(len, 1 << 20)]; mem.getBytes(r.getMinAddress(), buf);
            boolean pad = true;
            for (int i = 0; i < buf.length; i++) { int x = buf[i] & 0xff; if (x == 0 || x == 0x90 || x == 0xcc) continue; if (x == 0x8d && i + 2 < buf.length && buf[i + 1] == 0x40 && buf[i + 2] == 0) { i += 2; continue; } pad = false; break; }
            if (pad) continue;
            StringBuilder hb = new StringBuilder(); for (int i = 0; i < Math.min(8, buf.length); i++) hb.append(String.format("%02x", buf[i] & 0xff));
            StringBuilder rs = new StringBuilder(); int k = 0;
            for (Address p : rm.getReferenceDestinationIterator(new AddressSet(r), true)) {
                for (Reference ref : rm.getReferencesTo(p)) { if (k >= 4) break; Function o = fm.getFunctionContaining(ref.getFromAddress()); rs.append(" " + ref.getFromAddress() + ">" + p + "@" + (o == null ? "-" : o.getName()) + ":" + ref.getReferenceType()); k++; }
                if (k >= 4) break;
            }
            String line = "U " + r.getMinAddress() + " " + r.getMaxAddress().add(1) + " " + len + " " + hb + " |" + rs;
            if (out != null) out.println(line); else println(line);
            n++;
        }
        if (out != null) out.close();
        println("UnownedScan: " + n + " non-padding unowned ranges");
    }
}
