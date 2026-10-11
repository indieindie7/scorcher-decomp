// Marks the LORT.BIN entry (jump target at file start) and prints analysis stats.
//@category Scorcher
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;

public class ScorcherStats extends GhidraScript {
    @Override
    public void run() throws Exception {
        Address entry = toAddr(0x06005E8A);
        if (!currentProgram.getMemory().contains(entry)) {
            // not LORT.BIN
        } else if (getFunctionAt(entry) == null) {
            disassemble(entry);
            createFunction(entry, "lort_entry");
        } else {
            getFunctionAt(entry).setName("lort_entry", SourceType.USER_DEFINED);
        }
        int n = 0; long bytes = 0;
        for (Function f : currentProgram.getFunctionManager().getFunctions(true)) { n++; bytes += f.getBody().getNumAddresses(); }
        println("functions " + n + ", code bytes in functions " + bytes + " of " + currentProgram.getMemory().getSize());
        println("strings defined " + currentProgram.getListing().getDefinedData(true).toString());
    }
}
