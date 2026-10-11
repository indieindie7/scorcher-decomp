// Pre-analysis setup for the flattened SCORDOS.EXE image (tools/leflatten.py): marks the entry point from the .map file
// and splits the single block at each LE object start, setting execute/write from the object flags (0x4 exec, 0x2 write).
// Idempotent: a split is skipped when a block already starts at that address.
//@category Scorcher
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.mem.*;
import ghidra.program.model.symbol.SourceType;
import java.nio.file.*;

public class ScordosSetup extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        Memory mem = currentProgram.getMemory();
        for (String line : Files.readAllLines(Paths.get(args[0]))) {
            if (line.startsWith("entry ")) {
                Address e = toAddr(Long.decode(line.substring(6).trim()));
                disassemble(e);
                createFunction(e, "scordos_entry");
                currentProgram.getSymbolTable().addExternalEntryPoint(e);
                println("entry " + e);
            } else if (line.startsWith("obj")) {
                String[] t = line.split(" ");
                Address start = toAddr(Long.decode(t[1]));
                int flags = Integer.decode(t[4].substring(6));
                MemoryBlock b = mem.getBlock(start);
                if (b == null) continue;
                if (!b.getStart().equals(start)) { mem.split(b, start); b = mem.getBlock(start); }
                b.setName(t[0] + "_" + t[3]);
                b.setExecute((flags & 4) != 0);
                b.setWrite((flags & 2) != 0);
                println("block " + b.getName() + " " + b.getStart() + "-" + b.getEnd() + (b.isExecute() ? " exec" : "") + (b.isWrite() ? " write" : ""));
            }
        }
    }
}
